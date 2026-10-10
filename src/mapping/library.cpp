#include "abys/mapping/library.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <limits>
#include <optional>
#include <ostream>
#include <utility>
#include <vector>

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>

namespace abys::mapping {
namespace {

struct LinearModel {
  double block;
  double fanout;
};

struct LinearTimingArc {
  liberty::TimingSense sense = liberty::TimingSense::kUnknown;
  bool has_rise = false;
  bool has_fall = false;
  double rise_block_delay = 0.0;
  double rise_fanout_delay = 0.0;
  double fall_block_delay = 0.0;
  double fall_fanout_delay = 0.0;
};

struct FittedTimingArc {
  LinearTimingArc arc;
  bool rise_from_fall = false;
  bool fall_from_rise = false;
};

struct InterpolationPoint {
  size_t lower;
  size_t upper;
  double fraction;
};

InterpolationPoint interpolation_point(const std::vector<double> &axis, double value) {
  assert(!axis.empty());
  if (axis.size() == 1) {
    return {0, 0, 0.0};
  }
  const auto upper = std::upper_bound(axis.begin(), axis.end(), value);
  size_t upper_index;
  if (upper == axis.begin()) {
    upper_index = 1;
  } else if (upper == axis.end()) {
    upper_index = axis.size() - 1;
  } else {
    upper_index = static_cast<size_t>(upper - axis.begin());
  }
  const size_t lower_index = upper_index - 1;
  return {lower_index, upper_index,
          (value - axis[lower_index]) / (axis[upper_index] - axis[lower_index])};
}

double delay_variable_value(liberty::TableVariable variable, double input_net_transition,
                            double total_output_net_capacitance) {
  if (variable == liberty::TableVariable::kInputNetTransition ||
      variable == liberty::TableVariable::kInputTransitionTime) {
    return input_net_transition;
  }
  assert(variable == liberty::TableVariable::kTotalOutputNetCapacitance);
  return total_output_net_capacitance;
}

double interpolate(const liberty::TimingTable &table, double input_net_transition,
                   double total_output_net_capacitance) {
  if (table.index_1.empty()) {
    assert(table.index_2.empty());
    assert(table.values.size() == 1);
    return table.values.front();
  }
  const double index_1_value =
      delay_variable_value(table.variable_1, input_net_transition, total_output_net_capacitance);
  assert(table.values.size() ==
         table.index_1.size() * (table.index_2.empty() ? 1 : table.index_2.size()));
  const InterpolationPoint first = interpolation_point(table.index_1, index_1_value);
  if (table.index_2.empty()) {
    return std::lerp(table.values[first.lower], table.values[first.upper], first.fraction);
  }
  const double index_2_value =
      delay_variable_value(table.variable_2, input_net_transition, total_output_net_capacitance);
  const InterpolationPoint second = interpolation_point(table.index_2, index_2_value);
  const size_t row_size = table.index_2.size();
  const double lower =
      std::lerp(table.values[first.lower * row_size + second.lower],
                table.values[first.lower * row_size + second.upper], second.fraction);
  const double upper =
      std::lerp(table.values[first.upper * row_size + second.lower],
                table.values[first.upper * row_size + second.upper], second.fraction);
  return std::lerp(lower, upper, first.fraction);
}

const std::vector<double> *axis(const liberty::TimingTable &table,
                                liberty::TableVariable variable) {
  if (table.variable_1 == variable) {
    return &table.index_1;
  }
  if (table.variable_2 == variable) {
    return &table.index_2;
  }
  return nullptr;
}

bool supported_delay_variable(liberty::TableVariable variable) {
  return variable == liberty::TableVariable::kUnknown ||
         variable == liberty::TableVariable::kInputNetTransition ||
         variable == liberty::TableVariable::kInputTransitionTime ||
         variable == liberty::TableVariable::kTotalOutputNetCapacitance;
}

std::optional<LinearModel> fit_delays(const std::vector<const liberty::TimingTable *> &tables,
                                      std::optional<double> input_slew) {
  assert(!tables.empty());
  std::vector<double> loads;
  std::optional<double> reference_slew = input_slew;
  double minimum_slew = std::numeric_limits<double>::infinity();
  double maximum_slew = -std::numeric_limits<double>::infinity();
  for (const auto *table : tables) {
    if (!supported_delay_variable(table->variable_1) ||
        !supported_delay_variable(table->variable_2)) {
      return std::nullopt;
    }
    const std::vector<double> *table_loads =
        axis(*table, liberty::TableVariable::kTotalOutputNetCapacitance);
    if (table_loads != nullptr) {
      loads.insert(loads.end(), table_loads->begin(), table_loads->end());
    }
    const std::vector<double> *slews = axis(*table, liberty::TableVariable::kInputNetTransition);
    if (slews == nullptr) {
      slews = axis(*table, liberty::TableVariable::kInputTransitionTime);
    }
    if (!reference_slew && slews != nullptr) {
      minimum_slew = std::min(minimum_slew, slews->front());
      maximum_slew = std::max(maximum_slew, slews->back());
    }
  }
  if (!reference_slew && minimum_slew != std::numeric_limits<double>::infinity()) {
    reference_slew = (minimum_slew + maximum_slew) / 2.0;
  }
  if (loads.empty()) {
    loads.push_back(0.0);
  } else {
    std::ranges::sort(loads);
    loads.erase(std::ranges::unique(loads).begin(), loads.end());
  }

  std::vector<double> delays;
  delays.reserve(loads.size());
  for (double load : loads) {
    double delay = -std::numeric_limits<double>::infinity();
    for (const auto *table : tables) {
      delay = std::max(delay, interpolate(*table, reference_slew.value_or(0.0), load));
    }
    delays.push_back(delay);
  }
  if (loads.size() == 1) {
    return LinearModel{delays.front(), 0.0};
  }

  double load_sum = 0.0;
  double delay_sum = 0.0;
  double square_sum = 0.0;
  double product_sum = 0.0;
  for (size_t index = 0; index < loads.size(); ++index) {
    load_sum += loads[index];
    delay_sum += delays[index];
    square_sum += loads[index] * loads[index];
    product_sum += loads[index] * delays[index];
  }
  const double count = static_cast<double>(loads.size());
  const double denominator = count * square_sum - load_sum * load_sum;
  assert(denominator > 0.0);
  const double fanout = (count * product_sum - load_sum * delay_sum) / denominator;
  double block = (delay_sum - fanout * load_sum) / count;
  double adjustment = 0.0;
  for (size_t index = 0; index < loads.size(); ++index) {
    adjustment = std::max(adjustment, delays[index] - (block + fanout * loads[index]));
  }
  return LinearModel{block + adjustment, fanout};
}

std::optional<FittedTimingArc> fit_timing_arc(const liberty::Pin &output_pin,
                                              liberty::PinId input_pin_id,
                                              std::optional<double> input_slew) {
  FittedTimingArc result;
  bool found = false;
  bool found_sense = false;
  std::vector<const liberty::TimingTable *> rise_tables;
  std::vector<const liberty::TimingTable *> fall_tables;
  for (const auto &arc : output_pin.timing_arcs) {
    if (std::ranges::find(arc.related_pin_ids, input_pin_id) == arc.related_pin_ids.end()) {
      continue;
    }
    if (found_sense && result.arc.sense != arc.sense &&
        arc.sense != liberty::TimingSense::kUnknown) {
      result.arc.sense = liberty::TimingSense::kNonUnate;
    }
    if (!found_sense && arc.sense != liberty::TimingSense::kUnknown) {
      result.arc.sense = arc.sense;
      found_sense = true;
    }
    if (arc.cell_rise) {
      rise_tables.push_back(&*arc.cell_rise);
      found = true;
    }
    if (arc.cell_fall) {
      fall_tables.push_back(&*arc.cell_fall);
      found = true;
    }
  }
  if (!found) {
    return std::nullopt;
  }
  if (rise_tables.empty()) {
    rise_tables = fall_tables;
    result.rise_from_fall = true;
  }
  if (fall_tables.empty()) {
    fall_tables = rise_tables;
    result.fall_from_rise = true;
  }
  if (!rise_tables.empty()) {
    const auto model = fit_delays(rise_tables, input_slew);
    if (!model) {
      return std::nullopt;
    }
    result.arc.has_rise = true;
    result.arc.rise_block_delay = model->block;
    result.arc.rise_fanout_delay = model->fanout;
  }
  if (!fall_tables.empty()) {
    const auto model = fit_delays(fall_tables, input_slew);
    if (!model) {
      return std::nullopt;
    }
    result.arc.has_fall = true;
    result.arc.fall_block_delay = model->block;
    result.arc.fall_fanout_delay = model->fanout;
  }
  return result;
}

boop::CellLibrary::PinPhase boop_phase(liberty::TimingSense sense) {
  if (sense == liberty::TimingSense::kPositiveUnate) {
    return boop::CellLibrary::NONINV;
  }
  if (sense == liberty::TimingSense::kNegativeUnate) {
    return boop::CellLibrary::INV;
  }
  return boop::CellLibrary::UNKNOWN;
}

} // namespace

Library::Library(const liberty::CellLibrary &library, Diagnostics &diagnostics,
                 const Options &options) {
  for (const auto &cell : library.cells) {
    if (cell.sequential) {
      continue;
    }

    std::vector<liberty::PinId> input_pin_ids;
    std::vector<liberty::PinId> output_pin_ids;
    for (liberty::PinId pin_id = 0; pin_id < cell.pins.size(); ++pin_id) {
      const auto &pin = cell.pins[pin_id];
      if (pin.direction == liberty::PinDirection::kInput) {
        input_pin_ids.push_back(pin_id);
      } else if (pin.direction == liberty::PinDirection::kOutput) {
        output_pin_ids.push_back(pin_id);
      }
    }
    if (output_pin_ids.empty()) {
      continue;
    }
    if (input_pin_ids.size() > 6) {
      excluded_cells_.push_back({cell.name, ExclusionReason::kTooManyInputs});
      continue;
    }
    std::optional<ExclusionReason> exclusion_reason;
    for (liberty::PinId output_pin_id : output_pin_ids) {
      const auto &output_pin = cell.pins[output_pin_id];
      if (!output_pin.function) {
        exclusion_reason = ExclusionReason::kMissingFunction;
        break;
      }
      if (output_pin.three_state) {
        exclusion_reason = ExclusionReason::kThreeState;
        break;
      }
    }
    if (exclusion_reason) {
      excluded_cells_.push_back({cell.name, *exclusion_reason});
      continue;
    }
    std::vector<std::string> input_names;
    input_names.reserve(input_pin_ids.size());
    for (liberty::PinId input_pin_id : input_pin_ids) {
      input_names.push_back(cell.pins[input_pin_id].name);
    }
    for (liberty::PinId output_pin_id : output_pin_ids) {
      kitty::dynamic_truth_table function{static_cast<uint32_t>(input_names.size())};
      if (!kitty::create_from_formula(function, *cell.pins[output_pin_id].function, input_names)) {
        exclusion_reason = ExclusionReason::kUnsupportedFunction;
        break;
      }
    }
    if (exclusion_reason) {
      excluded_cells_.push_back({cell.name, *exclusion_reason});
      continue;
    }
    const double area = cell.area.value_or(1.0);
    if (!cell.area) {
      diagnostics.warning(DiagnosticId::kMappingFallback, cell.name + ": area uses unit area 1");
    }
    std::vector<double> input_loads;
    input_loads.reserve(input_pin_ids.size());
    for (liberty::PinId input_pin_id : input_pin_ids) {
      const auto &input_pin = cell.pins[input_pin_id];
      if (input_pin.capacitance) {
        input_loads.push_back(*input_pin.capacitance);
      } else if (input_pin.rise_capacitance || input_pin.fall_capacitance) {
        const double rise = input_pin.rise_capacitance.value_or(*input_pin.fall_capacitance);
        const double fall = input_pin.fall_capacitance.value_or(rise);
        input_loads.push_back(std::max(rise, fall));
        diagnostics.warning(DiagnosticId::kMappingFallback,
                            cell.name + ": input pin " + input_pin.name +
                                " capacitance uses maximum transition capacitance");
      } else if (library.default_input_pin_capacitance) {
        input_loads.push_back(*library.default_input_pin_capacitance);
        diagnostics.warning(DiagnosticId::kMappingFallback,
                            cell.name + ": input pin " + input_pin.name +
                                " capacitance uses Liberty library default");
      } else {
        input_loads.push_back(1.0);
        diagnostics.warning(DiagnosticId::kMappingFallback, cell.name + ": input pin " +
                                                                input_pin.name +
                                                                " capacitance uses unit value 1");
      }
    }
    std::vector<double> max_loads;
    max_loads.reserve(output_pin_ids.size());
    for (liberty::PinId output_pin_id : output_pin_ids) {
      const auto &output_pin = cell.pins[output_pin_id];
      if (output_pin.max_capacitance) {
        max_loads.push_back(*output_pin.max_capacitance);
      } else if (library.default_max_capacitance) {
        max_loads.push_back(*library.default_max_capacitance);
        diagnostics.warning(DiagnosticId::kMappingFallback,
                            cell.name + ": output pin " + output_pin.name +
                                " max_capacitance uses Liberty library default");
      } else {
        max_loads.push_back(999.0);
        diagnostics.warning(DiagnosticId::kMappingFallback,
                            cell.name + ": output pin " + output_pin.name +
                                " max_capacitance uses GENLIB unconstrained value 999");
      }
    }
    std::vector<std::vector<std::optional<LinearTimingArc>>> fitted_timing_arcs;
    fitted_timing_arcs.reserve(output_pin_ids.size());
    for (liberty::PinId output_pin_id : output_pin_ids) {
      const auto &output_pin = cell.pins[output_pin_id];
      std::vector<std::optional<LinearTimingArc>> output_arcs;
      output_arcs.reserve(input_pin_ids.size());
      for (liberty::PinId input_pin_id : input_pin_ids) {
        auto fitted = fit_timing_arc(output_pin, input_pin_id, options.input_slew);
        if (!fitted) {
          LinearTimingArc timing;
          timing.has_rise = true;
          timing.has_fall = true;
          timing.rise_block_delay = 1.0;
          timing.fall_block_delay = 1.0;
          output_arcs.push_back(timing);
          diagnostics.warning(DiagnosticId::kMappingFallback,
                              cell.name + ": " + output_pin.name + " <- " +
                                  cell.pins[input_pin_id].name + " timing uses unit delay 1");
          continue;
        }
        const std::string arc_name =
            cell.name + ": " + output_pin.name + " <- " + cell.pins[input_pin_id].name;
        if (fitted->rise_from_fall) {
          diagnostics.warning(DiagnosticId::kMappingFallback,
                              arc_name + " rise delay uses fall timing tables");
        }
        if (fitted->fall_from_rise) {
          diagnostics.warning(DiagnosticId::kMappingFallback,
                              arc_name + " fall delay uses rise timing tables");
        }
        output_arcs.push_back(std::move(fitted->arc));
      }
      fitted_timing_arcs.push_back(std::move(output_arcs));
    }
    std::vector<std::string> output_names;
    std::vector<std::string> output_functions;
    std::vector<double> output_areas;
    std::vector<std::vector<boop::CellLibrary::TimingArc>> boop_timing_arcs;
    for (liberty::PinId output_pin_id : output_pin_ids) {
      const auto &output_pin = cell.pins[output_pin_id];
      output_names.push_back(output_pin.name);
      output_functions.push_back(*output_pin.function);
      output_areas.push_back(area);
      std::vector<boop::CellLibrary::TimingArc> output_timing_arcs;
      const size_t output = boop_timing_arcs.size();
      output_timing_arcs.reserve(input_pin_ids.size());
      for (const auto &timing : fitted_timing_arcs[output]) {
        output_timing_arcs.push_back(
            {timing ? boop_phase(timing->sense) : boop::CellLibrary::UNKNOWN,
             timing && timing->has_rise ? timing->rise_block_delay : 0.0,
             timing && timing->has_rise ? timing->rise_fanout_delay : 0.0,
             timing && timing->has_fall ? timing->fall_block_delay : 0.0,
             timing && timing->has_fall ? timing->fall_fanout_delay : 0.0});
      }
      boop_timing_arcs.push_back(std::move(output_timing_arcs));
    }
    cells_.AddCell(cell.name, std::move(input_names), std::move(input_loads),
                   std::move(output_names), std::move(max_loads), std::move(output_functions),
                   std::move(output_areas), std::move(boop_timing_arcs));
  }
}

size_t Library::cell_count() const {
  return static_cast<size_t>(cells_.GetNumCells());
}

std::span<const Library::ExcludedCell> Library::excluded_cells() const {
  return excluded_cells_;
}

const boop::CellLibrary &Library::cells() const {
  return cells_;
}

void Library::write_genlib(std::ostream &output) const {
  output << "GATE _const0_ 0 z=CONST0;\n"
            "GATE _const1_ 0 z=CONST1;\n";
  const auto flags = output.flags();
  const auto precision = output.precision();
  output << std::fixed << std::setprecision(4);
  for (int cell = 0; cell < cells_.GetNumCells(); ++cell) {
    if (cells_.GetNumOutputs(cell) != 1 || cells_.GetNumInputs(cell) == 0) {
      continue;
    }
    output << "GATE " << cells_.GetCellName(cell) << ' ' << cells_.GetOutputArea(cell, 0) << ' '
           << cells_.GetOutputName(cell, 0) << '=' << cells_.GetOutputFunction(cell, 0) << ";\n";
    for (int input = 0; input < cells_.GetNumInputs(cell); ++input) {
      const auto &arc = cells_.GetTimingArc(cell, 0, input);
      std::string_view phase = "UNKNOWN";
      if (arc.phase == boop::CellLibrary::INV) {
        phase = "INV";
      } else if (arc.phase == boop::CellLibrary::NONINV) {
        phase = "NONINV";
      }
      output << "\tPIN " << cells_.GetInputName(cell, input) << ' ' << phase << ' '
             << cells_.GetInputLoad(cell, input) << ' ' << cells_.GetMaxLoad(cell, 0) << ' '
             << arc.dRiseBlockDelay << ' ' << arc.dRiseFanoutDelay << ' ' << arc.dFallBlockDelay
             << ' ' << arc.dFallFanoutDelay << '\n';
    }
  }
  output.flags(flags);
  output.precision(precision);
}

} // namespace abys::mapping
