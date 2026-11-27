#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "abys/ir/type.h"

namespace abys::ir {

class ExprGraph {
public:
  // TODO: define backend synthesis constraints for nonconstant division, modulo, and power.
  enum class Op : uint8_t {
    kUnknown,
    // Values.
    kConst,
    kInput,
    // Subroutine calls.
    kCall,
    // Logical operations.
    kLogicalNot,
    // Reductions.
    kAndReduce,
    kOrReduce,
    kXorReduce,
    // Bitwise operations.
    kBitwiseNot,
    kAnd,
    kOr,
    kXor,
    // Arithmetic and shifts.
    kUnaryMinus,
    kAdd,
    kSub,
    kMul,
    kDiv,
    kMod,
    kPow,
    kShl,
    kShr,
    kAshr,
    // Comparisons.
    kEq,
    kLt,
    kLe,
    // Selection.
    kMux,
    kUnpackedMux,
    kPmux,
    kList,
    kCase,
    kUnpackedCase,
    // Conversion.
    kConvert,
    // Concatenation and gather.
    kConcat,
    kUnpackedConcat,
    kGather,
    // Reshaping and reversal.
    kUnpackedFlatten,
    kUnpackedFold,
    kReverse,
    // Range and element access.
    kRange,
    kUnpackedRange,
    kUnpackedSelect,
    // Assignment and sequencing.
    kSequence,
    kMaskedAssign,
    kUnpackedAssign,
    kUnpackedRangeAssign,
    // Event controls.
    kBothEdge,
    // Operation count.
    kCount,
  };

  // IDs for built-in constants.
  static constexpr ExprId constant_zero = 0;
  static constexpr ExprId constant_one = 1;

  // Creates the built-in constants.
  ExprGraph();

  // Number of expressions, including the built-in constants.
  ExprId size() const;

  // Fields shared by every expression.
  Op op(ExprId id) const;
  SignalWidth outer_dim(ExprId id) const;
  bool outer_sign(ExprId id) const;
  const std::vector<ExprId> &operands(ExprId id) const;

  // Expression insertion and rewrites.
  ExprId append(Op op, SignalWidth outer_dim = 0, bool outer_sign = false, std::vector<ExprId> operands = {});
  ExprId append_copy(const ExprGraph &source, ExprId id, std::vector<ExprId> operands);
  void replace_with(ExprId id, ExprId source);
  void set_op(ExprId id, Op op);
  std::span<ExprId> mutable_operands(ExprId id);
  void set_operands(ExprId id, std::vector<ExprId> operands);

  // Shared unpacked shape metadata.
  const UnpackedShape *unpacked_shape(ExprId id) const;
  void set_unpacked_shape(ExprId id, UnpackedShape shape);
  void copy_unpacked_shape(ExprId id, ExprId source);

  // kConst payload.
  const std::string &constant_value(ExprId id) const;
  void set_constant_value(ExprId id, std::string value);

  // kCall payload.
  SubrId call_target(ExprId id) const;
  const std::string &call_name(ExprId id) const;
  void set_call(ExprId id, SubrId target, std::string name);

  // Affine index parameters for kRange and kMaskedAssign.
  BitIndex affine_offset(ExprId id) const;
  const std::vector<BitIndex> &affine_strides(ExprId id) const;
  void set_affine_index(ExprId id, BitIndex offset, std::vector<BitIndex> strides);

  // Slice width for kMaskedAssign and kUnpackedRangeAssign.
  SignalWidth slice_width(ExprId id) const;
  void set_slice_width(ExprId id, SignalWidth width);

  // Static base for kUnpackedRangeAssign.
  std::optional<BitIndex> unpacked_range_assign_base(ExprId id) const;
  void set_unpacked_range_assign_base(ExprId id, BitIndex base);

private:
  using OpIndex = uint32_t;
  using UnpackedShapeId = uint32_t;
  static constexpr auto kInvalidShape = std::numeric_limits<UnpackedShapeId>::max();

  // Copies the source operator payload into the destination slot.
  void copy_operator_payload(ExprId id, const ExprGraph &source, ExprId old_id);

  // Next payload index for each operator, indexed by Op.
  std::array<OpIndex, static_cast<size_t>(Op::kCount)> next_op_indices_;

  // Indexed by ExprId.
  std::vector<Op> operators_;
  std::vector<OpIndex> op_indices_;
  std::vector<SignalWidth> outer_dims_;
  std::vector<bool> outer_signs_;
  std::vector<std::vector<ExprId>> operands_;
  std::vector<UnpackedShapeId> unpacked_shape_ids_;

  // Indexed by UnpackedShapeId.
  std::vector<UnpackedShape> unpacked_shapes_;

  // kConst payload, indexed by OpIndex.
  std::vector<std::string> constant_values_;

  // kCall payload, indexed by OpIndex.
  std::vector<SubrId> call_targets_;
  std::vector<std::string> call_names_;

  // kRange payload, indexed by OpIndex.
  std::vector<BitIndex> range_offsets_;
  std::vector<std::vector<BitIndex>> range_strides_;

  // kMaskedAssign payload, indexed by OpIndex.
  std::vector<BitIndex> masked_assign_offsets_;
  std::vector<std::vector<BitIndex>> masked_assign_strides_;
  std::vector<SignalWidth> masked_assign_slice_widths_;

  // kUnpackedRangeAssign payload, indexed by OpIndex.
  std::vector<std::optional<BitIndex>> unpacked_range_assign_bases_;
  std::vector<SignalWidth> unpacked_range_assign_slice_widths_;
};

} // namespace abys::ir
