#include "abys/ir/tig_transformer.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "abys/ir/expr_builder.h"

namespace abys::ir {
namespace {

std::string escaped_indexed_name(const std::string &name, SignalWidth index) {
  return "\\" + name + "[" + std::to_string(index) + "] ";
}

ExprId materialize_unpacked_range_assign_base(const ExprGraph &graph, ExprBuilder &builder,
                                              ExprId id) {
  if (const auto base = graph.unpacked_range_assign_base(id)) {
    return builder.find_or_create_const(*base, ExprBuilder::minimum_unsigned_width(*base), false);
  }
  return graph.operands(id).at(1);
}

} // namespace

std::optional<UnpackedShape> TigTransformer::get_unpacked_shape(const Tig::Module &module,
                                                                NodeId node_id, ExprId id) {
  if (const auto *shape = module.node_expr_graph(node_id).unpacked_shape(id)) {
    return *shape;
  }
  return std::nullopt;
}

std::vector<ExprId> TigTransformer::create_unpacked_element_selects(ExprBuilder &builder,
                                                                    ExprId data,
                                                                    const UnpackedShape &shape) {
  assert(!shape.unpacked_dims.empty());
  const SignalWidth extent = shape.unpacked_dims.front();
  assert(extent > 0);
  assert(extent <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
  std::vector<SignalWidth> remaining_dims(shape.unpacked_dims.begin() + 1,
                                          shape.unpacked_dims.end());
  const SignalWidth element_width =
      remaining_dims.empty() ? shape.inner_dim : remaining_dims.front();
  const bool element_sign = remaining_dims.empty() ? shape.inner_sign : false;
  if (builder.get_op(data) == ExprGraph::Op::kGather) {
    assert(builder.get_operands(data).size() == extent);
    return builder.get_operands(data);
  }
  std::vector<ExprId> elements;
  elements.reserve(extent);
  for (SignalWidth element = 0; element < extent; ++element) {
    const ExprId index = builder.find_or_create_const(
        static_cast<BitIndex>(element), ExprBuilder::minimum_unsigned_width(element), false);
    elements.push_back(builder.create_unpacked_select(
        data, index, 0, static_cast<BitIndex>(extent - 1), element_width, element_sign,
        remaining_dims, shape.inner_dim, shape.inner_sign));
  }
  return elements;
}

ExprId TigTransformer::materialize_affine_index(ExprBuilder &builder, const AffineIndex &index) {
  SignalWidth width =
      ExprBuilder::minimum_unsigned_width(static_cast<BitIndex>(std::abs(index.offset))) + 1;
  for (const auto &term : index.terms) {
    SignalWidth term_width = builder.get_outer_dim(term.index);
    if (!builder.get_outer_sign(term.index)) {
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

bool TigTransformer::can_decompose_affine_index(ExprBuilder &builder, const AffineIndex &index) {
  if (index.terms.empty() || index.offset < 0 || index.terms.front().stride == 0) {
    return false;
  }
  const bool reverse = index.terms.front().stride < 0;
  return std::ranges::all_of(index.terms, [&](const AffineIndex::Term &term) {
    return term.stride != 0 && term.stride != std::numeric_limits<BitIndex>::min() &&
           (term.stride < 0) == reverse && !builder.get_outer_sign(term.index);
  });
}

std::optional<TigTransformer::StridedIndex>
TigTransformer::extract_strided_index(ExprBuilder &builder, ExprId base) {
  if (builder.get_op(base) == ExprGraph::Op::kAdd) {
    assert(builder.get_operands(base).size() == 2);
    for (size_t constant_operand = 0; constant_operand < 2; ++constant_operand) {
      const auto value = builder.try_evaluate(builder.get_operands(base)[constant_operand]);
      if (value && *value >= 0) {
        auto result =
            extract_strided_index(builder, builder.get_operands(base)[1 - constant_operand]);
        if (result) {
          result->offset += static_cast<SignalWidth>(*value);
          return result;
        }
      }
    }
  }
  if (builder.get_op(base) == ExprGraph::Op::kMul) {
    assert(builder.get_operands(base).size() == 2);
    for (size_t constant_operand = 0; constant_operand < 2; ++constant_operand) {
      const auto value = builder.try_evaluate(builder.get_operands(base)[constant_operand]);
      if (value && *value > 0) {
        return StridedIndex{builder.get_operands(base)[1 - constant_operand],
                            static_cast<SignalWidth>(*value), 0};
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
  const SignalWidth amount_width = builder.get_outer_dim(amount);
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

std::vector<ExprId> TigTransformer::create_strided_barrel_shift(ExprBuilder &builder,
                                                                std::vector<ExprId> lanes,
                                                                ExprId amount, BitIndex stride,
                                                                ExprId fill) {
  assert(!lanes.empty());
  assert(stride != 0);
  assert(stride != std::numeric_limits<BitIndex>::min());
  const bool reverse = stride < 0;
  const SignalWidth magnitude = static_cast<SignalWidth>(std::abs(stride));
  std::vector<ExprId> result(lanes.size(), kInvalidExprId);
  for (SignalWidth residue = 0; residue < std::min(magnitude, lanes.size()); ++residue) {
    std::vector<ExprId> bit_class;
    for (size_t position = residue; position < lanes.size(); position += magnitude) {
      bit_class.push_back(lanes[position]);
    }
    if (reverse) {
      std::reverse(bit_class.begin(), bit_class.end());
    }
    bit_class = create_barrel_shift(builder, std::move(bit_class), amount, fill);
    if (reverse) {
      std::reverse(bit_class.begin(), bit_class.end());
    }
    size_t class_index = 0;
    for (size_t position = residue; position < lanes.size(); position += magnitude) {
      result[position] = bit_class[class_index++];
    }
  }
  assert(std::ranges::none_of(result, [](ExprId id) { return id == kInvalidExprId; }));
  return result;
}

void TigTransformer::decompose_unpacked_range(Tig::Module &module, NodeId node_id,
                                              ExprBuilder &builder, ExprId range_id) {
  const auto range_operands = module.node_expr_graph(node_id).operands(range_id);
  assert(module.node_expr_graph(node_id).op(range_id) == ExprGraph::Op::kUnpackedRange);
  assert(range_operands.size() == 2);
  const ExprId data = range_operands[0];
  const ExprId base = range_operands[1];
  const auto shape = get_unpacked_shape(module, node_id, range_id);
  const auto data_shape = get_unpacked_shape(module, node_id, data);
  assert(shape.has_value());
  assert(data_shape.has_value());
  assert(!shape->unpacked_dims.empty());
  assert(!data_shape->unpacked_dims.empty());
  const SignalWidth slice_width = shape->unpacked_dims.front();
  const SignalWidth extent = data_shape->unpacked_dims.front();
  assert(slice_width > 0);
  assert(extent > 0);

  std::vector<ExprId> elements = create_unpacked_element_selects(builder, data, *data_shape);
  std::vector<SignalWidth> remaining_dims(data_shape->unpacked_dims.begin() + 1,
                                          data_shape->unpacked_dims.end());
  SignalWidth fill_width = data_shape->inner_dim;
  for (SignalWidth dimension : remaining_dims) {
    assert(dimension > 0);
    assert(fill_width <= std::numeric_limits<SignalWidth>::max() / dimension);
    fill_width *= dimension;
  }
  ExprId fill = builder.find_or_create_const(std::to_string(fill_width) + "'bx", fill_width,
                                             data_shape->inner_sign);
  if (!remaining_dims.empty()) {
    fill = builder.create_unpacked_fold(fill, remaining_dims, data_shape->inner_dim,
                                        data_shape->inner_sign);
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
    const SignalWidth negative_positions = builder.get_outer_sign(index) ? slice_width - 1 : 0;
    assert(negative_positions <= std::numeric_limits<SignalWidth>::max() - extent);
    std::vector<ExprId> lanes(negative_positions, fill);
    lanes.insert(lanes.end(), elements.begin(), elements.end());
    lanes.insert(lanes.end(), slice_width - 1, fill);
    ExprId amount = index;
    assert(strided_index->offset <= std::numeric_limits<SignalWidth>::max() - negative_positions);
    const SignalWidth alignment = strided_index->offset + negative_positions;
    if (alignment > 0) {
      SignalWidth amount_width =
          std::max(builder.get_outer_dim(index), ExprBuilder::minimum_unsigned_width(alignment));
      if (builder.get_outer_sign(index)) {
        ++amount_width;
      }
      const ExprId offset = builder.find_or_create_const(
          static_cast<BitIndex>(alignment), amount_width, builder.get_outer_sign(index));
      amount = builder.create_add(index, offset);
    }
    lanes = create_barrel_shift(builder, std::move(lanes), amount, fill);
    selected.assign(lanes.begin(), lanes.begin() + slice_width);
  }

  const ExprId gathered = builder.create_gather(std::move(selected), shape->unpacked_dims,
                                                shape->inner_dim, shape->inner_sign);
  module.node_expr_graph(node_id).replace_with(range_id, gathered);
}

void TigTransformer::decompose_unpacked_sequence(Tig::Module &module, NodeId node_id,
                                                 ExprBuilder &builder, ExprId sequence_id) {
  const auto sequence_operands = module.node_expr_graph(node_id).operands(sequence_id);
  assert(module.node_expr_graph(node_id).op(sequence_id) == ExprGraph::Op::kSequence);
  assert(!sequence_operands.empty());
  const ExprId base = sequence_operands.front();
  assert(base != kInvalidExprId);
  const auto shape = get_unpacked_shape(module, node_id, sequence_id);
  assert(shape.has_value());
  assert(!shape->unpacked_dims.empty());
  const SignalWidth extent = shape->unpacked_dims.front();
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
  std::vector<SignalWidth> remaining_dims(shape->unpacked_dims.begin() + 1,
                                          shape->unpacked_dims.end());
  ExprId current_value = base;
  auto apply_nested_update = [&](ExprId next, ExprId current,
                                 const std::vector<SignalWidth> &unpacked_dims) {
    if (unpacked_dims.empty()) {
      return next;
    }
    const ExprId nested_sequence =
        builder.create_sequence(next, current, unpacked_dims, shape->inner_dim, shape->inner_sign);
    decompose_unpacked_sequence(module, node_id, builder, nested_sequence);
    return nested_sequence;
  };

  std::stack<PendingUpdate> pending;
  for (size_t operand = sequence_operands.size() - 1; operand != 0; --operand) {
    assert(sequence_operands[operand] != kInvalidExprId);
    pending.push({sequence_operands[operand], ExprGraph::constant_one});
  }
  while (!pending.empty()) {
    const PendingUpdate current = pending.top();
    pending.pop();
    if (current.id == kInvalidExprId) {
      continue;
    }
    const auto update_operands = module.node_expr_graph(node_id).operands(current.id);
    switch (module.node_expr_graph(node_id).op(current.id)) {
    case ExprGraph::Op::kSequence: {
      assert(!update_operands.empty());
      assert(update_operands.front() == kInvalidExprId);
      for (size_t operand = update_operands.size() - 1; operand != 0; --operand) {
        assert(update_operands[operand] != kInvalidExprId);
        pending.push({update_operands[operand], current.enable});
      }
      break;
    }
    case ExprGraph::Op::kMux:
    case ExprGraph::Op::kUnpackedMux: {
      assert(update_operands.size() == 3);
      const ExprId condition = update_operands[0];
      const ExprId then_enable = combine_enable(current.enable, condition);
      const ExprId else_enable =
          combine_enable(current.enable, builder.create_logical_not(condition));
      pending.push({update_operands[2], else_enable});
      pending.push({update_operands[1], then_enable});
      break;
    }
    case ExprGraph::Op::kCase:
    case ExprGraph::Op::kUnpackedCase: {
      assert(!update_operands.empty());
      const ExprId selector = update_operands[0];
      std::vector<ExprId> matches;
      std::vector<PendingUpdate> branches;
      size_t operand = 1;
      while (operand + 1 < update_operands.size()) {
        const ExprId match = builder.create_match(selector, update_operands[operand]);
        matches.push_back(match);
        branches.push_back({update_operands[operand + 1], combine_enable(current.enable, match)});
        operand += 2;
      }
      if (operand < update_operands.size()) {
        ExprId default_enable = current.enable;
        if (!matches.empty()) {
          const ExprId any_match =
              matches.size() == 1 ? matches.front() : builder.create_or(std::move(matches));
          default_enable = combine_enable(current.enable, builder.create_logical_not(any_match));
        }
        branches.push_back({update_operands[operand], default_enable});
      }
      for (auto branch = branches.rbegin(); branch != branches.rend(); ++branch) {
        pending.push(*branch);
      }
      break;
    }
    case ExprGraph::Op::kUnpackedAssign: {
      assert(update_operands.size() == 2);
      const ExprId next = update_operands[0];
      const ExprId index = update_operands[1];
      std::vector<ExprId> elements =
          create_unpacked_element_selects(builder, current_value, *shape);
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
      current_value = builder.create_gather(std::move(elements), shape->unpacked_dims,
                                            shape->inner_dim, shape->inner_sign);
      break;
    }
    case ExprGraph::Op::kUnpackedRangeAssign: {
      const ExprId next = update_operands[0];
      const ExprId base_index = materialize_unpacked_range_assign_base(
          module.node_expr_graph(node_id), builder, current.id);
      const SignalWidth slice_width = module.node_expr_graph(node_id).slice_width(current.id);
      const auto next_shape = get_unpacked_shape(module, node_id, next);
      assert(next_shape.has_value());
      assert(next_shape->unpacked_dims.front() == slice_width);
      const ExprId current_range = builder.create_unpacked_range(
          current_value, base_index, slice_width, next_shape->unpacked_dims, shape->inner_dim,
          shape->inner_sign);
      if (builder.get_op(current_range) == ExprGraph::Op::kUnpackedRange) {
        decompose_unpacked_range(module, node_id, builder, current_range);
      }
      const ExprId updated_range =
          apply_nested_update(next, current_range, next_shape->unpacked_dims);
      const std::vector<ExprId> updated_elements =
          create_unpacked_element_selects(builder, updated_range, *next_shape);
      std::vector<ExprId> current_elements =
          create_unpacked_element_selects(builder, current_value, *shape);
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
        const SignalWidth negative_positions =
            builder.get_outer_sign(base_index) ? slice_width - 1 : 0;
        if (negative_positions > 0) {
          const SignalWidth amount_width =
              std::max(builder.get_outer_dim(base_index),
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
      current_value = builder.create_gather(std::move(current_elements), shape->unpacked_dims,
                                            shape->inner_dim, shape->inner_sign);
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
      builder.create_gather(create_unpacked_element_selects(builder, current_value, *shape),
                            shape->unpacked_dims, shape->inner_dim, shape->inner_sign);
  module.node_expr_graph(node_id).replace_with(sequence_id, gathered);
}

ExprId TigTransformer::add_node_input_expr(Tig::Module &module, NodeId node_id, EdgeRef input) {
  assert(module.node_kind(node_id) == Tig::Module::NodeKind::kOp);
  assert(module.node_inputs(node_id).size() == module.node_input_expr_ids(node_id).size());
  for (size_t port = 0; port < module.node_inputs(node_id).size(); ++port) {
    if (module.node_inputs(node_id)[port].node_id == input.node_id &&
        module.node_inputs(node_id)[port].port_idx == input.port_idx) {
      return module.node_input_expr_ids(node_id)[port];
    }
  }
  const auto &name = module.node_output_name(input.node_id, input.port_idx);
  const auto index = module.signals.find_index(name);
  const auto *shape = index ? module.signals.shape(*index) : nullptr;
  const ExprId id = module.node_expr_graph(node_id).append(
      ExprGraph::Op::kInput,
      shape ? shape->unpacked_dims.front()
            : module.node_output_outer_dim(input.node_id, input.port_idx),
      !shape && module.node_output_outer_sign(input.node_id, input.port_idx));
  if (shape) {
    module.node_expr_graph(node_id).set_unpacked_shape(id, *shape);
  }
  module.node_inputs(node_id).push_back(input);
  module.node_input_expr_ids(node_id).push_back(id);
  return id;
}

void TigTransformer::clean_op_node(Tig::Module &module, NodeId node_id) {
  assert(module.node_kind(node_id) == Tig::Module::NodeKind::kOp);
  auto &source = module.node_expr_graph(node_id);
  ExprGraph compact;
  std::vector<ExprId> remap(source.size(), kInvalidExprId);
  remap[ExprGraph::constant_zero] = ExprGraph::constant_zero;
  remap[ExprGraph::constant_one] = ExprGraph::constant_one;

  const auto clone = [&](const auto &self, ExprId old_id) -> ExprId {
    if (old_id == kInvalidExprId) {
      return kInvalidExprId;
    }
    if (remap[old_id] != kInvalidExprId) {
      return remap[old_id];
    }
    auto expression_operands = source.operands(old_id);
    for (ExprId &operand : expression_operands) {
      operand = self(self, operand);
    }
    const ExprId new_id = compact.append_copy(source, old_id, std::move(expression_operands));
    remap[old_id] = new_id;
    return new_id;
  };

  for (ExprId &root : module.node_output_expr_ids(node_id)) {
    root = clone(clone, root);
  }
  assert(module.node_inputs(node_id).size() == module.node_input_expr_ids(node_id).size());
  std::vector<EdgeRef> compact_inputs;
  std::vector<ExprId> compact_input_expr_ids;
  for (size_t port = 0; port < module.node_inputs(node_id).size(); ++port) {
    const ExprId id = module.node_input_expr_ids(node_id)[port];
    assert(module.node_inputs(node_id)[port].node_id != kInvalidNodeId ||
           remap[id] == kInvalidExprId);
    if (remap[id] != kInvalidExprId) {
      compact_inputs.push_back(module.node_inputs(node_id)[port]);
      compact_input_expr_ids.push_back(remap[id]);
    }
  }
  module.node_inputs(node_id) = std::move(compact_inputs);
  module.node_input_expr_ids(node_id) = std::move(compact_input_expr_ids);

  source = std::move(compact);
}

bool TigTransformer::is_expr_graph_topological(const Tig::Module &module, NodeId node_id) {
  assert(module.node_kind(node_id) == Tig::Module::NodeKind::kOp);
  for (ExprId id = 0; id < module.node_expr_graph(node_id).size(); ++id) {
    for (ExprId operand : module.node_expr_graph(node_id).operands(id)) {
      if (operand != kInvalidExprId && operand >= id) {
        return false;
      }
    }
  }
  return true;
}

void TigTransformer::blast_expr_graph(Tig::Module &module, NodeId node_id,
                                      std::vector<std::vector<ExprId>> blasted_ids) {
  assert(module.node_kind(node_id) == Tig::Module::NodeKind::kOp);
  auto &graph = module.node_expr_graph(node_id);
  const size_t expr_count = blasted_ids.size();
  assert(expr_count <= nodes.size());
  ExprBuilder builder(module.node_expr_graph(node_id), diagnostics_);
  std::vector<ExprId> word_outputs(expr_count, kInvalidExprId);
  std::vector<bool> propagate_unpacked_assign(expr_count, false);
  const auto find_or_create_word_output = [&](ExprId id) {
    if (id == kInvalidExprId) {
      return id;
    }
    assert(id < expr_count);
    if (word_outputs[id] != kInvalidExprId) {
      return word_outputs[id];
    }
    assert(!blasted_ids[id].empty());
    if (blasted_ids[id].size() == 1 &&
        (blasted_ids[id].front() == id || !get_unpacked_shape(module, node_id, id))) {
      word_outputs[id] = blasted_ids[id].front();
      return word_outputs[id];
    }
    ExprId result = blasted_ids[id].front();
    if (blasted_ids[id].size() > 1) {
      std::vector<ExprId> bits = blasted_ids[id];
      std::reverse(bits.begin(), bits.end());
      result = builder.create_concat(std::move(bits), graph.outer_sign(id));
    }
    if (const auto shape = get_unpacked_shape(module, node_id, id)) {
      result = builder.create_unpacked_fold(result, shape->unpacked_dims, shape->inner_dim,
                                            shape->inner_sign);
    }
    word_outputs[id] = result;
    return result;
  };
  const auto update_word_operands = [&](ExprId id, const std::vector<ExprId> &expr_operands) {
    std::vector<ExprId> operands;
    operands.reserve(expr_operands.size());
    for (ExprId operand : expr_operands) {
      operands.push_back(find_or_create_word_output(operand));
    }
    graph.set_operands(id, std::move(operands));
  };
  const auto blast_word_output = [&](ExprId id, const std::vector<ExprId> &expr_operands,
                                     SignalWidth width, std::vector<ExprId> &bits) {
    update_word_operands(id, expr_operands);
    word_outputs[id] = id;
    const ExprId data =
        get_unpacked_shape(module, node_id, id) ? builder.create_unpacked_flatten(id) : id;
    for (SignalWidth bit = 0; bit < width; ++bit) {
      bits.push_back(builder.create_static_range(data, bit, 1, false));
    }
  };
  for (ExprId id = 0; id < expr_count; ++id) {
    if (!blasted_ids[id].empty()) {
      continue;
    }
    const auto expr_operands = graph.operands(id);
    const auto shape = get_unpacked_shape(module, node_id, id);
    const SignalWidth width = shape ? shape->flattened_width() : graph.outer_dim(id);
    auto &bits = blasted_ids[id];
    bits.reserve(width);
    switch (graph.op(id)) {
    case ExprGraph::Op::kConcat:
      for (auto operand = expr_operands.rbegin(); operand != expr_operands.rend(); ++operand) {
        bits.insert(bits.end(), blasted_ids[*operand].begin(), blasted_ids[*operand].end());
      }
      break;
    case ExprGraph::Op::kConvert: {
      assert(expr_operands.size() == 1);
      const auto &operand = blasted_ids[expr_operands.front()];
      assert(!operand.empty());
      bits = operand;
      bits.resize(width, graph.outer_sign(expr_operands.front()) ? operand.back()
                                                                 : ExprGraph::constant_zero);
      break;
    }
    case ExprGraph::Op::kReverse:
      assert(expr_operands.size() == 1);
      bits.assign(blasted_ids[expr_operands.front()].rbegin(),
                  blasted_ids[expr_operands.front()].rend());
      break;
    case ExprGraph::Op::kMux:
    case ExprGraph::Op::kUnpackedMux: {
      assert(expr_operands.size() == 3);
      if ((expr_operands[1] != kInvalidExprId && propagate_unpacked_assign[expr_operands[1]]) ||
          (expr_operands[2] != kInvalidExprId && propagate_unpacked_assign[expr_operands[2]])) {
        blast_word_output(id, expr_operands, width, bits);
        propagate_unpacked_assign[id] = true;
        break;
      }
      assert(blasted_ids[expr_operands[0]].size() == 1);
      const ExprId condition = blasted_ids[expr_operands[0]].front();
      assert(expr_operands[1] == kInvalidExprId || blasted_ids[expr_operands[1]].size() == width);
      assert(expr_operands[2] == kInvalidExprId || blasted_ids[expr_operands[2]].size() == width);
      for (SignalWidth bit = 0; bit < width; ++bit) {
        const ExprId then_id = expr_operands[1] == kInvalidExprId
                                   ? kInvalidExprId
                                   : blasted_ids[expr_operands[1]][bit];
        const ExprId else_id = expr_operands[2] == kInvalidExprId
                                   ? kInvalidExprId
                                   : blasted_ids[expr_operands[2]][bit];
        bits.push_back(then_id == kInvalidExprId && else_id == kInvalidExprId
                           ? kInvalidExprId
                           : builder.create_mux(condition, then_id, else_id));
      }
      break;
    }
    case ExprGraph::Op::kCase:
    case ExprGraph::Op::kUnpackedCase: {
      bool retain = false;
      for (size_t operand = 2; operand < expr_operands.size(); operand += 2) {
        retain |= expr_operands[operand] != kInvalidExprId &&
                  propagate_unpacked_assign[expr_operands[operand]];
      }
      if (expr_operands.size() % 2 == 0) {
        retain |= expr_operands.back() != kInvalidExprId &&
                  propagate_unpacked_assign[expr_operands.back()];
      }
      if (retain) {
        blast_word_output(id, expr_operands, width, bits);
        propagate_unpacked_assign[id] = true;
        break;
      }
      const ExprId selector = find_or_create_word_output(expr_operands.front());
      std::vector<ExprId> conditions;
      for (size_t operand = 1; operand + 1 < expr_operands.size(); operand += 2) {
        conditions.push_back(
            builder.create_match(selector, find_or_create_word_output(expr_operands[operand])));
      }
      for (SignalWidth bit = 0; bit < width; ++bit) {
        std::vector<ExprId> data;
        size_t operand = 1;
        while (operand + 1 < expr_operands.size()) {
          const ExprId data_id = expr_operands[operand + 1];
          data.push_back(data_id == kInvalidExprId ? kInvalidExprId : blasted_ids[data_id][bit]);
          operand += 2;
        }
        if (operand < expr_operands.size()) {
          const ExprId data_id = expr_operands[operand];
          data.push_back(data_id == kInvalidExprId ? kInvalidExprId : blasted_ids[data_id][bit]);
        }
        bits.push_back(
            std::ranges::all_of(data, [](ExprId data_id) { return data_id == kInvalidExprId; })
                ? kInvalidExprId
                : builder.create_pmux(conditions, std::move(data)));
      }
      break;
    }
    case ExprGraph::Op::kRange: {
      const auto base = builder.decode_affine_index(id);
      const auto &data = blasted_ids[expr_operands.front()];
      assert(data.size() >= width);
      if (base.terms.empty()) {
        assert(base.offset >= 0);
        assert(static_cast<SignalWidth>(base.offset) <= data.size() - width);
        bits.insert(bits.end(), data.begin() + base.offset, data.begin() + base.offset + width);
        break;
      }
      blast_word_output(id, expr_operands, width, bits);
      break;
    }
    case ExprGraph::Op::kMaskedAssign: {
      const SignalWidth assigned_width = graph.slice_width(id);
      const auto base = builder.decode_affine_index(id);
      if (base.terms.empty()) {
        assert(blasted_ids[expr_operands[0]].size() >= width);
        assert(blasted_ids[expr_operands[1]].size() >= assigned_width);
        for (SignalWidth bit = 0; bit < width; ++bit) {
          assert(bit <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
          const BitIndex next_bit = static_cast<BitIndex>(bit) - base.offset;
          if (next_bit >= 0 && static_cast<SignalWidth>(next_bit) < assigned_width) {
            bits.push_back(blasted_ids[expr_operands[1]][next_bit]);
          } else {
            bits.push_back(blasted_ids[expr_operands[0]][bit]);
          }
        }
        break;
      }
      blast_word_output(id, expr_operands, width, bits);
      break;
    }
    case ExprGraph::Op::kGather: {
      assert(shape);
      const SignalWidth element_width = width / graph.outer_dim(id);
      for (ExprId operand : expr_operands) {
        bits.insert(bits.end(), blasted_ids[operand].begin(), blasted_ids[operand].end());
      }
      assert(bits.size() == expr_operands.size() * element_width);
      break;
    }
    case ExprGraph::Op::kUnpackedConcat:
      for (ExprId operand : expr_operands) {
        bits.insert(bits.end(), blasted_ids[operand].begin(), blasted_ids[operand].end());
      }
      break;
    case ExprGraph::Op::kUnpackedFlatten:
    case ExprGraph::Op::kUnpackedFold:
      assert(expr_operands.size() == 1);
      bits = blasted_ids[expr_operands.front()];
      break;
    case ExprGraph::Op::kUnpackedAssign: {
      assert(shape);
      propagate_unpacked_assign[id] = true;
      assert(expr_operands.size() == 2);
      const ExprId next = expr_operands[0];
      const ExprId index = expr_operands[1];
      const SignalWidth extent = shape->unpacked_dims.front();
      assert(extent > 0);
      assert(width % extent == 0);
      const SignalWidth element_width = width / extent;
      assert(next != kInvalidExprId);
      assert(blasted_ids[next].size() == element_width);
      const auto static_index = builder.try_evaluate(index);
      if (!static_index) {
        blast_word_output(id, expr_operands, width, bits);
        break;
      }
      for (SignalWidth element = 0; element < extent; ++element) {
        const bool selected =
            *static_index >= 0 && static_cast<SignalWidth>(*static_index) == element;
        for (SignalWidth bit = 0; bit < element_width; ++bit) {
          const ExprId next_bit = blasted_ids[next][bit];
          bits.push_back(selected ? next_bit : kInvalidExprId);
        }
      }
      break;
    }
    case ExprGraph::Op::kUnpackedRangeAssign: {
      assert(shape);
      propagate_unpacked_assign[id] = true;
      const ExprId next = expr_operands[0];
      const SignalWidth slice_width = graph.slice_width(id);
      const SignalWidth extent = shape->unpacked_dims.front();
      assert(extent > 0);
      assert(width % extent == 0);
      const SignalWidth element_width = width / extent;
      assert(blasted_ids[next].size() == slice_width * element_width);
      auto static_base = graph.unpacked_range_assign_base(id);
      if (!static_base) {
        static_base = builder.try_evaluate(expr_operands[1]);
      }
      if (!static_base) {
        blast_word_output(id, expr_operands, width, bits);
        break;
      }
      for (SignalWidth destination = 0; destination < extent; ++destination) {
        for (SignalWidth bit = 0; bit < element_width; ++bit) {
          ExprId selected = kInvalidExprId;
          for (SignalWidth source = 0; source < slice_width; ++source) {
            const ExprId next_bit = blasted_ids[next][source * element_width + bit];
            if (next_bit == kInvalidExprId) {
              continue;
            }
            const BitIndex candidate_base =
                static_cast<BitIndex>(destination) - static_cast<BitIndex>(source);
            if (*static_base == candidate_base) {
              selected = next_bit;
            }
          }
          bits.push_back(selected);
        }
      }
      break;
    }
    case ExprGraph::Op::kSequence: {
      assert(!expr_operands.empty());
      blast_word_output(id, expr_operands, width, bits);
      if (expr_operands.front() == kInvalidExprId) {
        propagate_unpacked_assign[id] = std::ranges::any_of(
            expr_operands.begin() + 1, expr_operands.end(), [&](ExprId operand) {
              return operand != kInvalidExprId && propagate_unpacked_assign[operand];
            });
      }
      break;
    }
    case ExprGraph::Op::kUnpackedSelect: {
      assert(expr_operands.size() == 2);
      const auto index = builder.try_evaluate(expr_operands[1]);
      if (index && *index >= 0) {
        const SignalWidth offset = static_cast<SignalWidth>(*index) * width;
        const auto &data = blasted_ids[expr_operands.front()];
        assert(offset <= data.size() - width);
        bits.insert(bits.end(), data.begin() + offset, data.begin() + offset + width);
        break;
      }
      blast_word_output(id, expr_operands, width, bits);
      break;
    }
    case ExprGraph::Op::kUnpackedRange: {
      assert(shape);
      assert(expr_operands.size() == 2);
      const auto base = builder.try_evaluate(expr_operands[1]);
      if (base && *base >= 0) {
        const SignalWidth element_width = width / graph.outer_dim(id);
        const SignalWidth offset = static_cast<SignalWidth>(*base) * element_width;
        const auto &data = blasted_ids[expr_operands.front()];
        assert(offset <= data.size() - width);
        bits.insert(bits.end(), data.begin() + offset, data.begin() + offset + width);
        break;
      }
      blast_word_output(id, expr_operands, width, bits);
      break;
    }
    default: {
      blast_word_output(id, expr_operands, width, bits);
      break;
    }
    }
    assert(bits.size() == width);
  }
  for (ExprId &root : module.node_output_expr_ids(node_id)) {
    if (root == kInvalidExprId) {
      continue;
    }
    assert(root < expr_count);
    assert(blasted_ids[root].size() == 1);
    root = blasted_ids[root].front();
  }
}

EdgeRef TigTransformer::add_node_output_expr(Tig::Module &module, NodeId node_id, ExprId expr_id,
                                             std::string name) {
  const PortIndex port_idx = static_cast<PortIndex>(module.node_output_count(node_id));
  module.append_node_output(node_id, name, module.node_expr_graph(node_id).outer_dim(expr_id),
                            module.node_expr_graph(node_id).outer_sign(expr_id));
  module.node_output_expr_ids(node_id).push_back(expr_id);
  module.node_output_combs(node_id).push_back(true);
  if (!name.empty()) {
    const auto &graph = module.node_expr_graph(node_id);
    append_signal(module, name, graph.outer_dim(expr_id), graph.outer_sign(expr_id),
                  graph.unpacked_shape(expr_id));
  }
  return {node_id, port_idx};
}

std::string TigTransformer::create_temporary_name(Tig::Module &module) const {
  return naming_.transformer_temporary_signal_prefix +
         std::to_string(module.transform_name_count++);
}

void TigTransformer::append_signal(Tig::Module &module, const std::string &name,
                                   SignalWidth outer_dim, bool outer_sign,
                                   const UnpackedShape *shape) {
  if (!module.signals.append(name, outer_dim, outer_sign, shape)) {
    diagnostics_.error(DiagnosticId::kDuplicateSignalName, name);
  }
}

TigTransformer::TigTransformer(Tig &design, Diagnostics &diagnostics, const NamingOptions &naming)
    : design_(design), diagnostics_(diagnostics), naming_(naming) {}

void TigTransformer::flatten_subroutine() {
  for (auto &module : design_.modules) {
    for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
      if (module.node_kind(node_id) != Tig::Module::NodeKind::kOp) {
        continue;
      }
      ExprGraph &expr_graph = module.node_expr_graph(node_id);
      bool rewrote_call = false;
      std::unordered_map<ExprId, std::vector<SubrId>> call_ancestries;
      auto replace_call_with_zero = [&](ExprId call_id, std::string detail) {
        diagnostics_.error(DiagnosticId::kLoweringUnsupportedExpressionReplacedWithZero,
                           std::move(detail));
        expr_graph.set_op(call_id, ExprGraph::Op::kConvert);
        expr_graph.set_operands(call_id, {ExprGraph::constant_zero});
      };
      for (ExprId call_id = 0; call_id < expr_graph.size(); ++call_id) {
        if (expr_graph.op(call_id) != ExprGraph::Op::kCall) {
          continue;
        }
        rewrote_call = true;
        const SubrId subr_id = expr_graph.call_target(call_id);
        bool recursive = false;
        for (SubrId ancestor : call_ancestries[call_id]) {
          if (ancestor == subr_id) {
            recursive = true;
            break;
          }
        }
        if (recursive) {
          diagnostics_.error(DiagnosticId::kTransformRecursiveSubroutineCallReplacedWithZero,
                             expr_graph.call_name(call_id));
          expr_graph.set_op(call_id, ExprGraph::Op::kConvert);
          expr_graph.set_operands(call_id, {ExprGraph::constant_zero});
          continue;
        }
        if (subr_id >= design_.subroutines.size() ||
            design_.subroutines[subr_id].output_expr_id == kInvalidExprId) {
          replace_call_with_zero(call_id, "unknown subroutine: " + expr_graph.call_name(call_id));
          continue;
        }
        const Tig::Subroutine &subr = design_.subroutines[subr_id];
        if (expr_graph.operands(call_id).size() != subr.inputs.size()) {
          replace_call_with_zero(call_id, "call arity mismatch: " + expr_graph.call_name(call_id));
          continue;
        }
        std::unordered_map<ExprId, ExprId> id_map;
        id_map.reserve(subr.expr_graph.size() + 1);
        id_map.emplace(kInvalidExprId, kInvalidExprId);
        bool call_valid = true;
        assert(subr.inputs.size() == subr.input_expr_ids.size());
        for (size_t j = 0; j < subr.inputs.size(); ++j) {
          id_map.emplace(subr.input_expr_ids[j], expr_graph.operands(call_id)[j]);
        }
        assert(subr.captures.size() == subr.capture_expr_ids.size());
        for (size_t j = 0; j < subr.captures.size(); ++j) {
          const auto capture = subr.captures[j];
          assert(capture.node_id != kInvalidNodeId);
          assert(&module == &design_.modules.at(subr.module_id));
          const ExprId capture_id = add_node_input_expr(module, node_id, capture);
          id_map.emplace(subr.capture_expr_ids[j], capture_id);
        }
        if (!call_valid) {
          continue;
        }
        id_map.emplace(ExprGraph::constant_zero, ExprGraph::constant_zero);
        id_map.emplace(ExprGraph::constant_one, ExprGraph::constant_one);
        for (ExprId src_id = 2; src_id < subr.expr_graph.size(); ++src_id) {
          if (subr.expr_graph.op(src_id) == ExprGraph::Op::kConst) {
            id_map.emplace(src_id, expr_graph.append_copy(subr.expr_graph, src_id, {}));
          }
        }
        for (ExprId src_id = 0; src_id < static_cast<ExprId>(subr.expr_graph.size()); ++src_id) {
          if (id_map.find(src_id) != id_map.end()) {
            continue;
          }
          std::vector<ExprId> new_ops;
          new_ops.reserve(subr.expr_graph.operands(src_id).size());
          for (ExprId sop : subr.expr_graph.operands(src_id)) {
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
          const ExprId dst_id = expr_graph.append_copy(subr.expr_graph, src_id, std::move(new_ops));
          id_map.emplace(src_id, dst_id);
          if (subr.expr_graph.op(src_id) == ExprGraph::Op::kCall) {
            auto ancestry = call_ancestries[call_id];
            ancestry.push_back(subr_id);
            call_ancestries.emplace(dst_id, std::move(ancestry));
          }
        }
        if (!call_valid) {
          continue;
        }
        auto rit = id_map.find(subr.output_expr_id);
        if (rit == id_map.end()) {
          replace_call_with_zero(call_id, "failed to map subroutine root: " + subr.name);
          continue;
        }
        const ExprId inlined_root = rit->second;
        expr_graph.set_op(call_id, ExprGraph::Op::kConvert); // temporary buffer op
        expr_graph.set_operands(call_id, {inlined_root});
      }
      if (rewrote_call) {
        clean_op_node(module, node_id);
      }
    }
  }
  design_.subroutines.clear();
}

std::vector<EdgeRef> TigTransformer::create_memory_writes(Tig::Module &module, NodeId op_id,
                                                          ExprId sequence_id) {
  assert(module.node_kind(op_id) == Tig::Module::NodeKind::kOp);
  assert(module.node_expr_graph(op_id).op(sequence_id) == ExprGraph::Op::kSequence);
  ExprBuilder builder(module.node_expr_graph(op_id), diagnostics_);
  std::vector<SignalWidth> full_extents;
  if (const auto *shape = module.node_expr_graph(op_id).unpacked_shape(sequence_id)) {
    full_extents = shape->unpacked_dims;
  }
  assert(!full_extents.empty());
  struct LocalWrite {
    EdgeRef enable;
    EdgeRef data;
    std::vector<EdgeRef> region;
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
    const auto expr_operands = module.node_expr_graph(op_id).operands(id);
    if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kSequence) {
      assert(!expr_operands.empty());
      for (size_t operand = 1; operand < expr_operands.size(); ++operand) {
        self(self, expr_operands[operand], enable);
      }
      return;
    }
    if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kMux ||
        module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kUnpackedMux) {
      const ExprId condition = expr_operands.at(0);
      self(self, expr_operands.at(1), conjunction(enable, condition));
      self(self, expr_operands.at(2), conjunction(enable, builder.create_logical_not(condition)));
      return;
    }
    if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kCase ||
        module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kUnpackedCase) {
      const ExprId selector = expr_operands.at(0);
      ExprId unmatched = ExprGraph::constant_one;
      size_t operand = 1;
      while (operand + 1 < expr_operands.size()) {
        const ExprId match = builder.create_match(selector, expr_operands[operand]);
        self(self, expr_operands[operand + 1], conjunction(enable, conjunction(unmatched, match)));
        unmatched = conjunction(unmatched, builder.create_logical_not(match));
        operand += 2;
      }
      if (operand < expr_operands.size()) {
        self(self, expr_operands[operand], conjunction(enable, unmatched));
      }
      return;
    }
    std::vector<ExprId> region;
    std::vector<bool> region_ranges;
    ExprId data = id;
    while (module.node_expr_graph(op_id).op(data) == ExprGraph::Op::kUnpackedAssign ||
           module.node_expr_graph(op_id).op(data) == ExprGraph::Op::kUnpackedRangeAssign) {
      const auto assign_operands = module.node_expr_graph(op_id).operands(data);
      const bool range =
          module.node_expr_graph(op_id).op(data) == ExprGraph::Op::kUnpackedRangeAssign;
      region.push_back(range ? materialize_unpacked_range_assign_base(module.node_expr_graph(op_id),
                                                                      builder, data)
                             : assign_operands.at(1));
      const auto width = range ? module.node_expr_graph(op_id).slice_width(data) : 1;
      region.push_back(builder.find_or_create_const(
          static_cast<BitIndex>(width), ExprBuilder::minimum_unsigned_width(width), false));
      region_ranges.push_back(range);
      data = assign_operands.at(0);
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
    std::vector<EdgeRef> region_refs;
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

  const SignalWidth outer_dim = module.node_expr_graph(op_id).outer_dim(sequence_id);
  const bool sign = module.node_expr_graph(op_id).outer_sign(sequence_id);
  std::vector<EdgeRef> writes;
  for (auto &local : local_writes) {
    const NodeId write_id = module.append_node(Tig::Module::NodeKind::kMemoryWrite);
    module.node_inputs(write_id) = {local.enable, local.data};
    module.node_inputs(write_id).insert(module.node_inputs(write_id).end(), local.region.begin(),
                                        local.region.end());
    module.node_memory_range_flags(write_id) = std::move(local.region_ranges);
    module.append_node_output(write_id, "", outer_dim, sign);
    module.node_output_combs(write_id).push_back(false);
    writes.push_back({write_id, 0});
  }
  return writes;
}

bool TigTransformer::create_memory_reads(Tig::Module &module, NodeId op_id, ExprId id,
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
  auto expr_operands = module.node_expr_graph(op_id).operands(id);
  if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kUnpackedSelect ||
      module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kUnpackedRange) {
    for (size_t operand = 1; operand < expr_operands.size(); ++operand) {
      create_memory_reads(module, op_id, expr_operands[operand], kInvalidExprId, nullptr, visited,
                          pending_reads);
    }
    std::vector<MemoryReadDimension> new_region;
    if (region == nullptr) {
      read_id = id;
      region = &new_region;
    }
    const SignalWidth extent =
        module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kUnpackedSelect
            ? 1
            : module.node_expr_graph(op_id).outer_dim(id);
    region->push_back({expr_operands.at(1), extent,
                       module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kUnpackedRange});
    return create_memory_reads(module, op_id, expr_operands.at(0), read_id, region, visited,
                               pending_reads);
  }
  if (read_id != kInvalidExprId) {
    if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kInput) {
      assert(module.node_inputs(op_id).size() == module.node_input_expr_ids(op_id).size());
      for (size_t port = 0; port < module.node_inputs(op_id).size(); ++port) {
        if (module.node_input_expr_ids(op_id)[port] != id) {
          continue;
        }
        const auto input = module.node_inputs(op_id)[port];
        const auto source_id = input.node_id;
        const bool extends_read = module.node_kind(source_id) == Tig::Module::NodeKind::kMemoryRead;
        if (!extends_read && module.node_kind(source_id) != Tig::Module::NodeKind::kMemory &&
            (module.node_kind(source_id) != Tig::Module::NodeKind::kJoin ||
             module.node_kind(module.node_inputs(source_id).front().node_id) !=
                 Tig::Module::NodeKind::kMemory)) {
          continue;
        }
        std::vector<EdgeRef> region_refs;
        ExprBuilder expr_builder(module.node_expr_graph(op_id), diagnostics_);
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
        std::vector<EdgeRef> read_inputs;
        std::vector<bool> region_ranges;
        std::vector<bool> new_region_ranges;
        for (auto dimension = region->rbegin(); dimension != region->rend(); ++dimension) {
          new_region_ranges.push_back(dimension->range);
        }
        if (extends_read) {
          read_inputs = module.node_inputs(source_id);
          region_ranges = module.node_memory_range_flags(source_id);
        } else {
          const auto ren = add_node_output_expr(module, op_id, ExprGraph::constant_one,
                                                create_temporary_name(module));
          read_inputs = {ren, input};
        }
        if (extends_read && region_ranges.back()) {
          const auto old_index_ref = read_inputs.at(read_inputs.size() - 2);
          assert(old_index_ref.node_id == op_id);
          const ExprId old_index = module.node_output_expr_ids(op_id).at(old_index_ref.port_idx);
          const auto new_index_ref = region_refs.at(0);
          const ExprId new_index = module.node_output_expr_ids(op_id).at(new_index_ref.port_idx);
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
        const NodeId memory_read_id = module.append_node(Tig::Module::NodeKind::kMemoryRead);
        module.node_inputs(memory_read_id) = std::move(read_inputs);
        module.node_memory_range_flags(memory_read_id) = std::move(region_ranges);
        module.append_node_output(memory_read_id, "",
                                  module.node_expr_graph(op_id).outer_dim(read_id),
                                  module.node_expr_graph(op_id).outer_sign(read_id));
        module.node_output_combs(memory_read_id).push_back(true);
        pending_reads.push_back({read_id, memory_read_id});
        return true;
      }
      return false;
    }

    const size_t first_operand =
        module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kSequence ? 1 : 0;
    for (size_t operand = first_operand; operand < expr_operands.size(); ++operand) {
      create_memory_reads(module, op_id, expr_operands[operand], kInvalidExprId, nullptr, visited,
                          pending_reads);
    }
    if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kSequence) {
      assert(!expr_operands.empty());
      const bool resolved = create_memory_reads(module, op_id, expr_operands.front(), read_id,
                                                region, visited, pending_reads);
      if (resolved) {
        diagnostics_.error(DiagnosticId::kTransformUnsupportedMemoryRead,
                           "memory read after write ignores previous writes");
      }
      return resolved;
    }
    if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kCall) {
      diagnostics_.warning(DiagnosticId::kTransformUnsupportedMemoryRead,
                           "memory read through subroutine call was not extracted");
      return false;
    }
    diagnostics_.error(DiagnosticId::kTransformUnsupportedMemoryRead,
                       "memory source is not a memory input or sequence");
    return false;
  }
  for (ExprId operand : expr_operands) {
    create_memory_reads(module, op_id, operand, kInvalidExprId, nullptr, visited, pending_reads);
  }
  return false;
}

void TigTransformer::infer_memory() {
  for (auto &module : design_.modules) {
    const size_t original_node_count = module.size();
    std::vector<bool> modified_ops(original_node_count, false);

    for (NodeId op_id = 0; op_id < original_node_count; ++op_id) {
      if (module.node_kind(op_id) != Tig::Module::NodeKind::kOp) {
        continue;
      }
      std::vector<PendingMemoryRead> pending_reads;
      std::vector<bool> visited(module.node_expr_graph(op_id).size(), false);
      const std::vector<ExprId> roots = module.node_output_expr_ids(op_id);
      for (ExprId root : roots) {
        create_memory_reads(module, op_id, root, kInvalidExprId, nullptr, visited, pending_reads);
      }

      for (const auto &pending : pending_reads) {
        module.node_expr_graph(op_id).set_op(pending.id, ExprGraph::Op::kInput);
        module.node_expr_graph(op_id).set_operands(pending.id, {});
        const std::string name = create_temporary_name(module);
        module.node_output_name(pending.node_id, 0) = name;
        const auto &graph = module.node_expr_graph(op_id);
        append_signal(module, name, graph.outer_dim(pending.id), graph.outer_sign(pending.id),
                      graph.unpacked_shape(pending.id));
        module.node_inputs(op_id).push_back({pending.node_id, 0});
        module.node_input_expr_ids(op_id).push_back(pending.id);
      }
      modified_ops[op_id] = !pending_reads.empty();
    }

    for (NodeId memory_id = 0; memory_id < original_node_count; ++memory_id) {
      if (module.node_kind(memory_id) != Tig::Module::NodeKind::kMemory) {
        continue;
      }
      auto update = module.node_inputs(memory_id).at(0);
      auto update_kind = module.node_kind(update.node_id);
      if (update_kind == Tig::Module::NodeKind::kMemoryWrite) {
        continue;
      }
      const bool has_multi_driver = update_kind == Tig::Module::NodeKind::kMultiDriver;
      if (has_multi_driver) {
        bool already_inferred = !module.node_inputs(update.node_id).empty();
        for (const auto &source : module.node_inputs(update.node_id)) {
          if (module.node_kind(source.node_id) != Tig::Module::NodeKind::kMemoryWrite) {
            already_inferred = false;
            break;
          }
        }
        if (already_inferred) {
          continue;
        }
      }
      const std::vector<EdgeRef> sources =
          has_multi_driver ? module.node_inputs(update.node_id) : std::vector<EdgeRef>{update};
      std::vector<EdgeRef> writes;
      for (auto source : sources) {
        const auto op_id = source.node_id;
        assert(module.node_kind(op_id) == Tig::Module::NodeKind::kOp);
        const ExprId root = module.node_output_expr_ids(op_id).at(source.port_idx);
        const auto root_writes = create_memory_writes(module, source.node_id, root);
        writes.insert(writes.end(), root_writes.begin(), root_writes.end());
        module.node_output_expr_ids(source.node_id)[source.port_idx] = kInvalidExprId;
        modified_ops[source.node_id] = true;
      }
      assert(!writes.empty());
      if (writes.size() == 1) {
        module.node_inputs(memory_id)[0] = writes.front();
      } else if (has_multi_driver) {
        module.node_inputs(update.node_id) = std::move(writes);
      } else {
        const NodeId driver_id = module.append_node(Tig::Module::NodeKind::kMultiDriver);
        module.node_inputs(driver_id) = std::move(writes);
        module.append_node_output(driver_id,
                                  module.node_output_name(update.node_id, update.port_idx),
                                  module.node_output_outer_dim(update.node_id, update.port_idx),
                                  module.node_output_outer_sign(update.node_id, update.port_idx));
        module.node_output_combs(driver_id).push_back(false);
        module.node_inputs(memory_id)[0] = {driver_id, 0};
      }
    }

    for (NodeId op_id = 0; op_id < original_node_count; ++op_id) {
      if (modified_ops[op_id]) {
        clean_op_node(module, op_id);
      }
    }
  }
}

void TigTransformer::decompose_dynamic_access() {
  for (auto &module : design_.modules) {
    for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
      if (module.node_kind(node_id) != Tig::Module::NodeKind::kOp) {
        continue;
      }
      ExprBuilder builder(module.node_expr_graph(node_id), diagnostics_);
      const size_t expression_count = module.node_expr_graph(node_id).size();
      std::vector<bool> visited(expression_count);
      bool modified = false;
      const auto decompose = [&](const auto &self, ExprId id) -> void {
        if (id == kInvalidExprId || id >= expression_count || visited[id]) {
          return;
        }
        visited[id] = true;
        const auto expression_operands = module.node_expr_graph(node_id).operands(id);
        for (ExprId operand : expression_operands) {
          self(self, operand);
        }
        switch (module.node_expr_graph(node_id).op(id)) {
        case ExprGraph::Op::kShl:
        case ExprGraph::Op::kShr:
        case ExprGraph::Op::kAshr: {
          assert(expression_operands.size() == 2);
          if (builder.try_evaluate(expression_operands[1]).has_value()) {
            return;
          }
          const ExprId data = expression_operands[0];
          const SignalWidth outer_dim = builder.get_outer_dim(data);
          assert(outer_dim > 0);
          assert(outer_dim <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
          std::vector<ExprId> lanes;
          lanes.reserve(outer_dim);
          for (SignalWidth lane = 0; lane < outer_dim; ++lane) {
            lanes.push_back(builder.create_static_range(data, lane, 1, false));
          }
          const bool reverse = module.node_expr_graph(node_id).op(id) == ExprGraph::Op::kShl;
          if (reverse) {
            std::reverse(lanes.begin(), lanes.end());
          }
          const ExprId fill = module.node_expr_graph(node_id).op(id) == ExprGraph::Op::kAshr
                                  ? lanes.back()
                                  : ExprGraph::constant_zero;
          lanes = create_barrel_shift(builder, std::move(lanes), expression_operands[1], fill);
          if (!reverse) {
            std::reverse(lanes.begin(), lanes.end());
          }
          const ExprId expanded =
              builder.create_concat(std::move(lanes), builder.get_outer_sign(data));
          modified = true;
          module.node_expr_graph(node_id).set_op(id, ExprGraph::Op::kConvert);
          module.node_expr_graph(node_id).set_operands(id, {expanded});
          return;
        }
        case ExprGraph::Op::kRange: {
          assert(!expression_operands.empty());
          const ExprId data = expression_operands[0];
          const auto affine = builder.decode_affine_index(id);
          if (affine.terms.empty()) {
            return;
          }
          const SignalWidth data_outer_dim = builder.get_outer_dim(data);
          assert(module.node_expr_graph(node_id).outer_dim(id) > 0);
          assert(module.node_expr_graph(node_id).outer_dim(id) <= data_outer_dim);
          const ExprId fill = builder.find_or_create_const("1'bx", 1, false);
          std::vector<ExprId> result;
          if (can_decompose_affine_index(builder, affine) &&
              static_cast<uint64_t>(affine.offset) +
                      module.node_expr_graph(node_id).outer_dim(id) <=
                  std::numeric_limits<SignalWidth>::max()) {
            const SignalWidth offset = static_cast<SignalWidth>(affine.offset);
            const SignalWidth lane_count = std::max(
                data_outer_dim,
                static_cast<SignalWidth>(offset + module.node_expr_graph(node_id).outer_dim(id)));
            std::vector<ExprId> lanes;
            lanes.reserve(lane_count);
            for (SignalWidth bit = 0; bit < data_outer_dim; ++bit) {
              lanes.push_back(builder.create_static_range(data, bit, 1, false));
            }
            lanes.resize(lane_count, fill);
            for (const auto &term : affine.terms) {
              lanes = create_strided_barrel_shift(builder, std::move(lanes), term.index,
                                                  term.stride, fill);
            }
            result.assign(lanes.begin() + offset,
                          lanes.begin() + offset + module.node_expr_graph(node_id).outer_dim(id));
          } else {
            diagnostics_.warning(DiagnosticId::kTransformAffineIndexMaterialized, "packed range");
            const ExprId index = materialize_affine_index(builder, affine);
            const SignalWidth negative_positions =
                module.node_expr_graph(node_id).outer_dim(id) - 1;
            std::vector<ExprId> lanes(negative_positions, fill);
            for (SignalWidth bit = 0; bit < data_outer_dim; ++bit) {
              lanes.push_back(builder.create_static_range(data, bit, 1, false));
            }
            lanes.insert(lanes.end(), module.node_expr_graph(node_id).outer_dim(id) - 1, fill);
            ExprId amount = index;
            if (negative_positions > 0) {
              const SignalWidth amount_width =
                  std::max(builder.get_outer_dim(index),
                           ExprBuilder::minimum_unsigned_width(negative_positions) + 1);
              const ExprId alignment = builder.find_or_create_const(
                  static_cast<BitIndex>(negative_positions), amount_width, true);
              amount = builder.create_add(index, alignment);
            }
            lanes = create_barrel_shift(builder, std::move(lanes), amount, fill);
            result.assign(lanes.begin(),
                          lanes.begin() + module.node_expr_graph(node_id).outer_dim(id));
          }
          std::reverse(result.begin(), result.end());
          const ExprId expanded = builder.create_concat(
              std::move(result), module.node_expr_graph(node_id).outer_sign(id));
          modified = true;
          module.node_expr_graph(node_id).set_op(id, ExprGraph::Op::kConvert);
          module.node_expr_graph(node_id).set_operands(id, {expanded});
          return;
        }
        case ExprGraph::Op::kMaskedAssign: {
          assert(expression_operands.size() >= 2);
          const ExprId current = expression_operands[0];
          const ExprId next = expression_operands[1];
          const auto affine = builder.decode_affine_index(id);
          if (affine.terms.empty()) {
            return;
          }
          const SignalWidth slice_width = module.node_expr_graph(node_id).slice_width(id);
          const SignalWidth outer_dim = builder.get_outer_dim(current);
          assert(slice_width <= outer_dim);
          assert(builder.get_outer_dim(next) == slice_width);
          if (can_decompose_affine_index(builder, affine) &&
              static_cast<uint64_t>(affine.offset) + slice_width <=
                  std::numeric_limits<SignalWidth>::max()) {
            const SignalWidth offset = static_cast<SignalWidth>(affine.offset);
            const SignalWidth lane_count =
                std::max(outer_dim, static_cast<SignalWidth>(offset + slice_width));
            std::vector<ExprId> next_lanes(lane_count, ExprGraph::constant_zero);
            std::vector<ExprId> mask_lanes(lane_count, ExprGraph::constant_zero);
            for (SignalWidth bit = 0; bit < slice_width; ++bit) {
              next_lanes[offset + bit] = builder.create_static_range(next, bit, 1, false);
              mask_lanes[offset + bit] = ExprGraph::constant_one;
            }
            for (const auto &term : affine.terms) {
              assert(term.stride != std::numeric_limits<BitIndex>::min());
              next_lanes = create_strided_barrel_shift(builder, std::move(next_lanes), term.index,
                                                       -term.stride, ExprGraph::constant_zero);
              mask_lanes = create_strided_barrel_shift(builder, std::move(mask_lanes), term.index,
                                                       -term.stride, ExprGraph::constant_zero);
            }
            std::vector<ExprId> result;
            result.reserve(outer_dim);
            for (SignalWidth bit = 0; bit < outer_dim; ++bit) {
              const ExprId current_bit = builder.create_static_range(current, bit, 1, false);
              result.push_back(builder.create_mux(mask_lanes[bit], next_lanes[bit], current_bit));
            }
            std::reverse(result.begin(), result.end());
            const ExprId expanded = builder.create_concat(
                std::move(result), module.node_expr_graph(node_id).outer_sign(id));
            modified = true;
            module.node_expr_graph(node_id).set_op(id, ExprGraph::Op::kConvert);
            module.node_expr_graph(node_id).set_operands(id, {expanded});
            return;
          }
          diagnostics_.warning(DiagnosticId::kTransformAffineIndexMaterialized,
                               "packed masked assignment");
          const ExprId index = materialize_affine_index(builder, affine);
          const SignalWidth negative_positions = slice_width - 1;
          ExprId amount = index;
          if (negative_positions > 0) {
            const SignalWidth amount_width =
                std::max(builder.get_outer_dim(index),
                         ExprBuilder::minimum_unsigned_width(negative_positions) + 1);
            const ExprId shift_offset = builder.find_or_create_const(
                static_cast<BitIndex>(negative_positions), amount_width, true);
            amount = builder.create_add(index, shift_offset);
          }
          std::vector<ExprId> result(outer_dim, kInvalidExprId);
          std::vector<ExprId> next_lanes;
          std::vector<ExprId> mask_lanes;
          for (SignalWidth bit = 0; bit < outer_dim; ++bit) {
            if (bit < slice_width) {
              next_lanes.push_back(builder.create_static_range(next, bit, 1, false));
              mask_lanes.push_back(ExprGraph::constant_one);
            } else {
              next_lanes.push_back(ExprGraph::constant_zero);
              mask_lanes.push_back(ExprGraph::constant_zero);
            }
          }
          next_lanes.insert(next_lanes.end(), negative_positions, ExprGraph::constant_zero);
          mask_lanes.insert(mask_lanes.end(), negative_positions, ExprGraph::constant_zero);
          std::reverse(next_lanes.begin(), next_lanes.end());
          std::reverse(mask_lanes.begin(), mask_lanes.end());
          next_lanes =
              create_barrel_shift(builder, std::move(next_lanes), amount, ExprGraph::constant_zero);
          mask_lanes =
              create_barrel_shift(builder, std::move(mask_lanes), amount, ExprGraph::constant_zero);
          std::reverse(next_lanes.begin(), next_lanes.end());
          std::reverse(mask_lanes.begin(), mask_lanes.end());
          for (SignalWidth bit = 0; bit < outer_dim; ++bit) {
            const size_t lane = negative_positions + bit;
            const ExprId current_bit = builder.create_static_range(current, bit, 1, false);
            result[bit] = builder.create_mux(mask_lanes[lane], next_lanes[lane], current_bit);
          }
          assert(std::ranges::none_of(result, [](ExprId bit) { return bit == kInvalidExprId; }));
          std::reverse(result.begin(), result.end());
          const ExprId expanded = builder.create_concat(
              std::move(result), module.node_expr_graph(node_id).outer_sign(id));
          modified = true;
          module.node_expr_graph(node_id).set_op(id, ExprGraph::Op::kConvert);
          module.node_expr_graph(node_id).set_operands(id, {expanded});
          return;
        }
        case ExprGraph::Op::kUnpackedRange: {
          assert(expression_operands.size() == 2);
          if (builder.try_evaluate(expression_operands[1]).has_value()) {
            return;
          }
          decompose_unpacked_range(module, node_id, builder, id);
          modified = true;
          return;
        }
        case ExprGraph::Op::kUnpackedSelect: {
          assert(expression_operands.size() == 2);
          const ExprId data = expression_operands[0];
          const ExprId index = expression_operands[1];
          if (builder.try_evaluate(index).has_value()) {
            return;
          }
          const auto data_shape = get_unpacked_shape(module, node_id, data);
          assert(data_shape.has_value());
          std::vector<SignalWidth> remaining_dims(data_shape->unpacked_dims.begin() + 1,
                                                  data_shape->unpacked_dims.end());
          std::vector<ExprId> elements =
              create_unpacked_element_selects(builder, data, *data_shape);
          SignalWidth fill_width = data_shape->inner_dim;
          for (SignalWidth dimension : remaining_dims) {
            assert(dimension > 0);
            assert(fill_width <= std::numeric_limits<SignalWidth>::max() / dimension);
            fill_width *= dimension;
          }
          ExprId fill = builder.find_or_create_const(std::to_string(fill_width) + "'bx", fill_width,
                                                     data_shape->inner_sign);
          if (!remaining_dims.empty()) {
            fill = builder.create_unpacked_fold(fill, remaining_dims, data_shape->inner_dim,
                                                data_shape->inner_sign);
          }
          elements = create_barrel_shift(builder, std::move(elements), index, fill);
          const ExprId expanded = elements.front();
          modified = true;
          module.node_expr_graph(node_id).set_op(id, module.node_expr_graph(node_id).op(expanded));
          module.node_expr_graph(node_id).set_operands(
              id, module.node_expr_graph(node_id).operands(expanded));
          return;
        }
        case ExprGraph::Op::kSequence:
          assert(!expression_operands.empty());
          if (expression_operands.front() == kInvalidExprId) {
            return;
          }
          decompose_unpacked_sequence(module, node_id, builder, id);
          modified = true;
          return;
        default:
          return;
        }
      };
      for (ExprId root : module.node_output_expr_ids(node_id)) {
        decompose(decompose, root);
      }
      if (modified) {
        clean_op_node(module, node_id);
      }
    }
  }
}

bool TigTransformer::has_fixed_inputs(Tig::Module::NodeKind kind) {
  using NodeKind = Tig::Module::NodeKind;
  switch (kind) {
  case NodeKind::kPi:
  case NodeKind::kPo:
  case NodeKind::kInstance:
  case NodeKind::kMemory:
  case NodeKind::kMemoryRead:
  case NodeKind::kMemoryWrite:
  case NodeKind::kAndNetwork:
  case NodeKind::kBoundNetwork:
  case NodeKind::kMacro:
  case NodeKind::kUnknown:
  case NodeKind::kFlatten:
    return true;
  default:
    return false;
  }
}

bool TigTransformer::has_fixed_outputs(Tig::Module::NodeKind kind) {
  using NodeKind = Tig::Module::NodeKind;
  switch (kind) {
  case NodeKind::kPi:
  case NodeKind::kPo:
  case NodeKind::kInstance:
  case NodeKind::kMemory:
  case NodeKind::kMemoryRead:
  case NodeKind::kMemoryWrite:
  case NodeKind::kAndNetwork:
  case NodeKind::kBoundNetwork:
  case NodeKind::kMacro:
  case NodeKind::kUnknown:
  case NodeKind::kFold:
  case NodeKind::kConstant:
    return true;
  default:
    return false;
  }
}

std::optional<UnpackedShape> TigTransformer::get_output_shape(const Tig::Module &module,
                                                              NodeId node_id,
                                                              PortIndex port_idx) const {
  using NodeKind = Tig::Module::NodeKind;
  assert(node_id < module.size());
  assert(port_idx < module.node_output_count(node_id));
  if (module.node_kind(node_id) == NodeKind::kOp) {
    assert(port_idx < module.node_output_expr_ids(node_id).size());
    const ExprId root = module.node_output_expr_ids(node_id)[port_idx];
    return root == kInvalidExprId ? std::nullopt : get_unpacked_shape(module, node_id, root);
  }
  const UnpackedShape *shape = nullptr;
  if (module.node_kind(node_id) == NodeKind::kInstance) {
    assert(module.node_module_id(node_id) < design_.modules.size());
    const auto &ports = design_.modules[module.node_module_id(node_id)].output_ports;
    shape = ports.shape(port_idx);
  } else {
    const auto &name = module.node_output_name(node_id, port_idx);
    if (!name.empty()) {
      if (const auto index = module.signals.find_index(name)) {
        shape = module.signals.shape(*index);
      }
    }
  }
  return shape ? std::optional<UnpackedShape>(*shape) : std::nullopt;
}

EdgeRef TigTransformer::insert_flatten(Tig::Module &module, EdgeRef input,
                                       const std::optional<UnpackedShape> &shape) {
  using NodeKind = Tig::Module::NodeKind;
  assert(input.node_id != kInvalidNodeId);
  assert(input.node_id < module.size());
  assert(input.port_idx < module.node_output_count(input.node_id));
  auto &output_name = module.node_output_name(input.node_id, input.port_idx);
  const auto output_outer_dim = module.node_output_outer_dim(input.node_id, input.port_idx);
  const auto output_outer_sign = module.node_output_outer_sign(input.node_id, input.port_idx);
  if (output_name.empty()) {
    output_name = create_temporary_name(module);
    append_signal(module, output_name, output_outer_dim, output_outer_sign,
                  shape ? &*shape : nullptr);
  }
  const NodeId flatten_id = module.append_node(NodeKind::kFlatten);
  module.node_inputs(flatten_id).push_back(input);
  module.append_node_output(flatten_id, create_temporary_name(module),
                            shape ? shape->flattened_width() : output_outer_dim,
                            shape ? shape->inner_sign : output_outer_sign);
  module.node_output_combs(flatten_id).push_back(true);
  return {flatten_id, 0};
}

EdgeRef TigTransformer::insert_fold(Tig::Module &module, EdgeRef input,
                                    const std::optional<UnpackedShape> &shape) {
  using NodeKind = Tig::Module::NodeKind;
  assert(input.node_id != kInvalidNodeId);
  assert(input.node_id < module.size());
  assert(input.port_idx < module.node_output_count(input.node_id));
  auto &output_name = module.node_output_name(input.node_id, input.port_idx);
  const auto output_outer_dim = module.node_output_outer_dim(input.node_id, input.port_idx);
  const auto output_outer_sign = module.node_output_outer_sign(input.node_id, input.port_idx);
  const bool has_name = !output_name.empty();
  std::string name = has_name ? output_name : create_temporary_name(module);
  const auto source_shape = get_output_shape(module, input.node_id, input.port_idx);
  output_name = create_temporary_name(module);
  append_signal(module, output_name, output_outer_dim, output_outer_sign,
                source_shape ? &*source_shape : nullptr);
  std::optional<std::string> constant_value;
  const auto source_id = input.node_id;
  if (!shape && module.node_kind(source_id) == NodeKind::kOp) {
    assert(input.port_idx < module.node_output_expr_ids(source_id).size());
    const ExprId root = module.node_output_expr_ids(source_id)[input.port_idx];
    const auto &graph = module.node_expr_graph(source_id);
    if (root != kInvalidExprId && graph.op(root) == ExprGraph::Op::kConst) {
      constant_value = graph.constant_value(root);
    } else if (root != kInvalidExprId) {
      // TODO: Remove this evaluation once constant propagation runs before bit-blasting.
      ExprGraph source_expr_graph = graph;
      ExprBuilder source_builder(source_expr_graph, diagnostics_);
      if (const auto value = source_builder.try_evaluate(root)) {
        constant_value = format_integral_constant(*value, output_outer_dim, output_outer_sign);
      }
    }
  }
  const NodeId adapter_id =
      module.append_node(constant_value ? NodeKind::kConstant : NodeKind::kFold);
  const auto fold_outer_dim = shape ? shape->unpacked_dims.front() : output_outer_dim;
  const bool fold_outer_sign = shape ? false : output_outer_sign;
  module.append_node_output(adapter_id, name, fold_outer_dim, fold_outer_sign);
  module.node_output_combs(adapter_id).push_back(true);
  if (constant_value) {
    module.node_constant_value(adapter_id) = std::move(*constant_value);
  } else {
    module.node_inputs(adapter_id).push_back(input);
  }
  if (!has_name) {
    append_signal(module, name, fold_outer_dim, fold_outer_sign, shape ? &*shape : nullptr);
  }
  return {adapter_id, 0};
}

void TigTransformer::insert_fixed_interfaces(Tig::Module &module) {
  using NodeKind = Tig::Module::NodeKind;
  const size_t node_count = module.size();
  std::vector<size_t> input_counts;
  input_counts.reserve(node_count);
  for (NodeId node_id = 0; node_id < node_count; ++node_id) {
    input_counts.push_back(module.node_inputs(node_id).size());
  }
  for (NodeId consumer_id = 0; consumer_id < node_count; ++consumer_id) {
    const NodeKind consumer_kind = module.node_kind(consumer_id);
    for (size_t input_idx = 0; input_idx < input_counts[consumer_id]; ++input_idx) {
      EdgeRef input = module.node_inputs(consumer_id)[input_idx];
      assert(input.node_id != kInvalidNodeId);
      const NodeKind source_kind = module.node_kind(input.node_id);
      if (has_fixed_outputs(source_kind) && !has_fixed_inputs(consumer_kind)) {
        const auto shape = get_output_shape(module, input.node_id, input.port_idx);
        if (shape || module.node_output_outer_dim(input.node_id, input.port_idx) > 1) {
          input = insert_flatten(module, input, shape);
        }
      } else if (has_fixed_inputs(consumer_kind) && !has_fixed_outputs(source_kind)) {
        std::optional<UnpackedShape> shape;
        if (consumer_kind == NodeKind::kInstance) {
          const auto &ports = design_.modules[module.node_module_id(consumer_id)].input_ports;
          if (const auto *port_shape = ports.shape(input_idx)) {
            shape = *port_shape;
          }
        } else {
          shape = get_output_shape(module, input.node_id, input.port_idx);
        }
        if (shape || module.node_output_outer_dim(input.node_id, input.port_idx) > 1) {
          input = insert_fold(module, input, shape);
        }
      }
      module.node_inputs(consumer_id)[input_idx] = input;
    }
  }
}

void TigTransformer::split_outputs(Tig::Module &module, PortMaps &port_maps) {
  using NodeKind = Tig::Module::NodeKind;
  const size_t node_count = module.size();
  for (NodeId node_id = 0; node_id < node_count; ++node_id) {
    const bool is_op = module.node_kind(node_id) == NodeKind::kOp;
    if (has_fixed_outputs(module.node_kind(node_id))) {
      port_maps[node_id].resize(module.node_output_count(node_id));
      for (PortIndex port = 0; port < module.node_output_count(node_id); ++port) {
        if (module.node_output_outer_dim(node_id, port) == 1) {
          port_maps[node_id][port] = {port};
        }
      }
      continue;
    }

    const bool has_combs = !module.node_output_combs(node_id).empty();
    if (is_op) {
      assert(module.node_output_count(node_id) == module.node_output_expr_ids(node_id).size());
      assert(module.node_output_count(node_id) == module.node_output_combs(node_id).size());
    } else {
      assert(!has_combs ||
             module.node_output_count(node_id) == module.node_output_combs(node_id).size());
    }

    const PortIndex old_output_count = module.node_output_count(node_id);
    auto &node_port_maps = port_maps[node_id];
    assert(node_port_maps.empty());
    node_port_maps.resize(old_output_count);
    std::optional<ExprBuilder> builder;
    if (is_op) {
      builder.emplace(module.node_expr_graph(node_id), diagnostics_);
    }
    std::vector<std::string> names;
    std::vector<SignalWidth> outer_dims;
    std::vector<bool> outer_signs;
    std::vector<ExprId> roots;
    std::vector<bool> combs;
    for (PortIndex old_port = 0; old_port < old_output_count; ++old_port) {
      const auto &old_name = module.node_output_name(node_id, old_port);
      const auto old_outer_dim = module.node_output_outer_dim(node_id, old_port);
      const ExprId root = is_op ? module.node_output_expr_ids(node_id)[old_port] : kInvalidExprId;
      const auto shape =
          is_op
              ? (root == kInvalidExprId ? std::nullopt : get_unpacked_shape(module, node_id, root))
              : get_output_shape(module, node_id, old_port);
      auto &mapping = node_port_maps[old_port];
      mapping.resize(shape ? shape->flattened_width() : old_outer_dim);

      if (is_op && root == kInvalidExprId) {
        assert(names.size() < kInvalidPortIndex);
        const PortIndex new_port = static_cast<PortIndex>(names.size());
        names.push_back(old_name);
        outer_dims.push_back(old_outer_dim);
        outer_signs.push_back(module.node_output_outer_sign(node_id, old_port));
        roots.push_back(root);
        combs.push_back(module.node_output_combs(node_id)[old_port]);
        std::ranges::fill(mapping, new_port);
        continue;
      }

      std::vector<SignalWidth> indices(shape ? shape->unpacked_dims.size() : 0, 0);
      SignalWidth packed_bit = 0;
      const ExprId data = is_op && shape ? builder->create_unpacked_flatten(root) : root;
      for (SignalWidth bit = 0; bit < mapping.size(); ++bit) {
        std::string name;
        if (!old_name.empty()) {
          if (module.node_kind(node_id) == NodeKind::kFlatten && old_name.front() != '\\') {
            name = escaped_indexed_name(old_name, bit);
            append_signal(module, name, 1, false);
          } else {
            name = old_name;
          }
          if (module.node_kind(node_id) != NodeKind::kFlatten && shape) {
            for (SignalWidth index : indices) {
              name += "[" + std::to_string(index) + "]";
            }
            name += "[" + std::to_string(packed_bit) + "]";
          } else if (module.node_kind(node_id) != NodeKind::kFlatten && mapping.size() > 1) {
            name += "[" + std::to_string(bit) + "]";
          }
        }

        assert(names.size() < kInvalidPortIndex);
        mapping[bit] = static_cast<PortIndex>(names.size());
        names.push_back(std::move(name));
        outer_dims.push_back(1);
        outer_signs.push_back(false);
        if (is_op) {
          roots.push_back(builder->create_static_range(data, bit, 1, false));
        }
        if (has_combs) {
          combs.push_back(module.node_output_combs(node_id)[old_port]);
        }

        if (!shape || ++packed_bit < shape->inner_dim) {
          continue;
        }
        packed_bit = 0;
        for (size_t dimension = indices.size(); dimension-- > 0;) {
          if (++indices[dimension] < shape->unpacked_dims[dimension]) {
            break;
          }
          indices[dimension] = 0;
        }
      }
    }

    module.set_node_outputs(node_id, std::move(names), std::move(outer_dims),
                            std::move(outer_signs));
    if (is_op) {
      module.node_output_expr_ids(node_id) = std::move(roots);
    }
    if (has_combs) {
      module.node_output_combs(node_id) = std::move(combs);
    }
  }
}

void TigTransformer::blast_op_nodes(Tig::Module &module, const PortMaps &port_maps) {
  const auto mapped_bit = [&](EdgeRef old_input, SignalWidth bit) -> EdgeRef {
    assert(old_input.node_id != kInvalidNodeId);
    const auto &mapping = port_maps.at(old_input.node_id).at(old_input.port_idx);
    assert(bit < mapping.size());
    assert(mapping[bit] != kInvalidPortIndex);
    return {old_input.node_id, mapping[bit]};
  };

  for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
    if (module.node_kind(node_id) != Tig::Module::NodeKind::kOp) {
      continue;
    }
    assert(is_expr_graph_topological(module, node_id));
    std::vector<EdgeRef> old_inputs = std::move(module.node_inputs(node_id));
    assert(old_inputs.size() == module.node_input_expr_ids(node_id).size());
    const std::vector<ExprId> old_input_expr_ids = module.node_input_expr_ids(node_id);
    std::vector<std::vector<ExprId>> blasted_ids(module.node_expr_graph(node_id).size());
    std::vector<EdgeRef> inputs;
    std::vector<ExprId> input_expr_ids;
    for (size_t input = 0; input < old_inputs.size(); ++input) {
      const EdgeRef old_input = old_inputs[input];
      assert(old_input.node_id != kInvalidNodeId);
      const auto &mapping = port_maps.at(old_input.node_id).at(old_input.port_idx);
      const ExprId old_input_expr_id = old_input_expr_ids[input];
      assert(old_input_expr_id < blasted_ids.size());
      const bool split =
          get_unpacked_shape(module, node_id, old_input_expr_id) || mapping.size() > 1;
      if (!split) {
        assert(!mapping.empty());
        assert(mapping.front() != kInvalidPortIndex);
        inputs.push_back({old_input.node_id, mapping.front()});
        input_expr_ids.push_back(old_input_expr_id);
        blasted_ids[old_input_expr_id].push_back(old_input_expr_id);
        continue;
      }
      auto &bits = blasted_ids[old_input_expr_id];
      bits.reserve(mapping.size());
      for (SignalWidth bit = 0; bit < mapping.size(); ++bit) {
        const EdgeRef source = mapped_bit(old_input, bit);
        const ExprId input_id = module.node_expr_graph(node_id).append(ExprGraph::Op::kInput, 1);
        inputs.push_back(source);
        input_expr_ids.push_back(input_id);
        bits.push_back(input_id);
      }
    }
    module.node_inputs(node_id) = std::move(inputs);
    module.node_input_expr_ids(node_id) = std::move(input_expr_ids);
    blast_expr_graph(module, node_id, std::move(blasted_ids));
    clean_op_node(module, node_id);
  }
}

void TigTransformer::blast_non_op_nodes(Tig::Module &module, const PortMaps &port_maps) {
  using NodeKind = Tig::Module::NodeKind;
  const auto append_bits = [&](std::vector<EdgeRef> &inputs, EdgeRef input) {
    assert(input.node_id != kInvalidNodeId);
    const auto &mapping = port_maps.at(input.node_id).at(input.port_idx);
    for (PortIndex port : mapping) {
      assert(port != kInvalidPortIndex);
      inputs.push_back({input.node_id, port});
    }
  };

  for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
    size_t data_input_count = 0;
    switch (module.node_kind(node_id)) {
    case NodeKind::kMultiDriver:
    case NodeKind::kEdgeMultiDriver:
    case NodeKind::kJoin:
      data_input_count = module.node_inputs(node_id).size();
      break;
    case NodeKind::kFf: {
      assert(module.node_clock_kind(node_id) != EdgeKind::kNone);
      const size_t control_input_count = module.node_reset_kind(node_id) == EdgeKind::kNone ? 1 : 2;
      assert(module.node_inputs(node_id).size() > control_input_count);
      data_input_count = module.node_inputs(node_id).size() - control_input_count;
      break;
    }
    case NodeKind::kFold:
      assert(module.node_output_count(node_id) == 1);
      assert(!module.node_inputs(node_id).empty());
      data_input_count = module.node_inputs(node_id).size();
      break;
    default:
      continue;
    }

    std::vector<EdgeRef> old_inputs = std::move(module.node_inputs(node_id));
    module.node_inputs(node_id).clear();
    for (size_t input = 0; input < data_input_count; ++input) {
      append_bits(module.node_inputs(node_id), old_inputs[input]);
    }
    module.node_inputs(node_id).insert(module.node_inputs(node_id).end(),
                                       old_inputs.begin() + data_input_count, old_inputs.end());
  }
}

void TigTransformer::remove_buffers(Tig::Module &module) {
  using NodeKind = Tig::Module::NodeKind;
  const size_t node_count = module.size();
  Replacements replacements;
  for (NodeId node_id = 0; node_id < node_count; ++node_id) {
    if (module.node_kind(node_id) != NodeKind::kOp) {
      continue;
    }
    for (PortIndex port = 0; port < module.node_output_count(node_id); ++port) {
      const ExprId root = module.node_output_expr_ids(node_id)[port];
      if (root == kInvalidExprId ||
          module.node_expr_graph(node_id).op(root) != ExprGraph::Op::kInput) {
        continue;
      }
      const auto input = std::ranges::find(module.node_input_expr_ids(node_id), root);
      assert(input != module.node_input_expr_ids(node_id).end());
      EdgeRef source = module.node_inputs(
          node_id)[static_cast<size_t>(input - module.node_input_expr_ids(node_id).begin())];
      assert(source.node_id != kInvalidNodeId);
      while (true) {
        const auto replacement = replacements.find({source.node_id, source.port_idx});
        if (replacement == replacements.end()) {
          break;
        }
        source = replacement->second;
      }
      replacements.emplace(std::pair{node_id, port}, source);
    }
  }
  for (const auto &replacement : replacements) {
    const auto [node_id, port] = replacement.first;
    module.node_output_name(node_id, port).clear();
    module.node_output_expr_ids(node_id)[port] = kInvalidExprId;
    module.node_output_combs(node_id)[port] = false;
  }
  apply_replacements(module, replacements);
}

void TigTransformer::apply_replacements(Tig::Module &module, const Replacements &replacements) {
  for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
    for (EdgeRef &input : module.node_inputs(node_id)) {
      if (input.node_id == kInvalidNodeId) {
        continue;
      }
      const auto replacement = replacements.find({input.node_id, input.port_idx});
      if (replacement != replacements.end()) {
        input = replacement->second;
      }
    }
  }
}

bool TigTransformer::collect_loop_terminals(Tig::Module &module, std::span<EdgeRef> input,
                                            std::vector<std::span<EdgeRef>> &terminals) {
  using NodeKind = Tig::Module::NodeKind;
  assert(!input.empty());
  const auto source_ref = std::ranges::find_if(
      input, [](const auto &input_ref) { return input_ref.node_id != kInvalidNodeId; });
  if (source_ref == input.end()) {
    return true;
  }
  const NodeId source_id = source_ref->node_id;
  assert(source_id < module.size());
  switch (module.node_kind(source_id)) {
  case NodeKind::kJoin: {
    assert(input.size() == module.node_output_count(source_id));
    assert(module.node_inputs(source_id).size() % module.node_output_count(source_id) == 0);
    const size_t driver_count =
        module.node_inputs(source_id).size() / module.node_output_count(source_id);
    for (size_t driver = 0; driver < driver_count; ++driver) {
      std::span ff_refs(module.node_inputs(source_id).data() +
                            driver * module.node_output_count(source_id),
                        module.node_output_count(source_id));
      if (!collect_loop_terminals(module, ff_refs, terminals)) {
        return false;
      }
    }
    return true;
  }
  case NodeKind::kFf: {
    assert(input.size() == module.node_output_count(source_id));
    assert(module.node_inputs(source_id).size() >= module.node_output_count(source_id));
    std::span data_refs(module.node_inputs(source_id).data(), module.node_output_count(source_id));
    const auto edge_ref = std::ranges::find_if(
        data_refs, [](const auto &data_ref) { return data_ref.node_id != kInvalidNodeId; });
    if (edge_ref != data_refs.end() &&
        module.node_kind(edge_ref->node_id) == NodeKind::kEdgeMultiDriver) {
      return collect_loop_terminals(module, data_refs, terminals);
    }
    terminals.push_back(data_refs);
    return true;
  }
  case NodeKind::kEdgeMultiDriver: {
    assert(input.size() == module.node_output_count(source_id));
    assert(module.node_inputs(source_id).size() % module.node_output_count(source_id) == 0);
    const size_t driver_count =
        module.node_inputs(source_id).size() / module.node_output_count(source_id);
    for (size_t driver = 0; driver < driver_count; ++driver) {
      terminals.emplace_back(module.node_inputs(source_id).data() +
                                 driver * module.node_output_count(source_id),
                             module.node_output_count(source_id));
    }
    return true;
  }
  case NodeKind::kFlatten:
  case NodeKind::kFold:
  case NodeKind::kMemory:
  case NodeKind::kMemoryRead:
  case NodeKind::kMemoryWrite:
    return false;
  default:
    terminals.push_back(input);
    return true;
  }
}

void TigTransformer::remove_feedback(Tig::Module &module, NodeId node_id, size_t driver_count) {
  assert(module.node_output_count(node_id) != 0);
  assert(driver_count * module.node_output_count(node_id) <= module.node_inputs(node_id).size());
  std::vector<std::span<EdgeRef>> terminals;
  for (size_t driver = 0; driver < driver_count; ++driver) {
    std::span input(module.node_inputs(node_id).data() + driver * module.node_output_count(node_id),
                    module.node_output_count(node_id));
    if (!collect_loop_terminals(module, input, terminals)) {
      return;
    }
  }
  for (auto terminal : terminals) {
    assert(terminal.size() == module.node_output_count(node_id));
    for (PortIndex port = 0; port < module.node_output_count(node_id); ++port) {
      if (terminal[port].node_id == node_id && terminal[port].port_idx == port) {
        terminal[port].node_id = kInvalidNodeId;
      }
    }
  }
}

std::vector<EdgeRef> TigTransformer::select_driver_inputs(Tig::Module &module, NodeId node_id,
                                                          size_t driver_count) {
  assert(module.node_output_count(node_id) != 0);
  assert(driver_count * module.node_output_count(node_id) <= module.node_inputs(node_id).size());
  std::vector<std::span<EdgeRef>> inputs;
  std::vector<std::span<EdgeRef>> terminals;
  if (module.node_kind(node_id) == Tig::Module::NodeKind::kFf) {
    std::span data_refs(module.node_inputs(node_id).data(), module.node_output_count(node_id));
    if (!collect_loop_terminals(module, data_refs, terminals)) {
      return {};
    }
    inputs = terminals;
  } else {
    inputs.reserve(driver_count);
    terminals.reserve(driver_count);
    for (size_t driver = 0; driver < driver_count; ++driver) {
      std::span input(module.node_inputs(node_id).data() +
                          driver * module.node_output_count(node_id),
                      module.node_output_count(node_id));
      std::vector<std::span<EdgeRef>> collected;
      if (!collect_loop_terminals(module, input, collected)) {
        return {};
      }
      if (collected.empty()) {
        continue;
      }
      assert(collected.size() == 1);
      inputs.push_back(input);
      terminals.push_back(collected.front());
    }
  }
  assert(inputs.size() == terminals.size());
  std::vector<EdgeRef> selected(module.node_output_count(node_id));
  std::vector<size_t> selected_counts(module.node_output_count(node_id), 0);
  for (size_t driver = 0; driver < inputs.size(); ++driver) {
    const auto input = inputs[driver];
    const auto terminal = terminals[driver];
    for (PortIndex port = 0; port < module.node_output_count(node_id); ++port) {
      if (terminal[port].node_id == kInvalidNodeId) {
        input[port].node_id = kInvalidNodeId;
        continue;
      }
      if (selected_counts[port]++ == 0) {
        selected[port] = input[port];
      } else {
        terminal[port].node_id = kInvalidNodeId;
      }
      input[port].node_id = kInvalidNodeId;
    }
  }
  for (PortIndex port = 0; port < module.node_output_count(node_id); ++port) {
    if (selected_counts[port] > 1) {
      diagnostics_.warning(DiagnosticId::kTransformMultipleDriversResolved,
                           module.node_output_name(node_id, port));
    }
  }
  return selected;
}

void TigTransformer::resolve_multiple_drivers(Tig::Module &module) {
  using NodeKind = Tig::Module::NodeKind;
  const size_t node_count = module.size();

  const auto transfer_output_names = [&](const Replacements &replacements) {
    for (const auto &[source, replacement] : replacements) {
      auto &source_name = module.node_output_name(source.first, source.second);
      if (source_name.empty()) {
        continue;
      }
      if (replacement.node_id != kInvalidNodeId) {
        module.node_output_name(replacement.node_id, replacement.port_idx) = source_name;
      }
      source_name.clear();
    }
  };

  for (NodeId node_id = 0; node_id < node_count; ++node_id) {
    if (module.node_kind(node_id) != NodeKind::kMultiDriver) {
      continue;
    }
    assert(module.node_inputs(node_id).size() % module.node_output_count(node_id) == 0);
    remove_feedback(module, node_id,
                    module.node_inputs(node_id).size() / module.node_output_count(node_id));
  }

  for (NodeId node_id = 0; node_id < node_count; ++node_id) {
    if (module.node_kind(node_id) != NodeKind::kJoin) {
      continue;
    }
    assert(module.node_inputs(node_id).size() % module.node_output_count(node_id) == 0);
    remove_feedback(module, node_id,
                    module.node_inputs(node_id).size() / module.node_output_count(node_id));
  }

  for (NodeId ff_id = 0; ff_id < node_count; ++ff_id) {
    if (module.node_kind(ff_id) != NodeKind::kFf) {
      continue;
    }
    remove_feedback(module, ff_id, 1);
  }

  for (NodeId ff_id = 0; ff_id < node_count; ++ff_id) {
    if (module.node_kind(ff_id) != NodeKind::kFf) {
      continue;
    }
    const auto selected = select_driver_inputs(module, ff_id, 1);
    if (selected.empty()) {
      continue;
    }
    std::span data_refs(module.node_inputs(ff_id).data(), module.node_output_count(ff_id));
    assert(selected.size() == data_refs.size());
    std::ranges::copy(selected, data_refs.begin());
  }

  Replacements join_replacements;
  for (NodeId join_id = 0; join_id < node_count; ++join_id) {
    if (module.node_kind(join_id) != NodeKind::kJoin) {
      continue;
    }
    const auto selected = select_driver_inputs(
        module, join_id, module.node_inputs(join_id).size() / module.node_output_count(join_id));
    if (selected.empty()) {
      continue;
    }
    for (PortIndex port = 0; port < selected.size(); ++port) {
      join_replacements.emplace(std::pair{join_id, port}, selected[port]);
    }
  }
  transfer_output_names(join_replacements);
  apply_replacements(module, join_replacements);

  Replacements multi_driver_replacements;
  for (NodeId multi_driver_id = static_cast<NodeId>(node_count); multi_driver_id-- > 0;) {
    if (module.node_kind(multi_driver_id) != NodeKind::kMultiDriver) {
      continue;
    }
    auto selected = select_driver_inputs(module, multi_driver_id,
                                         module.node_inputs(multi_driver_id).size() /
                                             module.node_output_count(multi_driver_id));
    if (selected.empty()) {
      continue;
    }
    for (PortIndex port = 0; port < selected.size(); ++port) {
      EdgeRef &selected_ref = selected[port];
      if (selected_ref.node_id != kInvalidNodeId) {
        const auto replacement =
            multi_driver_replacements.find({selected_ref.node_id, selected_ref.port_idx});
        if (replacement != multi_driver_replacements.end()) {
          selected_ref = replacement->second;
        }
      }
      multi_driver_replacements.emplace(std::pair{multi_driver_id, port}, selected_ref);
    }
  }
  transfer_output_names(multi_driver_replacements);
  apply_replacements(module, multi_driver_replacements);
}

void TigTransformer::blast() {
  for (auto &module : design_.modules) {
    insert_fixed_interfaces(module);

    PortMaps port_maps(module.size());
    split_outputs(module, port_maps);
    blast_op_nodes(module, port_maps);
    blast_non_op_nodes(module, port_maps);
    remove_buffers(module);
    resolve_multiple_drivers(module);
  }
}

} // namespace abys::ir
