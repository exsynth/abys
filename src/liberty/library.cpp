#include "abys/liberty/library.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <charconv>
#include <cmath>
#include <sstream>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace abys::liberty {
namespace {

struct TableTemplate {
  std::string name;
  TableVariable variable_1 = TableVariable::kUnknown;
  TableVariable variable_2 = TableVariable::kUnknown;
  std::string variable_1_name;
  std::string variable_2_name;
  std::vector<double> index_1;
  std::vector<double> index_2;
};

const Attribute *find_attribute(const Group &group, std::string_view name) {
  for (const auto &attribute : group.attributes) {
    if (attribute.name == name) {
      return &attribute;
    }
  }
  return nullptr;
}

std::vector<Attribute> additional_attributes(const Group &group,
                                             std::initializer_list<std::string_view> known_names) {
  std::vector<Attribute> result;
  for (const auto &attribute : group.attributes) {
    if (std::ranges::find(known_names, attribute.name) == known_names.end()) {
      result.push_back(attribute);
    }
  }
  return result;
}

std::optional<std::string> attribute_text(const Group &group, std::string_view name) {
  const Attribute *attribute = find_attribute(group, name);
  if (attribute == nullptr || attribute->values.size() != 1) {
    return std::nullopt;
  }
  return attribute->values.front().text;
}

std::optional<double> attribute_number(const Group &group, std::string_view name) {
  const auto text = attribute_text(group, name);
  if (!text) {
    return std::nullopt;
  }
  double result;
  const auto conversion = std::from_chars(text->data(), text->data() + text->size(), result);
  if (conversion.ec != std::errc() || conversion.ptr != text->data() + text->size()) {
    return std::nullopt;
  }
  if (!std::isfinite(result)) {
    return std::nullopt;
  }
  return result;
}

bool nonnegative_attribute_number(const Group &group, std::string_view name,
                                  std::optional<double> &result, std::string &message) {
  if (find_attribute(group, name) == nullptr) {
    return true;
  }
  result = attribute_number(group, name);
  if (!result || *result < 0.0) {
    message = "invalid " + std::string(name);
    return false;
  }
  return true;
}

std::optional<std::vector<double>> number_list(std::string_view text) {
  std::vector<double> result;
  while (!text.empty()) {
    const size_t comma = text.find(',');
    std::string_view item = text.substr(0, comma);
    while (!item.empty() && std::isspace(static_cast<unsigned char>(item.front()))) {
      item.remove_prefix(1);
    }
    while (!item.empty() && std::isspace(static_cast<unsigned char>(item.back()))) {
      item.remove_suffix(1);
    }
    double value;
    const auto conversion = std::from_chars(item.data(), item.data() + item.size(), value);
    if (item.empty() || conversion.ec != std::errc() ||
        conversion.ptr != item.data() + item.size() || !std::isfinite(value)) {
      return std::nullopt;
    }
    result.push_back(value);
    if (comma == std::string_view::npos) {
      break;
    }
    text.remove_prefix(comma + 1);
    if (text.empty()) {
      return std::nullopt;
    }
  }
  return result;
}

std::optional<std::vector<double>> attribute_numbers(const Group &group, std::string_view name) {
  const Attribute *attribute = find_attribute(group, name);
  if (attribute == nullptr || attribute->values.size() != 1) {
    return std::nullopt;
  }
  return number_list(attribute->values.front().text);
}

bool valid_unit_symbol(std::string_view symbol, char dimension) {
  return (symbol.size() == 1 && symbol.front() == dimension) ||
         (symbol.size() == 2 && symbol.back() == dimension &&
          (symbol.front() == 'm' || symbol.front() == 'u' || symbol.front() == 'n' ||
           symbol.front() == 'p' || symbol.front() == 'f'));
}

std::optional<Unit> simple_unit(const Group &group, std::string_view name, char dimension) {
  const auto text = attribute_text(group, name);
  if (!text) {
    return std::nullopt;
  }
  double multiplier;
  const auto conversion = std::from_chars(text->data(), text->data() + text->size(), multiplier);
  const std::string_view symbol(conversion.ptr,
                                static_cast<size_t>(text->data() + text->size() - conversion.ptr));
  if (conversion.ec != std::errc() || !std::isfinite(multiplier) || multiplier <= 0.0 ||
      !valid_unit_symbol(symbol, dimension)) {
    return std::nullopt;
  }
  return Unit{std::string(text->data(), conversion.ptr), std::string(symbol)};
}

std::optional<Unit> complex_unit(const Group &group, std::string_view name, char dimension) {
  const Attribute *attribute = find_attribute(group, name);
  if (attribute == nullptr || attribute->values.size() != 2) {
    return std::nullopt;
  }
  const std::string &multiplier_text = attribute->values.front().text;
  double multiplier;
  const auto conversion = std::from_chars(
      multiplier_text.data(), multiplier_text.data() + multiplier_text.size(), multiplier);
  if (conversion.ec != std::errc() ||
      conversion.ptr != multiplier_text.data() + multiplier_text.size() ||
      !std::isfinite(multiplier) || multiplier <= 0.0 ||
      !valid_unit_symbol(attribute->values[1].text, dimension)) {
    return std::nullopt;
  }
  return Unit{multiplier_text, attribute->values[1].text};
}

TableVariable table_variable(const Group &group, std::string_view name, std::string &other_name) {
  const auto variable = attribute_text(group, name);
  if (!variable) {
    return TableVariable::kUnknown;
  }
  if (*variable == "input_net_transition") {
    return TableVariable::kInputNetTransition;
  }
  if (*variable == "input_transition_time") {
    return TableVariable::kInputTransitionTime;
  }
  if (*variable == "total_output_net_capacitance") {
    return TableVariable::kTotalOutputNetCapacitance;
  }
  if (*variable == "related_pin_transition") {
    return TableVariable::kRelatedPinTransition;
  }
  if (*variable == "constrained_pin_transition") {
    return TableVariable::kConstrainedPinTransition;
  }
  other_name = *variable;
  return TableVariable::kOther;
}

TimingSense timing_sense(const Group &group) {
  const auto sense = attribute_text(group, "timing_sense");
  if (!sense) {
    return TimingSense::kUnknown;
  }
  if (*sense == "positive_unate") {
    return TimingSense::kPositiveUnate;
  }
  if (*sense == "negative_unate") {
    return TimingSense::kNegativeUnate;
  }
  if (*sense == "non_unate") {
    return TimingSense::kNonUnate;
  }
  return TimingSense::kUnknown;
}

bool strictly_increasing(const std::vector<double> &values) {
  return std::adjacent_find(values.begin(), values.end(),
                            [](double lhs, double rhs) { return lhs >= rhs; }) == values.end();
}

bool parse_table_template(const Group &group, TableTemplate &result, std::string &message) {
  if (group.arguments.size() != 1) {
    message = "expected one lookup-table template name";
    return false;
  }
  result.name = group.arguments.front().text;
  result.variable_1 = table_variable(group, "variable_1", result.variable_1_name);
  result.variable_2 = table_variable(group, "variable_2", result.variable_2_name);
  const auto index_1 = attribute_numbers(group, "index_1");
  if (result.variable_1 == TableVariable::kUnknown || !index_1 || index_1->empty() ||
      !strictly_increasing(*index_1)) {
    message = "invalid first axis in lookup-table template " + result.name;
    return false;
  }
  result.index_1 = *index_1;
  if (result.variable_2 != TableVariable::kUnknown) {
    const auto index_2 = attribute_numbers(group, "index_2");
    if (!index_2 || index_2->empty() || !strictly_increasing(*index_2)) {
      message = "invalid second axis in lookup-table template " + result.name;
      return false;
    }
    result.index_2 = *index_2;
  }
  return true;
}

bool parse_timing_table(const Group &group,
                        const std::unordered_map<std::string, size_t> &template_ids,
                        const std::vector<TableTemplate> &templates, TimingTable &result,
                        std::string &message) {
  if (group.arguments.size() != 1) {
    message = "expected one lookup-table template reference";
    return false;
  }
  if (group.arguments.front().text == "scalar") {
    const Attribute *values = find_attribute(group, "values");
    if (values == nullptr || values->values.size() != 1) {
      message = "invalid scalar values in " + group.name;
      return false;
    }
    const auto parsed = number_list(values->values.front().text);
    if (!parsed || parsed->size() != 1) {
      message = "invalid scalar values in " + group.name;
      return false;
    }
    result.values = *parsed;
    return true;
  }
  const auto template_id = template_ids.find(group.arguments.front().text);
  if (template_id == template_ids.end()) {
    message = "unknown lookup-table template: " + group.arguments.front().text;
    return false;
  }
  result.template_name = group.arguments.front().text;
  const auto &table_template = templates[template_id->second];
  result.variable_1 = table_template.variable_1;
  result.variable_2 = table_template.variable_2;
  result.variable_1_name = table_template.variable_1_name;
  result.variable_2_name = table_template.variable_2_name;
  if (find_attribute(group, "index_1") != nullptr) {
    const auto index = attribute_numbers(group, "index_1");
    if (!index) {
      message = "invalid first index in " + group.name;
      return false;
    }
    result.index_1 = *index;
  } else {
    result.index_1 = table_template.index_1;
  }
  if (find_attribute(group, "index_2") != nullptr) {
    const auto index = attribute_numbers(group, "index_2");
    if (!index) {
      message = "invalid second index in " + group.name;
      return false;
    }
    result.index_2 = *index;
  } else {
    result.index_2 = table_template.index_2;
  }
  if (result.index_1.empty() || !strictly_increasing(result.index_1) ||
      (!result.index_2.empty() && !strictly_increasing(result.index_2))) {
    message = "invalid lookup-table indices in " + group.name;
    return false;
  }

  const Attribute *values = find_attribute(group, "values");
  if (values == nullptr || values->values.empty()) {
    message = "missing values in " + group.name;
    return false;
  }
  for (const auto &row : values->values) {
    const auto parsed = number_list(row.text);
    if (!parsed) {
      message = "invalid values in " + group.name;
      return false;
    }
    result.values.insert(result.values.end(), parsed->begin(), parsed->end());
  }
  const size_t expected_size =
      result.index_1.size() * (result.index_2.empty() ? 1 : result.index_2.size());
  if (result.values.size() != expected_size) {
    message = "lookup-table dimensions do not match values in " + group.name;
    return false;
  }
  return true;
}

std::vector<std::string> split_names(std::string_view names) {
  std::vector<std::string> result;
  std::istringstream stream{std::string(names)};
  std::string name;
  while (stream >> name) {
    result.push_back(std::move(name));
  }
  return result;
}

bool parse_timing_arc(const Group &group,
                      const std::unordered_map<std::string, size_t> &template_ids,
                      const std::vector<TableTemplate> &templates,
                      const std::unordered_map<std::string, PinId> &pin_ids, TimingArc &result,
                      std::string &message) {
  const auto related_pin = attribute_text(group, "related_pin");
  if (!related_pin) {
    message = "missing related_pin in timing group";
    return false;
  }
  const std::vector<std::string> related_pins = split_names(*related_pin);
  if (related_pins.empty()) {
    message = "missing related_pin in timing group";
    return false;
  }
  for (const auto &name : related_pins) {
    const auto pin_id = pin_ids.find(name);
    if (pin_id == pin_ids.end()) {
      message = "unknown related pin: " + name;
      return false;
    }
    result.related_pin_ids.push_back(pin_id->second);
  }
  result.sense = timing_sense(group);
  result.type = attribute_text(group, "timing_type");
  result.when = attribute_text(group, "when");
  result.additional_attributes =
      additional_attributes(group, {"related_pin", "timing_sense", "timing_type", "when"});
  for (const auto &table_group : group.groups) {
    std::optional<TimingTable> *table = nullptr;
    if (table_group.name == "cell_rise") {
      table = &result.cell_rise;
    } else if (table_group.name == "cell_fall") {
      table = &result.cell_fall;
    } else if (table_group.name == "rise_transition") {
      table = &result.rise_transition;
    } else if (table_group.name == "fall_transition") {
      table = &result.fall_transition;
    } else {
      result.additional_groups.push_back(table_group);
      continue;
    }
    if (*table) {
      message = "duplicate " + table_group.name + " table in timing group";
      return false;
    }
    TimingTable parsed;
    if (!parse_timing_table(table_group, template_ids, templates, parsed, message)) {
      return false;
    }
    *table = std::move(parsed);
  }
  return true;
}

PinDirection pin_direction(const Group &group) {
  const auto direction = attribute_text(group, "direction");
  if (!direction) {
    return PinDirection::kUnknown;
  }
  if (*direction == "input") {
    return PinDirection::kInput;
  }
  if (*direction == "output") {
    return PinDirection::kOutput;
  }
  if (*direction == "inout") {
    return PinDirection::kInout;
  }
  if (*direction == "internal") {
    return PinDirection::kInternal;
  }
  return PinDirection::kUnknown;
}

bool is_sequential(const Group &cell) {
  for (const auto &group : cell.groups) {
    if (group.name == "ff" || group.name == "ff_bank" || group.name == "latch" ||
        group.name == "latch_bank" || group.name == "statetable") {
      return true;
    }
  }
  return false;
}

std::optional<CellLibrary> error(Diagnostics &diagnostics, std::string_view source_name,
                                 size_t line, std::string message) {
  std::string detail;
  if (!source_name.empty()) {
    detail = std::string(source_name) + ':';
  }
  detail += std::to_string(line) + ": " + std::move(message);
  diagnostics.error(DiagnosticId::kLibertySemanticError, std::move(detail));
  return std::nullopt;
}

} // namespace

