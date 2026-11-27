#include "abys/ir/tig_transformer.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <optional>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "abys/ir/expr_builder.h"

namespace abys::ir {

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

} // namespace abys::ir
