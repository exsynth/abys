#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "abys/infra/diagnostics.h"
#include "abys/liberty/ast.h"

namespace abys::liberty {

using PinId = uint32_t;

enum class PinDirection { kUnknown, kInput, kOutput, kInout, kInternal };
enum class TableVariable {
  kUnknown,
  kOther,
  kInputNetTransition,
  kInputTransitionTime,
  kTotalOutputNetCapacitance,
  kRelatedPinTransition,
  kConstrainedPinTransition,
};
enum class TimingSense { kUnknown, kPositiveUnate, kNegativeUnate, kNonUnate };

struct TimingTable {
  std::string template_name;
  TableVariable variable_1 = TableVariable::kUnknown;
  TableVariable variable_2 = TableVariable::kUnknown;
  std::string variable_1_name;
  std::string variable_2_name;
  std::vector<double> index_1;
  std::vector<double> index_2;
  std::vector<double> values;
};

struct TimingArc {
  std::vector<PinId> related_pin_ids;
  TimingSense sense = TimingSense::kUnknown;
  std::optional<std::string> type;
  std::optional<std::string> when;
  std::optional<TimingTable> cell_rise;
  std::optional<TimingTable> cell_fall;
  std::optional<TimingTable> rise_transition;
  std::optional<TimingTable> fall_transition;
  std::vector<Attribute> additional_attributes;
  std::vector<Group> additional_groups;
};

struct Pin {
  std::string name;
  PinDirection direction = PinDirection::kUnknown;
  std::optional<std::string> function;
  std::optional<std::string> three_state;
  std::optional<double> capacitance;
  std::optional<double> rise_capacitance;
  std::optional<double> fall_capacitance;
  std::optional<double> max_capacitance;
  std::vector<TimingArc> timing_arcs;
  std::vector<Attribute> additional_attributes;
  std::vector<Group> additional_groups;
};

struct Cell {
  std::string name;
  std::optional<double> area;
  std::vector<Pin> pins;
  std::vector<Group> sequential_groups;
  std::vector<Attribute> additional_attributes;
  std::vector<Group> additional_groups;
  bool sequential = false;
};

struct Unit {
  std::string multiplier;
  std::string symbol;
};

struct CellLibrary {
  std::string name;
  std::optional<Unit> time_unit;
  std::optional<Unit> capacitive_load_unit;
  std::optional<double> default_input_pin_capacitance;
  std::optional<double> default_max_capacitance;
  std::vector<Group> table_template_groups;
  std::vector<Cell> cells;
  std::vector<Attribute> additional_attributes;
  std::vector<Group> additional_groups;
};

std::optional<CellLibrary> build_cell_library(Ast ast, Diagnostics &diagnostics,
                                              std::string_view source_name = {});

} // namespace abys::liberty