std::optional<CellLibrary> build_cell_library(Ast ast, Diagnostics &diagnostics,
                                              std::string_view source_name) {
  if (ast.groups.size() != 1 || ast.groups.front().name != "library" ||
      ast.groups.front().arguments.size() != 1) {
    return error(diagnostics, source_name, ast.groups.empty() ? 0 : ast.groups.front().line,
                 "expected one library group with one name");
  }

  CellLibrary result;
  const Group &library_group = ast.groups.front();
  result.name = library_group.arguments.front().text;
  if (const Attribute *attribute = find_attribute(library_group, "time_unit")) {
    result.time_unit = simple_unit(library_group, attribute->name, 's');
    if (!result.time_unit) {
      return error(diagnostics, source_name, attribute->line, "invalid time_unit");
    }
  }
  if (const Attribute *attribute = find_attribute(library_group, "capacitive_load_unit")) {
    result.capacitive_load_unit = complex_unit(library_group, attribute->name, 'f');
    if (!result.capacitive_load_unit) {
      return error(diagnostics, source_name, attribute->line, "invalid capacitive_load_unit");
    }
  }
  if (const Attribute *attribute = find_attribute(library_group, "default_input_pin_cap")) {
    result.default_input_pin_capacitance = attribute_number(library_group, attribute->name);
    if (!result.default_input_pin_capacitance || *result.default_input_pin_capacitance < 0.0) {
      return error(diagnostics, source_name, attribute->line, "invalid default_input_pin_cap");
    }
  }
  if (const Attribute *attribute = find_attribute(library_group, "default_max_capacitance")) {
    result.default_max_capacitance = attribute_number(library_group, attribute->name);
    if (!result.default_max_capacitance || *result.default_max_capacitance < 0.0) {
      return error(diagnostics, source_name, attribute->line, "invalid default_max_capacitance");
    }
  }
  result.additional_attributes =
      additional_attributes(library_group, {"time_unit", "capacitive_load_unit",
                                            "default_input_pin_cap", "default_max_capacitance"});
  std::string message;
  std::unordered_map<std::string, size_t> template_ids;
  std::vector<TableTemplate> table_templates;
  for (const auto &group : library_group.groups) {
    if (group.name != "lu_table_template") {
      continue;
    }
    result.table_template_groups.push_back(group);
    TableTemplate table_template;
    message.clear();
    if (!parse_table_template(group, table_template, message)) {
      return error(diagnostics, source_name, group.line, std::move(message));
    }
    if (!template_ids.emplace(table_template.name, table_templates.size()).second) {
      return error(diagnostics, source_name, group.line,
                   "duplicate lookup-table template: " + table_template.name);
    }
    table_templates.push_back(std::move(table_template));
  }
  for (const auto &group : library_group.groups) {
    if (group.name != "lu_table_template" && group.name != "cell") {
      result.additional_groups.push_back(group);
    }
  }

  std::unordered_set<std::string> cell_names;
  for (const auto &cell_group : library_group.groups) {
    if (cell_group.name != "cell") {
      continue;
    }
    if (cell_group.arguments.size() != 1) {
      return error(diagnostics, source_name, cell_group.line, "expected one cell name");
    }
    Cell cell;
    cell.name = cell_group.arguments.front().text;
    if (!cell_names.insert(cell.name).second) {
      return error(diagnostics, source_name, cell_group.line, "duplicate cell: " + cell.name);
    }
    std::string message;
    if (!nonnegative_attribute_number(cell_group, "area", cell.area, message)) {
      return error(diagnostics, source_name, cell_group.line,
                   std::move(message) + " in cell " + cell.name);
    }
    cell.additional_attributes = additional_attributes(cell_group, {"area"});
    cell.sequential = is_sequential(cell_group);
    if (cell.sequential) {
      for (const auto &group : cell_group.groups) {
        if (group.name == "ff" || group.name == "ff_bank" || group.name == "latch" ||
            group.name == "latch_bank" || group.name == "statetable") {
          cell.sequential_groups.push_back(group);
        }
      }
    }

    std::unordered_map<std::string, PinId> pin_ids;
    for (const auto &pin_group : cell_group.groups) {
      if (pin_group.name != "pin") {
        continue;
      }
      if (pin_group.arguments.size() != 1) {
        return error(diagnostics, source_name, pin_group.line,
                     "expected one pin name in cell " + cell.name);
      }
      Pin pin;
      pin.name = pin_group.arguments.front().text;
      if (!pin_ids.emplace(pin.name, cell.pins.size()).second) {
        return error(diagnostics, source_name, pin_group.line,
                     "duplicate pin " + pin.name + " in cell " + cell.name);
      }
      pin.direction = pin_direction(pin_group);
      pin.function = attribute_text(pin_group, "function");
      pin.three_state = attribute_text(pin_group, "three_state");
      pin.additional_attributes = additional_attributes(
          pin_group, {"direction", "function", "three_state", "capacitance", "rise_capacitance",
                      "fall_capacitance", "max_capacitance"});
      if (!nonnegative_attribute_number(pin_group, "capacitance", pin.capacitance, message) ||
          !nonnegative_attribute_number(pin_group, "rise_capacitance", pin.rise_capacitance,
                                        message) ||
          !nonnegative_attribute_number(pin_group, "fall_capacitance", pin.fall_capacitance,
                                        message) ||
          !nonnegative_attribute_number(pin_group, "max_capacitance", pin.max_capacitance,
                                        message)) {
        return error(diagnostics, source_name, pin_group.line,
                     std::move(message) + " on pin " + pin.name + " in cell " + cell.name);
      }
      cell.pins.push_back(std::move(pin));
    }
    for (const auto &group : cell_group.groups) {
      if (group.name != "pin" && group.name != "ff" && group.name != "ff_bank" &&
          group.name != "latch" && group.name != "latch_bank" && group.name != "statetable") {
        cell.additional_groups.push_back(group);
      }
    }
    for (const auto &pin_group : cell_group.groups) {
      if (pin_group.name != "pin") {
        continue;
      }
      Pin &pin = cell.pins[pin_ids.at(pin_group.arguments.front().text)];
      for (const auto &group : pin_group.groups) {
        if (group.name != "timing") {
          pin.additional_groups.push_back(group);
        }
      }
      for (const auto &timing_group : pin_group.groups) {
        if (timing_group.name != "timing") {
          continue;
        }
        TimingArc timing_arc;
        std::string message;
        if (!parse_timing_arc(timing_group, template_ids, table_templates, pin_ids, timing_arc,
                              message)) {
          return error(diagnostics, source_name, timing_group.line, std::move(message));
        }
        pin.timing_arcs.push_back(std::move(timing_arc));
      }
    }
    result.cells.push_back(std::move(cell));
  }
  return result;
}

} // namespace abys::liberty
