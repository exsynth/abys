#include "abys/liberty/writer.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <limits>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace abys::liberty {
namespace {

void write_quoted(std::ostream &output, std::string_view value) {
  output << '"';
  for (const char character : value) {
    if (character == '"' || character == '\\') {
      output << '\\';
    }
    output << character;
  }
  output << '"';
}

void write_value(std::ostream &output, const Value &value) {
  if (value.quoted) {
    write_quoted(output, value.text);
  } else {
    output << value.text;
  }
}

void write_values(std::ostream &output, const std::vector<Value> &values) {
  for (size_t index = 0; index < values.size(); ++index) {
    if (index != 0) {
      output << ", ";
    }
    write_value(output, values[index]);
  }
}

void write_attribute(std::ostream &output, const Attribute &attribute, size_t depth) {
  output << std::string(depth * 2, ' ') << attribute.name;
  if (attribute.kind == AttributeKind::kComplex) {
    output << " (";
    write_values(output, attribute.values);
    output << ");\n";
    return;
  }
  assert(attribute.values.size() == 1);
  output << (attribute.kind == AttributeKind::kVariable ? " = " : " : ");
  write_value(output, attribute.values.front());
  output << ";\n";
}

void write_group(std::ostream &output, const Group &group, size_t depth) {
  const std::string indent(depth * 2, ' ');
  output << indent << group.name << " (";
  write_values(output, group.arguments);
  output << ") {\n";
  struct Statement {
    size_t order;
    const Attribute *attribute;
    const Group *group;
  };
  std::vector<Statement> statements;
  statements.reserve(group.attributes.size() + group.groups.size());
  for (const auto &attribute : group.attributes) {
    statements.push_back({attribute.order, &attribute, nullptr});
  }
  for (const auto &child : group.groups) {
    statements.push_back({child.order, nullptr, &child});
  }
  std::ranges::stable_sort(statements, {}, &Statement::order);
  for (const auto &statement : statements) {
    if (statement.group != nullptr) {
      write_group(output, *statement.group, depth + 1);
      continue;
    }
    write_attribute(output, *statement.attribute, depth + 1);
  }
  output << indent << "}\n";
}

std::string_view direction_name(PinDirection direction) {
  switch (direction) {
  case PinDirection::kInput:
    return "input";
  case PinDirection::kOutput:
    return "output";
  case PinDirection::kInout:
    return "inout";
  case PinDirection::kInternal:
    return "internal";
  case PinDirection::kUnknown:
    return "";
  }
  return "";
}

std::string_view variable_name(TableVariable variable, std::string_view other_name) {
  switch (variable) {
  case TableVariable::kInputNetTransition:
    return "input_net_transition";
  case TableVariable::kInputTransitionTime:
    return "input_transition_time";
  case TableVariable::kTotalOutputNetCapacitance:
    return "total_output_net_capacitance";
  case TableVariable::kRelatedPinTransition:
    return "related_pin_transition";
  case TableVariable::kConstrainedPinTransition:
    return "constrained_pin_transition";
  case TableVariable::kOther:
    return other_name;
  case TableVariable::kUnknown:
    return "";
  }
  return "";
}

bool can_write(const TimingTable &table) {
  return table.index_1.empty() ||
         (!variable_name(table.variable_1, table.variable_1_name).empty() &&
          (table.index_2.empty() ||
           !variable_name(table.variable_2, table.variable_2_name).empty()));
}

std::string_view sense_name(TimingSense sense) {
  switch (sense) {
  case TimingSense::kPositiveUnate:
    return "positive_unate";
  case TimingSense::kNegativeUnate:
    return "negative_unate";
  case TimingSense::kNonUnate:
    return "non_unate";
  case TimingSense::kUnknown:
    return "";
  }
  return "";
}

void write_numbers(std::ostream &output, const std::vector<double> &values, size_t begin,
                   size_t end) {
  output << '"';
  for (size_t index = begin; index < end; ++index) {
    if (index != begin) {
      output << ", ";
    }
    output << values[index];
  }
  output << '"';
}

void write_table(std::ostream &output, std::string_view kind, const TimingTable &table) {
  output << "        " << kind << " (";
  if (table.index_1.empty()) {
    output << "scalar";
  } else {
    assert(!table.template_name.empty());
    output << table.template_name;
  }
  output << ") {\n";
  if (!table.index_1.empty()) {
    write_numbers(output << "          index_1 (", table.index_1, 0, table.index_1.size());
    output << ");\n";
  }
  if (!table.index_2.empty()) {
    write_numbers(output << "          index_2 (", table.index_2, 0, table.index_2.size());
    output << ");\n";
  }
  output << "          values (";
  if (table.index_2.empty()) {
    write_numbers(output, table.values, 0, table.values.size());
  } else {
    for (size_t row = 0; row < table.index_1.size(); ++row) {
      if (row != 0) {
        output << ", ";
      }
      write_numbers(output, table.values, row * table.index_2.size(),
                    (row + 1) * table.index_2.size());
    }
  }
  output << ");\n        }\n";
}

void write_optional_number(std::ostream &output, std::string_view name,
                           const std::optional<double> &value, size_t depth) {
  if (value) {
    output << std::string(depth * 2, ' ') << name << " : " << *value << ";\n";
  }
}

} // namespace

