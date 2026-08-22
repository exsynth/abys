#include "abys/ir/tig_transformer.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "abys/ir/expr_builder.h"

namespace abys::ir {

std::optional<ExprGraph::UnpackedProperties>
TigTransformer::get_unpacked_properties(const Tig::Module::Node &node, ExprId id) {
  for (const auto &properties : node.expr_graph.unpacked_properties) {
    if (properties.id == id) {
      return properties;
    }
  }
  return std::nullopt;
}

std::vector<ExprId>
TigTransformer::create_unpacked_element_selects(ExprBuilder &builder, ExprId data,
                                                const ExprGraph::UnpackedProperties &properties) {
  assert(!properties.unpacked_dims.empty());
  const SignalWidth extent = properties.unpacked_dims.front();
  assert(extent > 0);
  assert(extent <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
  std::vector<SignalWidth> remaining_dims(properties.unpacked_dims.begin() + 1,
                                          properties.unpacked_dims.end());
  const SignalWidth element_width =
      remaining_dims.empty() ? properties.width : remaining_dims.front();
  const bool element_sign = remaining_dims.empty() ? properties.sign : false;
  const auto &data_node = builder.get_node(data);
  if (data_node.op == ExprGraph::Op::kGather) {
    assert(data_node.operands.size() == extent);
    return data_node.operands;
  }
  std::vector<ExprId> elements;
  elements.reserve(extent);
  for (SignalWidth element = 0; element < extent; ++element) {
    const ExprId index = builder.find_or_create_const(
        static_cast<BitIndex>(element), ExprBuilder::minimum_unsigned_width(element), false);
    elements.push_back(builder.create_unpacked_select(
        data, index, 0, static_cast<BitIndex>(extent - 1), element_width, element_sign,
        remaining_dims, properties.width, properties.sign));
  }
  return elements;
}

ExprId TigTransformer::materialize_affine_index(ExprBuilder &builder,
                                                const ExprBuilder::AffineIndex &index) {
  SignalWidth width =
      ExprBuilder::minimum_unsigned_width(static_cast<BitIndex>(std::abs(index.offset))) + 1;
  for (const auto &term : index.terms) {
    SignalWidth term_width = builder.get_width(term.index);
    if (!builder.get_sign(term.index)) {
      ++term_width;
    }
    term_width +=
        ExprBuilder::minimum_unsigned_width(static_cast<BitIndex>(std::abs(term.stride))) + 1;
    width = std::max(width, term_width);
  }
  width += ExprBuilder::minimum_unsigned_width(static_cast<BitIndex>(index.terms.size())) + 1;
  ExprId result = builder.find_or_create_const(index.offset, width, true);
  for (const auto &term : index.terms) {
    const ExprId value = builder.create_convert(term.index, width, true);
    const ExprId stride = builder.find_or_create_const(term.stride, width, true);
    result = builder.create_add(result, builder.create_mul(value, stride));
  }
  return result;
}

std::optional<TigTransformer::StridedIndex>
TigTransformer::extract_strided_index(ExprBuilder &builder, ExprId base) {
  const auto &node = builder.get_node(base);
  if (node.op == ExprGraph::Op::kAdd) {
    assert(node.operands.size() == 2);
    for (size_t constant_operand = 0; constant_operand < 2; ++constant_operand) {
      const auto value = builder.try_evaluate(node.operands[constant_operand]);
      if (value && *value >= 0) {
        auto result = extract_strided_index(builder, node.operands[1 - constant_operand]);
        if (result) {
          result->offset += static_cast<SignalWidth>(*value);
          return result;
        }
      }
    }
  }
  if (node.op == ExprGraph::Op::kMul) {
    assert(node.operands.size() == 2);
    for (size_t constant_operand = 0; constant_operand < 2; ++constant_operand) {
      const auto value = builder.try_evaluate(node.operands[constant_operand]);
      if (value && *value > 0) {
        return StridedIndex{node.operands[1 - constant_operand], static_cast<SignalWidth>(*value),
                            0};
      }
    }
  }
  return StridedIndex{base, 1, 0};
}

std::vector<ExprId> TigTransformer::create_barrel_shift(ExprBuilder &builder,
                                                        std::vector<ExprId> lanes, ExprId amount,
                                                        ExprId fill) {
  assert(!lanes.empty());
  assert(lanes.size() <= std::numeric_limits<SignalWidth>::max());
  const SignalWidth width = static_cast<SignalWidth>(lanes.size());
  const SignalWidth amount_width = builder.get_width(amount);
  assert(amount_width <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
  for (SignalWidth stage = 0; stage < amount_width; ++stage) {
    const ExprId select = builder.create_static_range(amount, stage, 1, false);
    const bool distance_representable = stage < std::numeric_limits<size_t>::digits;
    const size_t distance = distance_representable ? size_t{1} << stage : width;
    std::vector<ExprId> next;
    next.reserve(width);
    for (SignalWidth lane = 0; lane < width; ++lane) {
      ExprId shifted = fill;
      if (distance < width && lane + distance < width) {
        shifted = lanes[lane + distance];
      }
      next.push_back(builder.create_mux(select, shifted, lanes[lane]));
    }
    lanes = std::move(next);
  }
  return lanes;
}

void TigTransformer::decompose_unpacked_range(Tig::Module::Node &node, ExprBuilder &builder,
                                              ExprId range_id) {
  const auto range = node.expr_graph.nodes[range_id];
  assert(range.op == ExprGraph::Op::kUnpackedRange);
  assert(range.operands.size() == 2);
  const ExprId data = range.operands[0];
  const ExprId base = range.operands[1];
  const auto properties = get_unpacked_properties(node, range_id);
  const auto data_properties = get_unpacked_properties(node, data);
  assert(properties.has_value());
  assert(data_properties.has_value());
  assert(!properties->unpacked_dims.empty());
  assert(!data_properties->unpacked_dims.empty());
  const SignalWidth slice_width = properties->unpacked_dims.front();
  const SignalWidth extent = data_properties->unpacked_dims.front();
  assert(slice_width > 0);
  assert(extent > 0);

  std::vector<ExprId> elements = create_unpacked_element_selects(builder, data, *data_properties);
  std::vector<SignalWidth> remaining_dims(data_properties->unpacked_dims.begin() + 1,
                                          data_properties->unpacked_dims.end());
  SignalWidth fill_width = data_properties->width;
  for (SignalWidth dimension : remaining_dims) {
    assert(dimension > 0);
    assert(fill_width <= std::numeric_limits<SignalWidth>::max() / dimension);
    fill_width *= dimension;
  }
  ExprId fill = builder.find_or_create_const(std::to_string(fill_width) + "'bx", fill_width,
                                             data_properties->sign);
  if (!remaining_dims.empty()) {
    fill = builder.create_unpacked_fold(fill, remaining_dims, data_properties->width,
                                        data_properties->sign);
  }

  std::vector<ExprId> selected;
  selected.reserve(slice_width);
  if (const auto static_base = builder.try_evaluate(base)) {
    const int64_t base_value = *static_base;
    for (SignalWidth source = 0; source < slice_width; ++source) {
      const int64_t element = base_value + static_cast<int64_t>(source);
      selected.push_back(element >= 0 && element < static_cast<int64_t>(extent)
                             ? elements[static_cast<SignalWidth>(element)]
                             : fill);
    }
  } else {
    const auto strided_index = extract_strided_index(builder, base);
    assert(strided_index.has_value());
    assert(strided_index->stride == 1);
    const ExprId index = strided_index->index;
    const SignalWidth negative_positions = builder.get_sign(index) ? slice_width - 1 : 0;
    assert(negative_positions <= std::numeric_limits<SignalWidth>::max() - extent);
    std::vector<ExprId> lanes(negative_positions, fill);
    lanes.insert(lanes.end(), elements.begin(), elements.end());
    lanes.insert(lanes.end(), slice_width - 1, fill);
    ExprId amount = index;
    assert(strided_index->offset <= std::numeric_limits<SignalWidth>::max() - negative_positions);
    const SignalWidth alignment = strided_index->offset + negative_positions;
    if (alignment > 0) {
      SignalWidth amount_width =
          std::max(builder.get_width(index), ExprBuilder::minimum_unsigned_width(alignment));
      if (builder.get_sign(index)) {
        ++amount_width;
      }
      const ExprId offset = builder.find_or_create_const(static_cast<BitIndex>(alignment),
                                                         amount_width, builder.get_sign(index));
      amount = builder.create_add(index, offset);
    }
    lanes = create_barrel_shift(builder, std::move(lanes), amount, fill);
    selected.assign(lanes.begin(), lanes.begin() + slice_width);
  }

  const ExprId gathered = builder.create_gather(std::move(selected), properties->unpacked_dims,
                                                properties->width, properties->sign);
  const auto gathered_node = node.expr_graph.nodes[gathered];
  auto &replacement = node.expr_graph.nodes[range_id];
  replacement.op = gathered_node.op;
  replacement.width = gathered_node.width;
  replacement.sign = gathered_node.sign;
  replacement.operands = gathered_node.operands;
}

void TigTransformer::decompose_unpacked_sequence(Tig::Module::Node &node, ExprBuilder &builder,
                                                 ExprId sequence_id) {
  const auto sequence = node.expr_graph.nodes[sequence_id];
  assert(sequence.op == ExprGraph::Op::kSequence);
  assert(!sequence.operands.empty());
  const ExprId base = sequence.operands.front();
  assert(base != kInvalidExprId);
  const auto properties = get_unpacked_properties(node, sequence_id);
  assert(properties.has_value());
  assert(!properties->unpacked_dims.empty());
  const SignalWidth extent = properties->unpacked_dims.front();
  assert(extent > 0);
  assert(extent <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));

  struct PendingUpdate {
    ExprId id;
    ExprId enable;
  };
  auto combine_enable = [&](ExprId lhs, ExprId rhs) {
    if (lhs == ExprGraph::constant_one) {
      return rhs;
    }
    if (rhs == ExprGraph::constant_one) {
      return lhs;
    }
    return builder.create_logical_and(lhs, rhs);
  };
  std::vector<SignalWidth> remaining_dims(properties->unpacked_dims.begin() + 1,
                                          properties->unpacked_dims.end());
  ExprId current_value = base;
  auto apply_nested_update = [&](ExprId next, ExprId current,
                                 const std::vector<SignalWidth> &unpacked_dims) {
    if (unpacked_dims.empty()) {
      return next;
    }
    const ExprId nested_sequence =
        builder.create_sequence(next, current, unpacked_dims, properties->width, properties->sign);
    decompose_unpacked_sequence(node, builder, nested_sequence);
    return nested_sequence;
  };

  std::stack<PendingUpdate> pending;
  for (size_t operand = sequence.operands.size() - 1; operand != 0; --operand) {
    assert(sequence.operands[operand] != kInvalidExprId);
    pending.push({sequence.operands[operand], ExprGraph::constant_one});
  }
  while (!pending.empty()) {
    const PendingUpdate current = pending.top();
    pending.pop();
    if (current.id == kInvalidExprId) {
      continue;
    }
    const auto update = node.expr_graph.nodes[current.id];
    switch (update.op) {
    case ExprGraph::Op::kSequence: {
      assert(!update.operands.empty());
      assert(update.operands.front() == kInvalidExprId);
      for (size_t operand = update.operands.size() - 1; operand != 0; --operand) {
        assert(update.operands[operand] != kInvalidExprId);
        pending.push({update.operands[operand], current.enable});
      }
      break;
    }
    case ExprGraph::Op::kMux:
    case ExprGraph::Op::kUnpackedMux: {
      assert(update.operands.size() == 3);
      const ExprId condition = update.operands[0];
      const ExprId then_enable = combine_enable(current.enable, condition);
      const ExprId else_enable =
          combine_enable(current.enable, builder.create_logical_not(condition));
      pending.push({update.operands[2], else_enable});
      pending.push({update.operands[1], then_enable});
      break;
    }
    case ExprGraph::Op::kCase: {
      assert(!update.operands.empty());
      const ExprId selector = update.operands[0];
      std::vector<ExprId> matches;
      std::vector<PendingUpdate> branches;
      size_t operand = 1;
      while (operand + 1 < update.operands.size()) {
        const ExprId match = builder.create_match(selector, update.operands[operand]);
        matches.push_back(match);
        branches.push_back({update.operands[operand + 1], combine_enable(current.enable, match)});
        operand += 2;
      }
      if (operand < update.operands.size()) {
        ExprId default_enable = current.enable;
        if (!matches.empty()) {
          const ExprId any_match =
              matches.size() == 1 ? matches.front() : builder.create_or(std::move(matches));
          default_enable = combine_enable(current.enable, builder.create_logical_not(any_match));
        }
        branches.push_back({update.operands[operand], default_enable});
      }
      for (auto branch = branches.rbegin(); branch != branches.rend(); ++branch) {
        pending.push(*branch);
      }
      break;
    }
    case ExprGraph::Op::kUnpackedAssign: {
      assert(update.operands.size() == 2);
      const ExprId next = update.operands[0];
      const ExprId index = update.operands[1];
      std::vector<ExprId> elements =
          create_unpacked_element_selects(builder, current_value, *properties);
      if (const auto static_index = builder.try_evaluate(index)) {
        if (*static_index >= 0 && static_cast<SignalWidth>(*static_index) < extent) {
          const SignalWidth element = static_cast<SignalWidth>(*static_index);
          const ExprId updated = apply_nested_update(next, elements[element], remaining_dims);
          elements[element] = current.enable == ExprGraph::constant_one
                                  ? updated
                                  : builder.create_mux(current.enable, updated, elements[element]);
        }
      } else {
        for (SignalWidth element = 0; element < extent; ++element) {
          const ExprId element_id = builder.find_or_create_const(
              static_cast<BitIndex>(element), ExprBuilder::minimum_unsigned_width(element), false);
          const ExprId match = builder.create_eq(index, element_id);
          const ExprId enable = combine_enable(current.enable, match);
          const ExprId updated = apply_nested_update(next, elements[element], remaining_dims);
          elements[element] = builder.create_mux(enable, updated, elements[element]);
        }
      }
      current_value = builder.create_gather(std::move(elements), properties->unpacked_dims,
                                            properties->width, properties->sign);
      break;
    }
    case ExprGraph::Op::kUnpackedRangeAssign: {
      assert(update.operands.size() == 3);
      const ExprId next = update.operands[0];
      const ExprId base_index = update.operands[1];
      const auto slice_width_value = builder.try_evaluate(update.operands[2]);
      assert(slice_width_value.has_value());
      assert(*slice_width_value > 0);
      const SignalWidth slice_width = static_cast<SignalWidth>(*slice_width_value);
      const auto next_properties = get_unpacked_properties(node, next);
      assert(next_properties.has_value());
      assert(next_properties->unpacked_dims.front() == slice_width);
      const ExprId current_range = builder.create_unpacked_range(
          current_value, base_index, slice_width, next_properties->unpacked_dims, properties->width,
          properties->sign);
      if (builder.get_node(current_range).op == ExprGraph::Op::kUnpackedRange) {
        decompose_unpacked_range(node, builder, current_range);
      }
      const ExprId updated_range =
          apply_nested_update(next, current_range, next_properties->unpacked_dims);
      const std::vector<ExprId> updated_elements =
          create_unpacked_element_selects(builder, updated_range, *next_properties);
      std::vector<ExprId> current_elements =
          create_unpacked_element_selects(builder, current_value, *properties);
      const ExprId fill = updated_elements.front();

      if (const auto static_base = builder.try_evaluate(base_index)) {
        const int64_t base_value = *static_base;
        for (SignalWidth source = 0; source < slice_width; ++source) {
          const int64_t destination = base_value + static_cast<int64_t>(source);
          if (destination < 0 || destination >= static_cast<int64_t>(extent)) {
            continue;
          }
          const SignalWidth element = static_cast<SignalWidth>(destination);
          current_elements[element] =
              current.enable == ExprGraph::constant_one
                  ? updated_elements[source]
                  : builder.create_mux(current.enable, updated_elements[source],
                                       current_elements[element]);
        }
      } else {
        ExprId amount = base_index;
        const SignalWidth negative_positions = builder.get_sign(base_index) ? slice_width - 1 : 0;
        if (negative_positions > 0) {
          const SignalWidth amount_width =
              std::max(builder.get_width(base_index),
                       ExprBuilder::minimum_unsigned_width(negative_positions) + 1);
          const ExprId offset = builder.find_or_create_const(
              static_cast<BitIndex>(negative_positions), amount_width, true);
          amount = builder.create_add(base_index, offset);
        }
        assert(negative_positions <= std::numeric_limits<SignalWidth>::max() - extent);
        const SignalWidth lane_count = negative_positions + extent;
        std::vector<ExprId> update_lanes(lane_count, fill);
        std::vector<ExprId> mask_lanes(lane_count, ExprGraph::constant_zero);
        for (SignalWidth source = 0; source < slice_width; ++source) {
          update_lanes[source] = updated_elements[source];
          mask_lanes[source] = ExprGraph::constant_one;
        }
        std::reverse(update_lanes.begin(), update_lanes.end());
        std::reverse(mask_lanes.begin(), mask_lanes.end());
        update_lanes = create_barrel_shift(builder, std::move(update_lanes), amount, fill);
        mask_lanes =
            create_barrel_shift(builder, std::move(mask_lanes), amount, ExprGraph::constant_zero);
        std::reverse(update_lanes.begin(), update_lanes.end());
        std::reverse(mask_lanes.begin(), mask_lanes.end());
        for (SignalWidth element = 0; element < extent; ++element) {
          const SignalWidth lane = negative_positions + element;
          const ExprId enable = combine_enable(current.enable, mask_lanes[lane]);
          current_elements[element] =
              builder.create_mux(enable, update_lanes[lane], current_elements[element]);
        }
      }
      current_value = builder.create_gather(std::move(current_elements), properties->unpacked_dims,
                                            properties->width, properties->sign);
      break;
    }
    default: {
      current_value = current.enable == ExprGraph::constant_one
                          ? current.id
                          : builder.create_mux(current.enable, current.id, current_value);
      break;
    }
    }
  }

  const ExprId gathered =
      builder.create_gather(create_unpacked_element_selects(builder, current_value, *properties),
                            properties->unpacked_dims, properties->width, properties->sign);
  const auto gathered_node = node.expr_graph.nodes[gathered];
  auto &replacement = node.expr_graph.nodes[sequence_id];
  replacement.op = gathered_node.op;
  replacement.width = gathered_node.width;
  replacement.sign = gathered_node.sign;
  replacement.operands = gathered_node.operands;
}

ExprId TigTransformer::add_node_input_expr(Tig::Module &module, Tig::Module::Node &node,
                                           Tig::Module::EdgeRef input) {
  assert(node.kind == Tig::Module::NodeKind::kOp);
  assert(node.inputs.size() == node.input_expr_ids.size());
  for (size_t port = 0; port < node.inputs.size(); ++port) {
    if (node.inputs[port].node_id == input.node_id &&
        node.inputs[port].port_idx == input.port_idx) {
      return node.input_expr_ids[port];
    }
  }
  const auto &output = module.nodes.at(input.node_id).outputs.at(input.port_idx);
  const ExprId id = static_cast<ExprId>(node.expr_graph.nodes.size());
  node.expr_graph.nodes.emplace_back();
  auto &expression = node.expr_graph.nodes.back();
  expression.op = ExprGraph::Op::kInput;
  expression.width = output.width;
  expression.sign = output.sign;
  for (const auto &signal : module.signals) {
    if (signal.name == output.name && !signal.unpacked_dims.empty()) {
      expression.width = signal.unpacked_dims.front();
      expression.sign = false;
      node.expr_graph.unpacked_properties.push_back(
          {id, signal.unpacked_dims, signal.width, signal.sign});
      break;
    }
  }
  node.inputs.push_back(input);
  node.input_expr_ids.push_back(id);
  return id;
}

void TigTransformer::clean_op_node(Tig::Module::Node &node) {
  assert(node.kind == Tig::Module::NodeKind::kOp);
  auto &source = node.expr_graph;
  ExprGraph compact;
  std::vector<ExprId> remap(source.nodes.size(), kInvalidExprId);
  remap[ExprGraph::constant_zero] = ExprGraph::constant_zero;
  remap[ExprGraph::constant_one] = ExprGraph::constant_one;

  const auto clone = [&](const auto &self, ExprId old_id) -> ExprId {
    if (old_id == kInvalidExprId) {
      return kInvalidExprId;
    }
    if (remap[old_id] != kInvalidExprId) {
      return remap[old_id];
    }
    auto expression = source.nodes[old_id];
    for (ExprId &operand : expression.operands) {
      operand = self(self, operand);
    }
    const ExprId new_id = static_cast<ExprId>(compact.nodes.size());
    remap[old_id] = new_id;
    compact.nodes.push_back(std::move(expression));
    return new_id;
  };

  for (ExprId &root : node.expr_roots) {
    root = clone(clone, root);
  }
  assert(node.inputs.size() == node.input_expr_ids.size());
  std::vector<Tig::Module::EdgeRef> compact_inputs;
  std::vector<ExprId> compact_input_expr_ids;
  for (size_t port = 0; port < node.inputs.size(); ++port) {
    const ExprId id = node.input_expr_ids[port];
    if (remap[id] != kInvalidExprId) {
      compact_inputs.push_back(node.inputs[port]);
      compact_input_expr_ids.push_back(remap[id]);
    }
  }
  node.inputs = std::move(compact_inputs);
  node.input_expr_ids = std::move(compact_input_expr_ids);

  compact.constants.clear();
  for (auto &constant : source.constants) {
    if (remap[constant.id] != kInvalidExprId) {
      constant.id = remap[constant.id];
      compact.constants.push_back(std::move(constant));
    }
  }
  for (auto &call : source.calls) {
    if (remap[call.id] != kInvalidExprId) {
      call.id = remap[call.id];
      compact.calls.push_back(std::move(call));
    }
  }
  for (auto &properties : source.unpacked_properties) {
    if (remap[properties.id] != kInvalidExprId) {
      properties.id = remap[properties.id];
      compact.unpacked_properties.push_back(std::move(properties));
    }
  }
  source = std::move(compact);
}

Tig::Module::EdgeRef TigTransformer::add_node_output_expr(Tig::Module &module, Tig::NodeId node_id,
                                                          ExprId expr_id, std::string name) {
  auto &node = module.nodes.at(node_id);
  const auto expr = node.expr_graph.nodes.at(expr_id);
  const PortIndex port_idx = static_cast<PortIndex>(node.outputs.size());
  node.outputs.push_back({name, expr.width, expr.sign});
  node.expr_roots.push_back(expr_id);
  node.combs.push_back(true);
  if (!name.empty()) {
    std::vector<SignalWidth> unpacked_dims;
    SignalWidth width = expr.width;
    bool sign = expr.sign;
    for (const auto &properties : node.expr_graph.unpacked_properties) {
      if (properties.id == expr_id) {
        unpacked_dims = properties.unpacked_dims;
        width = properties.width;
        sign = properties.sign;
        break;
      }
    }
    module.signals.push_back({name, std::move(unpacked_dims), width, sign});
  }
  return {node_id, port_idx};
}

std::string TigTransformer::create_temporary_name(Tig::Module &module) const {
  return naming_.transformer_temporary_signal_prefix +
         std::to_string(module.transform_name_count++);
}

TigTransformer::TigTransformer(Tig &design, Diagnostics &diagnostics, const NamingOptions &naming)
    : design_(design), diagnostics_(diagnostics), naming_(naming) {}

void TigTransformer::flatten_subroutine() {
  for (auto &module : design_.modules) {
    for (auto &node : module.nodes) {
      ExprGraph &expr_graph = node.expr_graph;
      if (expr_graph.calls.empty()) {
        continue;
      }
      std::vector<std::vector<SubrId>> call_ancestries(expr_graph.calls.size());
      auto replace_call_with_zero = [&](ExprId call_id, std::string detail) {
        diagnostics_.error(DiagnosticId::kLoweringUnsupportedExpressionReplacedWithZero,
                           std::move(detail));
        auto &call_node = expr_graph.nodes[call_id];
        call_node.op = ExprGraph::Op::kConvert;
        call_node.operands = {ExprGraph::constant_zero};
      };
      for (size_t i = 0; i < expr_graph.calls.size(); ++i) {
        const SubrId subr_id = expr_graph.calls[i].subr_id;
        const ExprId call_id = expr_graph.calls[i].id;
        bool recursive = false;
        for (SubrId ancestor : call_ancestries[i]) {
          if (ancestor == subr_id) {
            recursive = true;
            break;
          }
        }
        if (recursive) {
          diagnostics_.error(DiagnosticId::kTransformRecursiveSubroutineCallReplacedWithZero,
                             expr_graph.calls[i].name);
          auto &call_node = expr_graph.nodes[call_id];
          call_node.op = ExprGraph::Op::kConvert;
          call_node.operands = {ExprGraph::constant_zero};
          continue;
        }
        if (subr_id >= design_.subroutines.size() ||
            design_.subroutines[subr_id].expr_root == kInvalidExprId) {
          replace_call_with_zero(call_id, "unknown subroutine: " + expr_graph.calls[i].name);
          continue;
        }
        const Tig::Subroutine &subr = design_.subroutines[subr_id];
        if (expr_graph.nodes[call_id].operands.size() != subr.inputs.size()) {
          replace_call_with_zero(call_id, "call arity mismatch: " + expr_graph.calls[i].name);
          continue;
        }
        std::unordered_map<ExprId, ExprId> id_map;
        id_map.reserve(subr.expr_graph.nodes.size() + 1);
        id_map.emplace(kInvalidExprId, kInvalidExprId);
        bool call_valid = true;
        assert(subr.inputs.size() == subr.input_expr_ids.size());
        for (size_t j = 0; j < subr.inputs.size(); ++j) {
          id_map.emplace(subr.input_expr_ids[j], expr_graph.nodes[call_id].operands[j]);
        }
        assert(subr.captures.size() == subr.capture_expr_ids.size());
        for (size_t j = 0; j < subr.captures.size(); ++j) {
          const auto capture = subr.captures[j];
          assert(capture.node_id != Tig::kInvalidNodeId);
          assert(&module == &design_.modules.at(subr.module_id));
          const ExprId capture_id = add_node_input_expr(module, node, capture);
          id_map.emplace(subr.capture_expr_ids[j], capture_id);
        }
        if (!call_valid) {
          continue;
        }
        for (const auto &constant : subr.expr_graph.constants) {
          if (constant.id == ExprGraph::constant_zero) {
            id_map.emplace(constant.id, ExprGraph::constant_zero);
          } else if (constant.id == ExprGraph::constant_one) {
            id_map.emplace(constant.id, ExprGraph::constant_one);
          } else {
            const auto &src = subr.expr_graph.nodes[constant.id];
            const ExprId dst_id = static_cast<ExprId>(expr_graph.nodes.size());
            expr_graph.nodes.push_back(src);
            expr_graph.constants.push_back({dst_id, constant.value});
            id_map.emplace(constant.id, dst_id);
          }
        }
        for (ExprId src_id = 0; src_id < static_cast<ExprId>(subr.expr_graph.nodes.size());
             ++src_id) {
          if (id_map.find(src_id) != id_map.end()) {
            continue;
          }
          const auto &src = subr.expr_graph.nodes[src_id];
          std::vector<ExprId> new_ops;
          new_ops.reserve(src.operands.size());
          for (ExprId sop : src.operands) {
            auto mit = id_map.find(sop);
            if (mit == id_map.end()) {
              replace_call_with_zero(call_id,
                                     "subroutine graph is not topologically ordered: " + subr.name);
              call_valid = false;
              break;
            }
            new_ops.push_back(mit->second);
          }
          if (!call_valid) {
            break;
          }
          const ExprId dst_id = static_cast<ExprId>(expr_graph.nodes.size());
          expr_graph.nodes.push_back(src);
          expr_graph.nodes.back().operands = std::move(new_ops);
          id_map.emplace(src_id, dst_id);
          if (src.op == ExprGraph::Op::kCall) {
            for (const auto &src_call : subr.expr_graph.calls) {
              if (src_call.id == src_id) {
                expr_graph.calls.push_back({dst_id, src_call.subr_id, src_call.name});
                auto ancestry = call_ancestries[i];
                ancestry.push_back(subr_id);
                call_ancestries.push_back(std::move(ancestry));
                break;
              }
            }
          }
          for (const auto &src_unpacked_properties : subr.expr_graph.unpacked_properties) {
            if (src_unpacked_properties.id == src_id) {
              expr_graph.unpacked_properties.push_back(
                  {dst_id, src_unpacked_properties.unpacked_dims, src_unpacked_properties.width,
                   src_unpacked_properties.sign});
              break;
            }
          }
        }
        if (!call_valid) {
          continue;
        }
        auto rit = id_map.find(subr.expr_root);
        if (rit == id_map.end()) {
          replace_call_with_zero(call_id, "failed to map subroutine root: " + subr.name);
          continue;
        }
        const ExprId inlined_root = rit->second;
        auto &call_node = expr_graph.nodes[call_id];
        call_node.op = ExprGraph::Op::kConvert; // temporary buffer op
        call_node.operands.clear();
        call_node.operands.push_back(inlined_root);
      }
      expr_graph.calls.clear();
      clean_op_node(node);
    }
  }
  design_.subroutines.clear();
}

std::vector<Tig::Module::EdgeRef>
TigTransformer::create_memory_writes(Tig::Module &module, Tig::NodeId op_id, ExprId sequence_id) {
  auto &op = module.nodes.at(op_id);
  assert(op.kind == Tig::Module::NodeKind::kOp);
  assert(op.expr_graph.nodes.at(sequence_id).op == ExprGraph::Op::kSequence);
  ExprBuilder builder(op.expr_graph, diagnostics_);
  std::vector<SignalWidth> full_extents;
  for (const auto &properties : op.expr_graph.unpacked_properties) {
    if (properties.id == sequence_id) {
      full_extents = properties.unpacked_dims;
      break;
    }
  }
  assert(!full_extents.empty());
  struct LocalWrite {
    Tig::Module::EdgeRef enable;
    Tig::Module::EdgeRef data;
    std::vector<Tig::Module::EdgeRef> region;
    std::vector<bool> region_ranges;
  };
  std::vector<LocalWrite> local_writes;
  const auto conjunction = [&](ExprId a, ExprId b) {
    if (a == ExprGraph::constant_one) {
      return b;
    }
    if (b == ExprGraph::constant_one) {
      return a;
    }
    return builder.create_logical_and(a, b);
  };
  const auto lower_update = [&](const auto &self, ExprId id, ExprId enable) -> void {
    if (id == kInvalidExprId) {
      return;
    }
    const auto expr = op.expr_graph.nodes.at(id);
    if (expr.op == ExprGraph::Op::kSequence) {
      assert(!expr.operands.empty());
      for (size_t operand = 1; operand < expr.operands.size(); ++operand) {
        self(self, expr.operands[operand], enable);
      }
      return;
    }
    if (expr.op == ExprGraph::Op::kMux || expr.op == ExprGraph::Op::kUnpackedMux) {
      const ExprId condition = expr.operands.at(0);
      self(self, expr.operands.at(1), conjunction(enable, condition));
      self(self, expr.operands.at(2), conjunction(enable, builder.create_logical_not(condition)));
      return;
    }
    if (expr.op == ExprGraph::Op::kCase) {
      const ExprId selector = expr.operands.at(0);
      ExprId unmatched = ExprGraph::constant_one;
      size_t operand = 1;
      while (operand + 1 < expr.operands.size()) {
        const ExprId match = builder.create_match(selector, expr.operands[operand]);
        self(self, expr.operands[operand + 1], conjunction(enable, conjunction(unmatched, match)));
        unmatched = conjunction(unmatched, builder.create_logical_not(match));
        operand += 2;
      }
      if (operand < expr.operands.size()) {
        self(self, expr.operands[operand], conjunction(enable, unmatched));
      }
      return;
    }
    std::vector<ExprId> region;
    std::vector<bool> region_ranges;
    ExprId data = id;
    while (op.expr_graph.nodes.at(data).op == ExprGraph::Op::kUnpackedAssign ||
           op.expr_graph.nodes.at(data).op == ExprGraph::Op::kUnpackedRangeAssign) {
      const auto assign = op.expr_graph.nodes.at(data);
      const bool range = assign.op == ExprGraph::Op::kUnpackedRangeAssign;
      region.push_back(assign.operands.at(1));
      region.push_back(range ? assign.operands.at(2) : ExprGraph::constant_one);
      region_ranges.push_back(range);
      data = assign.operands.at(0);
    }
    if (region.empty()) {
      for (SignalWidth extent : full_extents) {
        assert(extent <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
        region.push_back(ExprGraph::constant_zero);
        region.push_back(builder.find_or_create_const(
            static_cast<BitIndex>(extent),
            ExprBuilder::minimum_unsigned_width(static_cast<BitIndex>(extent)), false));
        region_ranges.push_back(true);
      }
    }
    std::vector<Tig::Module::EdgeRef> region_refs;
    for (size_t operand = 0; operand < region.size(); operand += 2) {
      region_refs.push_back(
          add_node_output_expr(module, op_id, region[operand], create_temporary_name(module)));
      region_refs.push_back(add_node_output_expr(module, op_id, region[operand + 1]));
    }
    local_writes.push_back(
        {add_node_output_expr(module, op_id, enable, create_temporary_name(module)),
         add_node_output_expr(module, op_id, data, create_temporary_name(module)),
         std::move(region_refs), std::move(region_ranges)});
  };
  lower_update(lower_update, sequence_id, ExprGraph::constant_one);

  std::vector<Tig::Module::EdgeRef> writes;
  const auto sequence_expr = op.expr_graph.nodes.at(sequence_id);
  for (auto &local : local_writes) {
    const Tig::NodeId write_id = static_cast<Tig::NodeId>(module.nodes.size());
    module.nodes.emplace_back();
    auto &write = module.nodes.back();
    write.kind = Tig::Module::NodeKind::kMemoryWrite;
    write.inputs = {local.enable, local.data};
    write.inputs.insert(write.inputs.end(), local.region.begin(), local.region.end());
    write.memory_region_ranges = std::move(local.region_ranges);
    write.outputs.push_back({"", sequence_expr.width, sequence_expr.sign});
    write.expr_roots.push_back(kInvalidExprId);
    write.combs.push_back(false);
    writes.push_back({write_id, 0});
  }
  return writes;
}

bool TigTransformer::create_memory_reads(Tig::Module &module, Tig::NodeId op_id, ExprId id,
                                         ExprId read_id, std::vector<MemoryReadDimension> *region,
                                         std::vector<bool> &visited,
                                         std::vector<PendingMemoryRead> &pending_reads) {
  if (id == kInvalidExprId) {
    return false;
  }
  if (read_id == kInvalidExprId) {
    if (visited[id]) {
      return false;
    }
    visited[id] = true;
  }
  const auto expr = module.nodes[op_id].expr_graph.nodes[id];
  if (expr.op == ExprGraph::Op::kUnpackedSelect || expr.op == ExprGraph::Op::kUnpackedRange) {
    for (size_t operand = 1; operand < expr.operands.size(); ++operand) {
      create_memory_reads(module, op_id, expr.operands[operand], kInvalidExprId, nullptr, visited,
                          pending_reads);
    }
    std::vector<MemoryReadDimension> new_region;
    if (region == nullptr) {
      read_id = id;
      region = &new_region;
    }
    const SignalWidth extent = expr.op == ExprGraph::Op::kUnpackedSelect ? 1 : expr.width;
    region->push_back({expr.operands.at(1), extent, expr.op == ExprGraph::Op::kUnpackedRange});
    return create_memory_reads(module, op_id, expr.operands.at(0), read_id, region, visited,
                               pending_reads);
  }
  if (read_id != kInvalidExprId) {
    if (expr.op == ExprGraph::Op::kInput) {
      const auto &op = module.nodes[op_id];
      assert(op.inputs.size() == op.input_expr_ids.size());
      for (size_t port = 0; port < op.inputs.size(); ++port) {
        if (op.input_expr_ids[port] != id) {
          continue;
        }
        const auto input = op.inputs[port];
        const auto &source = module.nodes.at(input.node_id);
        const bool extends_read = source.kind == Tig::Module::NodeKind::kMemoryRead;
        if (!extends_read && source.kind != Tig::Module::NodeKind::kMemory &&
            (source.kind != Tig::Module::NodeKind::kJoin ||
             module.nodes.at(source.inputs.front().node_id).kind !=
                 Tig::Module::NodeKind::kMemory)) {
          continue;
        }
        std::vector<Tig::Module::EdgeRef> region_refs;
        ExprBuilder expr_builder(module.nodes[op_id].expr_graph, diagnostics_);
        for (auto dimension = region->rbegin(); dimension != region->rend(); ++dimension) {
          assert(dimension->extent <=
                 static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
          const BitIndex extent = static_cast<BitIndex>(dimension->extent);
          const ExprId extent_id =
              extent == 1 ? ExprGraph::constant_one
                          : expr_builder.find_or_create_const(
                                extent, ExprBuilder::minimum_unsigned_width(extent), false);
          region_refs.push_back(
              add_node_output_expr(module, op_id, dimension->index, create_temporary_name(module)));
          region_refs.push_back(add_node_output_expr(module, op_id, extent_id));
        }
        std::vector<Tig::Module::EdgeRef> read_inputs;
        std::vector<bool> region_ranges;
        std::vector<bool> new_region_ranges;
        for (auto dimension = region->rbegin(); dimension != region->rend(); ++dimension) {
          new_region_ranges.push_back(dimension->range);
        }
        if (extends_read) {
          read_inputs = source.inputs;
          region_ranges = source.memory_region_ranges;
        } else {
          const auto ren = add_node_output_expr(module, op_id, ExprGraph::constant_one,
                                                create_temporary_name(module));
          read_inputs = {ren, input};
        }
        if (extends_read && region_ranges.back()) {
          const auto old_index_ref = read_inputs.at(read_inputs.size() - 2);
          assert(old_index_ref.node_id == op_id);
          const ExprId old_index = module.nodes[op_id].expr_roots.at(old_index_ref.port_idx);
          const auto new_index_ref = region_refs.at(0);
          const ExprId new_index = module.nodes[op_id].expr_roots.at(new_index_ref.port_idx);
          const ExprId combined_index = expr_builder.create_add(old_index, new_index);
          read_inputs[read_inputs.size() - 2] =
              add_node_output_expr(module, op_id, combined_index, create_temporary_name(module));
          read_inputs.back() = region_refs.at(1);
          region_ranges.back() = new_region_ranges.front();
          region_refs.erase(region_refs.begin(), region_refs.begin() + 2);
          new_region_ranges.erase(new_region_ranges.begin());
        }
        read_inputs.insert(read_inputs.end(), region_refs.begin(), region_refs.end());
        region_ranges.insert(region_ranges.end(), new_region_ranges.begin(),
                             new_region_ranges.end());
        const auto read_expr = module.nodes[op_id].expr_graph.nodes[read_id];
        const Tig::NodeId memory_read_id = static_cast<Tig::NodeId>(module.nodes.size());
        module.nodes.emplace_back();
        auto &memory_read = module.nodes.back();
        memory_read.kind = Tig::Module::NodeKind::kMemoryRead;
        memory_read.inputs = std::move(read_inputs);
        memory_read.memory_region_ranges = std::move(region_ranges);
        memory_read.outputs.push_back({"", read_expr.width, read_expr.sign});
        memory_read.expr_roots.push_back(kInvalidExprId);
        memory_read.combs.push_back(true);
        pending_reads.push_back({read_id, memory_read_id});
        return true;
      }
      return false;
    }

    const size_t first_operand = expr.op == ExprGraph::Op::kSequence ? 1 : 0;
    for (size_t operand = first_operand; operand < expr.operands.size(); ++operand) {
      create_memory_reads(module, op_id, expr.operands[operand], kInvalidExprId, nullptr, visited,
                          pending_reads);
    }
    if (expr.op == ExprGraph::Op::kSequence) {
      assert(!expr.operands.empty());
      const bool resolved = create_memory_reads(module, op_id, expr.operands.front(), read_id,
                                                region, visited, pending_reads);
      if (resolved) {
        diagnostics_.error(DiagnosticId::kTransformUnsupportedMemoryRead,
                           "memory read after write ignores previous writes");
      }
      return resolved;
    }
    if (expr.op == ExprGraph::Op::kCall) {
      diagnostics_.warning(DiagnosticId::kTransformUnsupportedMemoryRead,
                           "memory read through subroutine call was not extracted");
      return false;
    }
    diagnostics_.error(DiagnosticId::kTransformUnsupportedMemoryRead,
                       "memory source is not a memory input or sequence");
    return false;
  }
  for (ExprId operand : expr.operands) {
    create_memory_reads(module, op_id, operand, kInvalidExprId, nullptr, visited, pending_reads);
  }
  return false;
}

void TigTransformer::infer_memory() {
  for (auto &module : design_.modules) {
    const size_t original_node_count = module.nodes.size();
    std::vector<bool> modified_ops(original_node_count, false);

    for (Tig::NodeId op_id = 0; op_id < original_node_count; ++op_id) {
      if (module.nodes[op_id].kind != Tig::Module::NodeKind::kOp) {
        continue;
      }
      std::vector<PendingMemoryRead> pending_reads;
      std::vector<bool> visited(module.nodes[op_id].expr_graph.nodes.size(), false);
      const std::vector<ExprId> roots = module.nodes[op_id].expr_roots;
      for (ExprId root : roots) {
        create_memory_reads(module, op_id, root, kInvalidExprId, nullptr, visited, pending_reads);
      }

      for (const auto &pending : pending_reads) {
        auto &op = module.nodes[op_id];
        auto &replacement = op.expr_graph.nodes[pending.id];
        replacement.op = ExprGraph::Op::kInput;
        replacement.operands.clear();
        const std::string name = create_temporary_name(module);
        module.nodes[pending.node_id].outputs.at(0).name = name;
        std::vector<SignalWidth> unpacked_dims;
        SignalWidth width = replacement.width;
        bool sign = replacement.sign;
        for (const auto &properties : op.expr_graph.unpacked_properties) {
          if (properties.id == pending.id) {
            unpacked_dims = properties.unpacked_dims;
            width = properties.width;
            sign = properties.sign;
            break;
          }
        }
        module.signals.push_back({name, std::move(unpacked_dims), width, sign});
        op.inputs.push_back({pending.node_id, 0});
        op.input_expr_ids.push_back(pending.id);
      }
      modified_ops[op_id] = !pending_reads.empty();
    }

    for (Tig::NodeId memory_id = 0; memory_id < original_node_count; ++memory_id) {
      if (module.nodes[memory_id].kind != Tig::Module::NodeKind::kMemory) {
        continue;
      }
      const auto update = module.nodes[memory_id].inputs.at(0);
      const auto update_kind = module.nodes.at(update.node_id).kind;
      if (update_kind == Tig::Module::NodeKind::kMemoryWrite) {
        continue;
      }
      const bool has_multi_driver = update_kind == Tig::Module::NodeKind::kMultiDriver;
      if (has_multi_driver) {
        bool already_inferred = !module.nodes.at(update.node_id).inputs.empty();
        for (const auto &source : module.nodes.at(update.node_id).inputs) {
          if (module.nodes.at(source.node_id).kind != Tig::Module::NodeKind::kMemoryWrite) {
            already_inferred = false;
            break;
          }
        }
        if (already_inferred) {
          continue;
        }
      }
      const auto output = module.nodes.at(update.node_id).outputs.at(update.port_idx);
      const std::vector<Tig::Module::EdgeRef> sources =
          has_multi_driver ? module.nodes.at(update.node_id).inputs
                           : std::vector<Tig::Module::EdgeRef>{update};
      std::vector<Tig::Module::EdgeRef> writes;
      for (auto source : sources) {
        const auto &op = module.nodes.at(source.node_id);
        assert(op.kind == Tig::Module::NodeKind::kOp);
        const ExprId root = op.expr_roots.at(source.port_idx);
        const auto root_writes = create_memory_writes(module, source.node_id, root);
        writes.insert(writes.end(), root_writes.begin(), root_writes.end());
        module.nodes[source.node_id].expr_roots[source.port_idx] = kInvalidExprId;
        modified_ops[source.node_id] = true;
      }
      assert(!writes.empty());
      if (writes.size() == 1) {
        module.nodes[memory_id].inputs[0] = writes.front();
      } else if (has_multi_driver) {
        module.nodes.at(update.node_id).inputs = std::move(writes);
      } else {
        const Tig::NodeId driver_id = static_cast<Tig::NodeId>(module.nodes.size());
        module.nodes.emplace_back();
        auto &driver = module.nodes.back();
        driver.kind = Tig::Module::NodeKind::kMultiDriver;
        driver.inputs = std::move(writes);
        driver.outputs.push_back(output);
        driver.expr_roots.push_back(kInvalidExprId);
        driver.combs.push_back(false);
        module.nodes[memory_id].inputs[0] = {driver_id, 0};
      }
    }

    for (Tig::NodeId op_id = 0; op_id < original_node_count; ++op_id) {
      if (modified_ops[op_id]) {
        clean_op_node(module.nodes[op_id]);
      }
    }
  }
}

void TigTransformer::decompose_dynamic_access() {
  for (auto &module : design_.modules) {
    for (auto &node : module.nodes) {
      if (node.kind != Tig::Module::NodeKind::kOp) {
        continue;
      }
      ExprBuilder builder(node.expr_graph, diagnostics_);
      const size_t expression_count = node.expr_graph.nodes.size();
      std::vector<bool> visited(expression_count);
      bool modified = false;
      const auto decompose = [&](const auto &self, ExprId id) -> void {
        if (id == kInvalidExprId || id >= expression_count || visited[id]) {
          return;
        }
        visited[id] = true;
        const auto expression = node.expr_graph.nodes[id];
        for (ExprId operand : expression.operands) {
          self(self, operand);
        }
        switch (expression.op) {
        case ExprGraph::Op::kShl:
        case ExprGraph::Op::kShr:
        case ExprGraph::Op::kAshr: {
          assert(expression.operands.size() == 2);
          if (builder.try_evaluate(expression.operands[1]).has_value()) {
            return;
          }
          const ExprId data = expression.operands[0];
          const SignalWidth width = builder.get_width(data);
          assert(width > 0);
          assert(width <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
          std::vector<ExprId> lanes;
          lanes.reserve(width);
          for (SignalWidth lane = 0; lane < width; ++lane) {
            lanes.push_back(builder.create_static_range(data, lane, 1, false));
          }
          const bool reverse = expression.op == ExprGraph::Op::kShl;
          if (reverse) {
            std::reverse(lanes.begin(), lanes.end());
          }
          const ExprId fill =
              expression.op == ExprGraph::Op::kAshr ? lanes.back() : ExprGraph::constant_zero;
          lanes = create_barrel_shift(builder, std::move(lanes), expression.operands[1], fill);
          if (!reverse) {
            std::reverse(lanes.begin(), lanes.end());
          }
          const ExprId expanded = builder.create_concat(std::move(lanes), builder.get_sign(data));
          auto &replacement = node.expr_graph.nodes[id];
          modified = true;
          replacement.op = ExprGraph::Op::kConvert;
          replacement.operands = {expanded};
          return;
        }
        case ExprGraph::Op::kRange: {
          assert(expression.operands.size() >= 2);
          const ExprId data = expression.operands[0];
          const auto affine = builder.decode_affine_index(expression.operands, 1);
          if (affine.terms.empty()) {
            return;
          }
          const SignalWidth data_width = builder.get_width(data);
          assert(expression.width > 0);
          assert(expression.width <= data_width);
          ExprId index;
          SignalWidth stride = 1;
          SignalWidth offset = 0;
          if (affine.terms.size() == 1 && affine.terms.front().stride > 0 && affine.offset >= 0 &&
              !builder.get_sign(affine.terms.front().index)) {
            index = affine.terms.front().index;
            stride = static_cast<SignalWidth>(affine.terms.front().stride);
            offset = static_cast<SignalWidth>(affine.offset);
          } else {
            index = materialize_affine_index(builder, affine);
          }
          const ExprId fill = builder.find_or_create_const("1'bx", 1, false);
          if (builder.get_sign(index)) {
            const SignalWidth negative_positions = expression.width - 1;
            std::vector<ExprId> lanes(negative_positions, fill);
            for (SignalWidth bit = 0; bit < data_width; ++bit) {
              lanes.push_back(builder.create_static_range(data, bit, 1, false));
            }
            lanes.insert(lanes.end(), expression.width - 1, fill);
            ExprId amount = index;
            if (negative_positions > 0) {
              SignalWidth amount_width =
                  std::max(builder.get_width(index),
                           ExprBuilder::minimum_unsigned_width(negative_positions) + 1);
              const ExprId alignment = builder.find_or_create_const(
                  static_cast<BitIndex>(negative_positions), amount_width, true);
              amount = builder.create_add(index, alignment);
            }
            lanes = create_barrel_shift(builder, std::move(lanes), amount, fill);
            std::vector<ExprId> result(lanes.begin(), lanes.begin() + expression.width);
            std::reverse(result.begin(), result.end());
            const ExprId expanded = builder.create_concat(std::move(result), expression.sign);
            auto &replacement = node.expr_graph.nodes[id];
            modified = true;
            replacement.op = ExprGraph::Op::kConvert;
            replacement.operands = {expanded};
            return;
          }
          std::vector<ExprId> result(expression.width, kInvalidExprId);
          std::vector<std::vector<ExprId>> bit_classes;
          bit_classes.reserve(std::min(stride, data_width));
          for (SignalWidth residue = 0; residue < std::min(stride, data_width); ++residue) {
            std::vector<ExprId> bit_class;
            for (SignalWidth bit = residue; bit < data_width; bit += stride) {
              bit_class.push_back(builder.create_static_range(data, bit, 1, false));
            }
            bit_classes.push_back(create_barrel_shift(builder, std::move(bit_class), index, fill));
          }
          for (SignalWidth bit = 0; bit < expression.width; ++bit) {
            if (bit > std::numeric_limits<SignalWidth>::max() - offset ||
                bit + offset >= data_width) {
              result[bit] = fill;
              continue;
            }
            const SignalWidth source = bit + offset;
            result[bit] = bit_classes[source % stride][source / stride];
          }
          assert(std::ranges::none_of(result, [](ExprId bit) { return bit == kInvalidExprId; }));
          std::reverse(result.begin(), result.end());
          const ExprId expanded = builder.create_concat(std::move(result), expression.sign);
          auto &replacement = node.expr_graph.nodes[id];
          modified = true;
          replacement.op = ExprGraph::Op::kConvert;
          replacement.operands = {expanded};
          return;
        }
        case ExprGraph::Op::kMaskedAssign: {
          assert(expression.operands.size() >= 4);
          const ExprId current = expression.operands[0];
          const ExprId next = expression.operands[1];
          const auto affine = builder.decode_affine_index(expression.operands, 3);
          if (affine.terms.empty()) {
            return;
          }
          const auto slice_width_value = builder.try_evaluate(expression.operands[2]);
          assert(slice_width_value.has_value());
          assert(*slice_width_value > 0);
          const SignalWidth slice_width = static_cast<SignalWidth>(*slice_width_value);
          const SignalWidth width = builder.get_width(current);
          assert(slice_width <= width);
          assert(builder.get_width(next) == slice_width);
          ExprId index;
          SignalWidth stride = 1;
          SignalWidth offset = 0;
          if (affine.terms.size() == 1 && affine.terms.front().stride > 0 && affine.offset >= 0) {
            index = affine.terms.front().index;
            stride = static_cast<SignalWidth>(affine.terms.front().stride);
            offset = static_cast<SignalWidth>(affine.offset);
          } else {
            index = materialize_affine_index(builder, affine);
          }
          const SignalWidth negative_positions = builder.get_sign(index) ? slice_width - 1 : 0;
          ExprId amount = index;
          if (negative_positions > 0) {
            const SignalWidth amount_width =
                std::max(builder.get_width(index),
                         ExprBuilder::minimum_unsigned_width(negative_positions) + 1);
            const ExprId shift_offset = builder.find_or_create_const(
                static_cast<BitIndex>(negative_positions), amount_width, true);
            amount = builder.create_add(index, shift_offset);
          }
          std::vector<ExprId> result(width, kInvalidExprId);
          for (SignalWidth residue = 0; residue < std::min(stride, width); ++residue) {
            std::vector<ExprId> next_class;
            std::vector<ExprId> mask_class;
            std::vector<SignalWidth> bits;
            for (SignalWidth bit = residue; bit < width; bit += stride) {
              bits.push_back(bit);
              if (bit >= offset && bit - offset < slice_width) {
                next_class.push_back(builder.create_static_range(next, bit - offset, 1, false));
                mask_class.push_back(ExprGraph::constant_one);
              } else {
                next_class.push_back(ExprGraph::constant_zero);
                mask_class.push_back(ExprGraph::constant_zero);
              }
            }
            next_class.insert(next_class.end(), negative_positions, ExprGraph::constant_zero);
            mask_class.insert(mask_class.end(), negative_positions, ExprGraph::constant_zero);
            std::reverse(next_class.begin(), next_class.end());
            std::reverse(mask_class.begin(), mask_class.end());
            next_class = create_barrel_shift(builder, std::move(next_class), amount,
                                             ExprGraph::constant_zero);
            mask_class = create_barrel_shift(builder, std::move(mask_class), amount,
                                             ExprGraph::constant_zero);
            std::reverse(next_class.begin(), next_class.end());
            std::reverse(mask_class.begin(), mask_class.end());
            for (size_t position = 0; position < bits.size(); ++position) {
              const SignalWidth bit = bits[position];
              const size_t lane = negative_positions + position;
              const ExprId current_bit = builder.create_static_range(current, bit, 1, false);
              result[bit] = builder.create_mux(mask_class[lane], next_class[lane], current_bit);
            }
          }
          assert(std::ranges::none_of(result, [](ExprId bit) { return bit == kInvalidExprId; }));
          std::reverse(result.begin(), result.end());
          const ExprId expanded = builder.create_concat(std::move(result), expression.sign);
          auto &replacement = node.expr_graph.nodes[id];
          modified = true;
          replacement.op = ExprGraph::Op::kConvert;
          replacement.operands = {expanded};
          return;
        }
        case ExprGraph::Op::kUnpackedRange: {
          assert(expression.operands.size() == 2);
          if (builder.try_evaluate(expression.operands[1]).has_value()) {
            return;
          }
          decompose_unpacked_range(node, builder, id);
          modified = true;
          return;
        }
        case ExprGraph::Op::kUnpackedSelect: {
          assert(expression.operands.size() == 2);
          const ExprId data = expression.operands[0];
          const ExprId index = expression.operands[1];
          if (builder.try_evaluate(index).has_value()) {
            return;
          }
          const auto data_properties = get_unpacked_properties(node, data);
          assert(data_properties.has_value());
          std::vector<SignalWidth> remaining_dims(data_properties->unpacked_dims.begin() + 1,
                                                  data_properties->unpacked_dims.end());
          std::vector<ExprId> elements =
              create_unpacked_element_selects(builder, data, *data_properties);
          SignalWidth fill_width = data_properties->width;
          for (SignalWidth dimension : remaining_dims) {
            assert(dimension > 0);
            assert(fill_width <= std::numeric_limits<SignalWidth>::max() / dimension);
            fill_width *= dimension;
          }
          ExprId fill = builder.find_or_create_const(std::to_string(fill_width) + "'bx", fill_width,
                                                     data_properties->sign);
          if (!remaining_dims.empty()) {
            fill = builder.create_unpacked_fold(fill, remaining_dims, data_properties->width,
                                                data_properties->sign);
          }
          elements = create_barrel_shift(builder, std::move(elements), index, fill);
          const ExprId expanded = elements.front();
          const auto expanded_node = node.expr_graph.nodes[expanded];
          auto &replacement = node.expr_graph.nodes[id];
          modified = true;
          replacement.op = expanded_node.op;
          replacement.operands = expanded_node.operands;
          return;
        }
        case ExprGraph::Op::kSequence:
          assert(!expression.operands.empty());
          if (expression.operands.front() == kInvalidExprId) {
            return;
          }
          decompose_unpacked_sequence(node, builder, id);
          modified = true;
          return;
        default:
          return;
        }
      };
      for (ExprId root : node.expr_roots) {
        decompose(decompose, root);
      }
      if (modified) {
        clean_op_node(node);
      }
    }
  }
}

} // namespace abys::ir
