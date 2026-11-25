#include "abys/ir/expr.h"

#include <cassert>
#include <utility>

namespace abys::ir {

ExprGraph::ExprGraph()
    : next_op_indices_{}, operators_{Op::kConst, Op::kConst}, op_indices_{0, 1}, widths_{1, 1},
      signs_{false, false}, operands_(2), unpacked_shape_ids_{kInvalidShape, kInvalidShape},
      constant_values_{"1'b0", "1'b1"} {
  next_op_indices_[static_cast<size_t>(Op::kConst)] = 2;
}

ExprId ExprGraph::size() const {
  return static_cast<ExprId>(operators_.size());
}

ExprGraph::Op ExprGraph::op(ExprId id) const {
  return operators_[id];
}

SignalWidth ExprGraph::width(ExprId id) const {
  return widths_[id];
}

bool ExprGraph::sign(ExprId id) const {
  return signs_[id];
}

const std::vector<ExprId> &ExprGraph::operands(ExprId id) const {
  return operands_[id];
}

ExprId ExprGraph::append(ExprGraph::Op op, SignalWidth width, bool sign,
                         std::vector<ExprId> inputs) {
  assert(operators_.size() < kInvalidExprId);
  const ExprId id = static_cast<ExprId>(operators_.size());
  operators_.push_back(ExprGraph::Op::kCount);
  op_indices_.push_back(0);
  widths_.push_back(width);
  signs_.push_back(sign);
  operands_.push_back(std::move(inputs));
  unpacked_shape_ids_.push_back(ExprGraph::kInvalidShape);
  set_op(id, op);
  return id;
}

ExprId ExprGraph::append_copy(const ExprGraph &source, ExprId old_id, std::vector<ExprId> inputs) {
  const auto kind = source.operators_[old_id];
  const auto id = append(kind, source.widths_[old_id], source.signs_[old_id], std::move(inputs));
  if (this == &source) {
    copy_unpacked_shape(id, old_id);
  } else if (const auto *shape = source.unpacked_shape(old_id)) {
    set_unpacked_shape(id, *shape);
  }
  copy_operator_payload(id, source, old_id);
  return id;
}

void ExprGraph::replace_with(ExprId id, ExprId source) {
  [[maybe_unused]] const auto *destination_shape = unpacked_shape(id);
  [[maybe_unused]] const auto *source_shape = unpacked_shape(source);
  assert(destination_shape == source_shape ||
         (destination_shape && source_shape &&
          destination_shape->unpacked_dims == source_shape->unpacked_dims &&
          destination_shape->width == source_shape->width &&
          destination_shape->sign == source_shape->sign));
  set_op(id, operators_[source]);
  widths_[id] = widths_[source];
  signs_[id] = signs_[source];
  operands_[id] = operands_[source];
  copy_operator_payload(id, *this, source);
}

void ExprGraph::set_op(ExprId id, ExprGraph::Op op) {
  assert(id > ExprGraph::constant_one && op != ExprGraph::Op::kCount);
  if (operators_[id] == op) {
    return;
  }
  operators_[id] = op;
  op_indices_[id] = next_op_indices_[static_cast<size_t>(op)]++;
  switch (op) {
  case ExprGraph::Op::kConst:
    constant_values_.emplace_back();
    break;
  case ExprGraph::Op::kCall:
    call_targets_.push_back(kInvalidSubrId);
    call_names_.emplace_back();
    break;
  case ExprGraph::Op::kRange:
    range_offsets_.push_back(0);
    range_strides_.emplace_back();
    break;
  case ExprGraph::Op::kMaskedAssign:
    masked_assign_offsets_.push_back(0);
    masked_assign_strides_.emplace_back();
    masked_assign_slice_widths_.push_back(0);
    break;
  case ExprGraph::Op::kUnpackedRangeAssign:
    unpacked_range_assign_slice_widths_.push_back(0);
    unpacked_range_assign_bases_.emplace_back();
    break;
  default:
    break;
  }
}

std::span<ExprId> ExprGraph::mutable_operands(ExprId id) {
  return operands_[id];
}

void ExprGraph::set_operands(ExprId id, std::vector<ExprId> operands) {
  operands_[id] = std::move(operands);
}

const UnpackedShape *ExprGraph::unpacked_shape(ExprId id) const {
  if (id == kInvalidExprId) {
    return nullptr;
  }
  const auto shape_id = unpacked_shape_ids_[id];
  return shape_id == ExprGraph::kInvalidShape ? nullptr : &unpacked_shapes_[shape_id];
}

void ExprGraph::set_unpacked_shape(ExprId id, UnpackedShape shape) {
  assert(unpacked_shapes_.size() < ExprGraph::kInvalidShape);
  const auto shape_id = static_cast<ExprGraph::UnpackedShapeId>(unpacked_shapes_.size());
  unpacked_shapes_.push_back(std::move(shape));
  unpacked_shape_ids_[id] = shape_id;
}

void ExprGraph::copy_unpacked_shape(ExprId id, ExprId source) {
  unpacked_shape_ids_[id] = unpacked_shape_ids_[source];
}

const std::string &ExprGraph::constant_value(ExprId id) const {
  assert(op(id) == Op::kConst);
  return constant_values_[op_indices_[id]];
}

void ExprGraph::set_constant_value(ExprId id, std::string value) {
  assert(id > constant_one && op(id) == Op::kConst);
  constant_values_[op_indices_[id]] = std::move(value);
}

SubrId ExprGraph::call_target(ExprId id) const {
  assert(op(id) == Op::kCall);
  return call_targets_[op_indices_[id]];
}

const std::string &ExprGraph::call_name(ExprId id) const {
  assert(op(id) == Op::kCall);
  return call_names_[op_indices_[id]];
}

void ExprGraph::set_call(ExprId id, SubrId target, std::string name) {
  assert(op(id) == Op::kCall);
  const auto index = op_indices_[id];
  call_targets_[index] = target;
  call_names_[index] = std::move(name);
}

BitIndex ExprGraph::affine_offset(ExprId id) const {
  assert(operators_[id] == ExprGraph::Op::kRange || operators_[id] == ExprGraph::Op::kMaskedAssign);
  return (operators_[id] == ExprGraph::Op::kRange ? range_offsets_
                                                  : masked_assign_offsets_)[op_indices_[id]];
}

const std::vector<BitIndex> &ExprGraph::affine_strides(ExprId id) const {
  assert(operators_[id] == ExprGraph::Op::kRange || operators_[id] == ExprGraph::Op::kMaskedAssign);
  return (operators_[id] == ExprGraph::Op::kRange ? range_strides_
                                                  : masked_assign_strides_)[op_indices_[id]];
}

void ExprGraph::set_affine_index(ExprId id, BitIndex offset, std::vector<BitIndex> strides) {
  const auto index = op_indices_[id];
  if (operators_[id] == ExprGraph::Op::kRange) {
    range_offsets_[index] = offset;
    range_strides_[index] = std::move(strides);
  } else {
    assert(operators_[id] == ExprGraph::Op::kMaskedAssign);
    masked_assign_offsets_[index] = offset;
    masked_assign_strides_[index] = std::move(strides);
  }
}

SignalWidth ExprGraph::slice_width(ExprId id) const {
  assert(operators_[id] == ExprGraph::Op::kMaskedAssign ||
         operators_[id] == ExprGraph::Op::kUnpackedRangeAssign);
  return (operators_[id] == ExprGraph::Op::kMaskedAssign
              ? masked_assign_slice_widths_
              : unpacked_range_assign_slice_widths_)[op_indices_[id]];
}

void ExprGraph::set_slice_width(ExprId id, SignalWidth width) {
  assert(width > 0 && width <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
  assert(operators_[id] == ExprGraph::Op::kMaskedAssign ||
         operators_[id] == ExprGraph::Op::kUnpackedRangeAssign);
  (operators_[id] == ExprGraph::Op::kMaskedAssign
       ? masked_assign_slice_widths_
       : unpacked_range_assign_slice_widths_)[op_indices_[id]] = width;
}

std::optional<BitIndex> ExprGraph::unpacked_range_assign_base(ExprId id) const {
  assert(op(id) == Op::kUnpackedRangeAssign);
  return unpacked_range_assign_bases_[op_indices_[id]];
}

void ExprGraph::set_unpacked_range_assign_base(ExprId id, BitIndex base) {
  assert(op(id) == Op::kUnpackedRangeAssign && base >= 0 && operands(id).size() == 1);
  unpacked_range_assign_bases_[op_indices_[id]] = base;
}

void ExprGraph::copy_operator_payload(ExprId id, const ExprGraph &source, ExprId old_id) {
  const ExprGraph::OpIndex index = op_indices_[id];
  const ExprGraph::OpIndex source_index = source.op_indices_[old_id];
  switch (source.op(old_id)) {
  case ExprGraph::Op::kConst:
    constant_values_[index] = source.constant_values_[source_index];
    break;
  case ExprGraph::Op::kCall:
    call_targets_[index] = source.call_targets_[source_index];
    call_names_[index] = source.call_names_[source_index];
    break;
  case ExprGraph::Op::kRange:
    set_affine_index(id, source.affine_offset(old_id), source.affine_strides(old_id));
    break;
  case ExprGraph::Op::kMaskedAssign:
    set_affine_index(id, source.affine_offset(old_id), source.affine_strides(old_id));
    set_slice_width(id, source.slice_width(old_id));
    break;
  case ExprGraph::Op::kUnpackedRangeAssign:
    set_slice_width(id, source.slice_width(old_id));
    unpacked_range_assign_bases_[index] = source.unpacked_range_assign_bases_[source_index];
    break;
  default:
    break;
  }
}

} // namespace abys::ir