void write(const CellLibrary &library, std::ostream &output) {
  const auto old_precision = output.precision();
  output << std::setprecision(std::numeric_limits<double>::max_digits10);

  output << "library (" << library.name << ") {\n";
  if (library.time_unit) {
    output << "  time_unit : ";
    write_quoted(output, library.time_unit->multiplier + library.time_unit->symbol);
    output << ";\n";
  }
  if (library.capacitive_load_unit) {
    output << "  capacitive_load_unit (" << library.capacitive_load_unit->multiplier << ", "
           << library.capacitive_load_unit->symbol << ");\n";
  }
  write_optional_number(output, "default_input_pin_cap", library.default_input_pin_capacitance, 1);
  write_optional_number(output, "default_max_capacitance", library.default_max_capacitance, 1);
  for (const auto &attribute : library.additional_attributes) {
    write_attribute(output, attribute, 1);
  }
  for (const auto &group : library.table_template_groups) {
    write_group(output, group, 1);
  }
  for (const auto &group : library.additional_groups) {
    write_group(output, group, 1);
  }
  for (const auto &cell : library.cells) {
    output << "  cell (" << cell.name << ") {\n";
    write_optional_number(output, "area", cell.area, 2);
    for (const auto &attribute : cell.additional_attributes) {
      write_attribute(output, attribute, 2);
    }
    for (const auto &group : cell.sequential_groups) {
      write_group(output, group, 2);
    }
    for (const auto &pin : cell.pins) {
      output << "    pin (" << pin.name << ") {\n";
      const std::string_view direction = direction_name(pin.direction);
      if (!direction.empty()) {
        output << "      direction : " << direction << ";\n";
      }
      if (pin.function) {
        output << "      function : ";
        write_quoted(output, *pin.function);
        output << ";\n";
      }
      if (pin.three_state) {
        output << "      three_state : ";
        write_quoted(output, *pin.three_state);
        output << ";\n";
      }
      write_optional_number(output, "capacitance", pin.capacitance, 3);
      write_optional_number(output, "rise_capacitance", pin.rise_capacitance, 3);
      write_optional_number(output, "fall_capacitance", pin.fall_capacitance, 3);
      write_optional_number(output, "max_capacitance", pin.max_capacitance, 3);
      for (const auto &attribute : pin.additional_attributes) {
        write_attribute(output, attribute, 3);
      }
      for (const auto &arc : pin.timing_arcs) {
        output << "      timing () {\n        related_pin : ";
        std::string related_pins;
        for (const PinId related_pin_id : arc.related_pin_ids) {
          assert(related_pin_id < cell.pins.size());
          if (!related_pins.empty()) {
            related_pins += ' ';
          }
          related_pins += cell.pins[related_pin_id].name;
        }
        write_quoted(output, related_pins);
        output << ";\n";
        const std::string_view sense = sense_name(arc.sense);
        if (!sense.empty()) {
          output << "        timing_sense : " << sense << ";\n";
        }
        if (arc.type) {
          output << "        timing_type : " << *arc.type << ";\n";
        }
        if (arc.when) {
          output << "        when : ";
          write_quoted(output, *arc.when);
          output << ";\n";
        }
        for (const auto &attribute : arc.additional_attributes) {
          write_attribute(output, attribute, 4);
        }
        if (arc.cell_rise && can_write(*arc.cell_rise)) {
          write_table(output, "cell_rise", *arc.cell_rise);
        }
        if (arc.cell_fall && can_write(*arc.cell_fall)) {
          write_table(output, "cell_fall", *arc.cell_fall);
        }
        if (arc.rise_transition && can_write(*arc.rise_transition)) {
          write_table(output, "rise_transition", *arc.rise_transition);
        }
        if (arc.fall_transition && can_write(*arc.fall_transition)) {
          write_table(output, "fall_transition", *arc.fall_transition);
        }
        for (const auto &group : arc.additional_groups) {
          write_group(output, group, 4);
        }
        output << "      }\n";
      }
      for (const auto &group : pin.additional_groups) {
        write_group(output, group, 3);
      }
      output << "    }\n";
    }
    for (const auto &group : cell.additional_groups) {
      write_group(output, group, 2);
    }
    output << "  }\n";
  }
  output << "}\n";
  output.precision(old_precision);
}

} // namespace abys::liberty
