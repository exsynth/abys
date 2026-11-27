#include <algorithm>
#include <bit>
#include <cassert>
#include <cmath>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>

#include "abys/ir/expr_builder.h"

namespace abys::ir {

namespace {

using UnsignedBitIndex = std::make_unsigned_t<BitIndex>;

BitIndex apply_integral_conversion(BitIndex value, SignalWidth width, bool sign) {
  constexpr SignalWidth kBitIndexWidth = std::numeric_limits<UnsignedBitIndex>::digits;
  assert(width > 0);
  if (width >= kBitIndexWidth) {
    return value;
  }
  const UnsignedBitIndex mask = (UnsignedBitIndex{1} << width) - 1;
  UnsignedBitIndex converted = static_cast<UnsignedBitIndex>(value) & mask;
  if (sign && (converted & (UnsignedBitIndex{1} << (width - 1)))) {
    converted |= ~mask;
  }
  return std::bit_cast<BitIndex>(converted);
}

std::optional<BitIndex> parse_integral_constant(std::string_view value) {
  bool negative = false;
  if (!value.empty() && value.front() == '-') {
    negative = true;
    value.remove_prefix(1);
  }

  size_t pos = value.find('\'');
  if (pos == std::string_view::npos) {
    return std::nullopt;
  }
  ++pos;
  if (pos < value.size() && value[pos] == 's') {
    ++pos;
  }
  if (pos >= value.size()) {
    return std::nullopt;
  }
  unsigned base;
  switch (value[pos]) {
  case 'b':
    base = 2;
    break;
  case 'o':
    base = 8;
    break;
  case 'd':
    base = 10;
    break;
  case 'h':
    base = 16;
    break;
  default:
    return std::nullopt;
  }
  ++pos;
  if (pos == value.size()) {
    return std::nullopt;
  }

  const UnsignedBitIndex limit =
      negative ? static_cast<UnsignedBitIndex>(std::numeric_limits<BitIndex>::max()) + 1
               : static_cast<UnsignedBitIndex>(std::numeric_limits<BitIndex>::max());
  UnsignedBitIndex magnitude = 0;
  for (; pos < value.size(); ++pos) {
    const char character = value[pos];
    unsigned digit;
    if (character >= '0' && character <= '9') {
      digit = static_cast<unsigned>(character - '0');
    } else if (character >= 'a' && character <= 'f') {
      digit = static_cast<unsigned>(character - 'a') + 10;
    } else if (character >= 'A' && character <= 'F') {
      digit = static_cast<unsigned>(character - 'A') + 10;
    } else {
      return std::nullopt;
    }
    if (digit >= base || magnitude > (limit - digit) / base) {
      return std::nullopt;
    }
    magnitude = magnitude * base + digit;
  }

  if (!negative) {
    return static_cast<BitIndex>(magnitude);
  }
  if (magnitude == static_cast<UnsignedBitIndex>(std::numeric_limits<BitIndex>::max()) + 1) {
    return std::numeric_limits<BitIndex>::min();
  }
  return -static_cast<BitIndex>(magnitude);
}

} // namespace

ExprBuilder::ExprBuilder(ExprGraph &graph, Diagnostics &diagnostics)
    : graph_(graph), diagnostics_(diagnostics) {}

ExprBuilder::ExprBuilder(const ExprBuilder &parent)
    : graph_(parent.graph_), diagnostics_(parent.diagnostics_), name_map_(parent.name_map_),
      input_map_(parent.input_map_) {}

SignalWidth ExprBuilder::get_outer_dim(ExprId id) const {
  return graph_.outer_dim(id);
}
bool ExprBuilder::get_sign(ExprId id) const {
  return graph_.sign(id);
}
bool ExprBuilder::is_sequence(ExprId id) const {
  assert(id != kInvalidExprId);
  return graph_.op(id) == ExprGraph::Op::kSequence;
}

ExprId ExprBuilder::get_constant_zero() {
  return ExprGraph::constant_zero;
}
ExprId ExprBuilder::get_constant_one() {
  return ExprGraph::constant_one;
}

ExprId ExprBuilder::find_or_create_input(std::string name, SignalWidth width, bool sign) {
  const ExprId current = get_current_value(name);
  if (current != kInvalidExprId) {
    return current;
  }
  const auto it = input_map_->find(name);
  if (it != input_map_->end()) {
    return it->second;
  }
  const ExprId id = graph_.append(ExprGraph::Op::kInput, width, sign);
  name_map_[name] = id;
  input_map_->emplace(std::move(name), id);
  return id;
}

ExprId ExprBuilder::find_or_create_unpacked_input(std::string name,
                                                  const std::vector<SignalWidth> &unpacked_dims,
                                                  SignalWidth inner_dim, bool element_sign) {
  assert(!unpacked_dims.empty());
  const ExprId current = get_current_value(name);
  if (current != kInvalidExprId) {
    return current;
  }
  const auto input = input_map_->find(name);
  if (input != input_map_->end()) {
    return input->second;
  }
  const ExprId id = find_or_create_input(std::move(name), unpacked_dims.front(), false);
  graph_.set_unpacked_shape(id, {unpacked_dims, inner_dim, element_sign});
  return id;
}

ExprId ExprBuilder::find_or_create_const(std::string value, SignalWidth width, bool sign) {
  if (width == 1 && !sign) {
    if (value == "1'b0") {
      return ExprGraph::constant_zero;
    }
    if (value == "1'b1") {
      return ExprGraph::constant_one;
    }
  }
  const ExprId id = graph_.append(ExprGraph::Op::kConst, width, sign);
  graph_.set_constant_value(id, std::move(value));
  return id;
}

ExprId ExprBuilder::find_or_create_const(BitIndex index, SignalWidth width, bool sign) {
  assert(width > 0);
  return find_or_create_const(format_integral_constant(index, width, sign), width, sign);
}

SignalWidth ExprBuilder::minimum_unsigned_width(BitIndex index) {
  assert(index >= 0);
  SignalWidth width = 1;
  while (index > 1) {
    ++width;
    index >>= 1;
  }
  return width;
}

SignalWidth ExprBuilder::minimum_signed_width(BitIndex index) {
  UnsignedBitIndex magnitude;
  if (index < 0) {
    magnitude = static_cast<UnsignedBitIndex>(-(index + 1)) + 1;
  } else {
    magnitude = static_cast<UnsignedBitIndex>(index);
  }
  SignalWidth width = 1;
  while (magnitude != 0) {
    ++width;
    magnitude >>= 1;
  }
  return width;
}

ExprId ExprBuilder::create_unary_reduce(ExprGraph::Op op, ExprId operand) {
  return graph_.append(op, 1, false, {operand});
}
ExprId ExprBuilder::create_logical_not(ExprId operand) {
  return create_unary_reduce(ExprGraph::Op::kLogicalNot, operand);
}
ExprId ExprBuilder::create_and_reduce(ExprId operand) {
  return create_unary_reduce(ExprGraph::Op::kAndReduce, operand);
}
ExprId ExprBuilder::create_or_reduce(ExprId operand) {
  return create_unary_reduce(ExprGraph::Op::kOrReduce, operand);
}
ExprId ExprBuilder::create_xor_reduce(ExprId operand) {
  return create_unary_reduce(ExprGraph::Op::kXorReduce, operand);
}

ExprId ExprBuilder::create_unary(ExprGraph::Op op, ExprId operand) {
  return graph_.append(op, graph_.outer_dim(operand), graph_.sign(operand), {operand});
}
ExprId ExprBuilder::create_bitwise_not(ExprId operand) {
  return create_unary(ExprGraph::Op::kBitwiseNot, operand);
}

ExprId ExprBuilder::create_nary(ExprGraph::Op op, std::vector<ExprId> operands) {
  assert(!operands.empty());
  SignalWidth width = 0;
  bool sign = true;
  for (ExprId operand : operands) {
    width = std::max(width, graph_.outer_dim(operand));
    sign &= graph_.sign(operand);
  }
  return graph_.append(op, width, sign, std::move(operands));
}
ExprId ExprBuilder::create_and(std::vector<ExprId> operands) {
  return create_nary(ExprGraph::Op::kAnd, std::move(operands));
}
ExprId ExprBuilder::create_or(std::vector<ExprId> operands) {
  return create_nary(ExprGraph::Op::kOr, std::move(operands));
}
ExprId ExprBuilder::create_xor(std::vector<ExprId> operands) {
  return create_nary(ExprGraph::Op::kXor, std::move(operands));
}

ExprId ExprBuilder::create_unary_plus(ExprId a) {
  return a;
}
ExprId ExprBuilder::create_unary_minus(ExprId a) {
  return create_unary(ExprGraph::Op::kUnaryMinus, a);
}

ExprId ExprBuilder::create_logical_binary(ExprGraph::Op op, ExprId a, ExprId b) {
  const ExprId id = graph_.append(op, 1);
  if (graph_.outer_dim(a) > 1) {
    a = create_or_reduce(a);
  }
  if (graph_.outer_dim(b) > 1) {
    b = create_or_reduce(b);
  }
  graph_.set_operands(id, {a, b});
  return id;
}
ExprId ExprBuilder::create_logical_and(ExprId a, ExprId b) {
  return create_logical_binary(ExprGraph::Op::kAnd, a, b);
}
ExprId ExprBuilder::create_logical_or(ExprId a, ExprId b) {
  return create_logical_binary(ExprGraph::Op::kOr, a, b);
}

ExprId ExprBuilder::create_binary(ExprGraph::Op op, ExprId a, ExprId b) {
  const ExprId id = graph_.append(op, std::max(graph_.outer_dim(a), graph_.outer_dim(b)),
                                  graph_.sign(a) & graph_.sign(b), {a, b});
  return id;
}
ExprId ExprBuilder::create_add(ExprId a, ExprId b) {
  return create_binary(ExprGraph::Op::kAdd, a, b);
}
ExprId ExprBuilder::create_sub(ExprId a, ExprId b) {
  return create_binary(ExprGraph::Op::kSub, a, b);
}
ExprId ExprBuilder::create_mul(ExprId a, ExprId b) {
  return create_binary(ExprGraph::Op::kMul, a, b);
}
ExprId ExprBuilder::create_div(ExprId a, ExprId b) {
  return create_binary(ExprGraph::Op::kDiv, a, b);
}
ExprId ExprBuilder::create_mod(ExprId a, ExprId b) {
  return create_binary(ExprGraph::Op::kMod, a, b);
}
ExprId ExprBuilder::create_pow(ExprId a, ExprId b) {
  return create_binary(ExprGraph::Op::kPow, a, b);
}

ExprId ExprBuilder::create_shift(ExprGraph::Op op, ExprId data, ExprId shamt) {
  const ExprId id = graph_.append(op, graph_.outer_dim(data), graph_.sign(data), {data, shamt});
  return id;
}
ExprId ExprBuilder::create_shl(ExprId data, ExprId shamt) {
  return create_shift(ExprGraph::Op::kShl, data, shamt);
}
ExprId ExprBuilder::create_shr(ExprId data, ExprId shamt) {
  return create_shift(ExprGraph::Op::kShr, data, shamt);
}
ExprId ExprBuilder::create_ashr(ExprId data, ExprId shamt) {
  return create_shift(ExprGraph::Op::kAshr, data, shamt);
}

ExprId ExprBuilder::create_compare(ExprGraph::Op op, ExprId a, ExprId b) {
  const ExprId id = graph_.append(op, 1, false, {a, b});
  return id;
}
ExprId ExprBuilder::create_eq(ExprId a, ExprId b) {
  return create_compare(ExprGraph::Op::kEq, a, b);
}
ExprId ExprBuilder::create_neq(ExprId a, ExprId b) {
  const ExprId id = create_compare(ExprGraph::Op::kEq, a, b);
  return create_logical_not(id);
}
ExprId ExprBuilder::create_lt(ExprId a, ExprId b) {
  return create_compare(ExprGraph::Op::kLt, a, b);
}
ExprId ExprBuilder::create_le(ExprId a, ExprId b) {
  return create_compare(ExprGraph::Op::kLe, a, b);
}
ExprId ExprBuilder::create_gt(ExprId a, ExprId b) {
  return create_lt(b, a);
}
ExprId ExprBuilder::create_ge(ExprId a, ExprId b) {
  return create_le(b, a);
}

ExprId ExprBuilder::create_sequence_branch(ExprId id) {
  if (id == kInvalidExprId || graph_.op(id) != ExprGraph::Op::kSequence) {
    return id;
  }
  assert(!graph_.operands(id).empty());
  graph_.mutable_operands(id).front() = kInvalidExprId;
  return id;
}

ExprId ExprBuilder::create_mux(ExprId cond, ExprId then, ExprId else_id) {
  then = create_sequence_branch(then);
  else_id = create_sequence_branch(else_id);
  const auto *then_shape = graph_.unpacked_shape(then);
  const auto *else_shape = graph_.unpacked_shape(else_id);
  if (then_shape == nullptr && else_shape != nullptr) {
    assert(then == kInvalidExprId);
  }
  if (then_shape != nullptr && else_shape == nullptr) {
    assert(else_id == kInvalidExprId);
  }
  if (then_shape != nullptr && else_shape != nullptr) {
    assert(then_shape->unpacked_dims == else_shape->unpacked_dims);
    assert(then_shape->inner_dim == else_shape->inner_dim);
    assert(then_shape->sign == else_shape->sign);
  }
  const UnpackedShape *unpacked_shape = then_shape != nullptr ? then_shape : else_shape;
  SignalWidth outer_dim;
  bool sign;
  if (then != kInvalidExprId && else_id != kInvalidExprId) {
    outer_dim = std::max(graph_.outer_dim(then), graph_.outer_dim(else_id));
    sign = graph_.sign(then) & graph_.sign(else_id);
  } else if (then != kInvalidExprId) {
    outer_dim = graph_.outer_dim(then);
    sign = graph_.sign(then);
  } else if (else_id != kInvalidExprId) {
    outer_dim = graph_.outer_dim(else_id);
    sign = graph_.sign(else_id);
  } else {
    diagnostics_.error(DiagnosticId::kLoweringUnsupportedExpressionReplacedWithZero,
                       "mux has no valid data operand");
    return ExprGraph::constant_zero;
  }
  const auto op = unpacked_shape ? ExprGraph::Op::kUnpackedMux : ExprGraph::Op::kMux;
  const ExprId id = graph_.append(op, outer_dim, sign, {cond, then, else_id});
  if (unpacked_shape != nullptr) {
    graph_.copy_unpacked_shape(id, then_shape ? then : else_id);
  }
  return id;
}

ExprId ExprBuilder::create_list(std::vector<ExprId> operands) {
  return graph_.append(ExprGraph::Op::kList, 0, false, std::move(operands));
}

ExprId ExprBuilder::create_match(ExprId selector, ExprId case_value) {
  if (graph_.op(case_value) == ExprGraph::Op::kList) {
    std::vector<ExprId> eqs;
    eqs.reserve(graph_.operands(case_value).size());
    for (ExprId list_item : graph_.operands(case_value)) {
      eqs.push_back(create_eq(selector, list_item));
    }
    return create_or(std::move(eqs));
  }
  return create_eq(selector, case_value);
}

ExprId ExprBuilder::create_pmux(std::vector<ExprId> conditions, std::vector<ExprId> data_ids) {
  assert(conditions.size() == data_ids.size() || conditions.size() + 1 == data_ids.size());
  std::vector<ExprId> operands;
  operands.reserve(conditions.size() + data_ids.size());
  bool is_first = true;
  SignalWidth outer_dim = 0;
  bool sign = false;
  for (size_t i = 0; i < data_ids.size(); ++i) {
    if (i < conditions.size()) {
      assert(get_outer_dim(conditions[i]) == 1);
      operands.push_back(conditions[i]);
    }
    operands.push_back(data_ids[i]);
    if (data_ids[i] == kInvalidExprId) {
      continue;
    }
    if (is_first) {
      is_first = false;
      outer_dim = graph_.outer_dim(data_ids[i]);
      sign = graph_.sign(data_ids[i]);
    } else {
      assert(outer_dim == graph_.outer_dim(data_ids[i]));
      assert(sign == graph_.sign(data_ids[i]));
    }
  }
  assert(!is_first);
  const ExprId id = graph_.append(ExprGraph::Op::kPmux, outer_dim, sign, std::move(operands));
  return id;
}

ExprId ExprBuilder::create_case(ExprId selector, std::vector<ExprId> case_values,
                                std::vector<ExprId> data_ids) {
  assert(case_values.size() == data_ids.size() || case_values.size() + 1 == data_ids.size());
  for (ExprId &data_id : data_ids) {
    data_id = create_sequence_branch(data_id);
  }
  const UnpackedShape *unpacked_shape = nullptr;
  ExprId shape_source = kInvalidExprId;
  bool has_packed_data = false;
  for (const ExprId data_id : data_ids) {
    if (data_id == kInvalidExprId) {
      continue;
    }
    const auto *shape = graph_.unpacked_shape(data_id);
    if (shape == nullptr) {
      assert(unpacked_shape == nullptr);
      has_packed_data = true;
      continue;
    }
    assert(!has_packed_data);
    if (unpacked_shape != nullptr) {
      assert(shape->unpacked_dims == unpacked_shape->unpacked_dims);
      assert(shape->inner_dim == unpacked_shape->inner_dim);
      assert(shape->sign == unpacked_shape->sign);
    } else {
      unpacked_shape = shape;
      shape_source = data_id;
    }
  }
  std::vector<ExprId> operands{selector};
  operands.reserve(1 + case_values.size() + data_ids.size());
  SignalWidth outer_dim = 0;
  bool sign = false;
  bool is_first = true;
  for (size_t i = 0; i < data_ids.size(); ++i) {
    if (i < case_values.size()) {
      operands.push_back(case_values[i]);
    }
    operands.push_back(data_ids[i]);
    if (data_ids[i] != kInvalidExprId) {
      if (is_first) {
        is_first = false;
        outer_dim = graph_.outer_dim(data_ids[i]);
        sign = graph_.sign(data_ids[i]);
      } else {
        assert(graph_.outer_dim(data_ids[i]) == outer_dim);
        assert(graph_.sign(data_ids[i]) == sign);
      }
    }
  }
  assert(!is_first);
  const auto op = unpacked_shape ? ExprGraph::Op::kUnpackedCase : ExprGraph::Op::kCase;
  const ExprId id = graph_.append(op, outer_dim, sign, std::move(operands));
  if (unpacked_shape != nullptr) {
    graph_.copy_unpacked_shape(id, shape_source);
  }
  return id;
}

ExprId ExprBuilder::create_convert(ExprId operand, SignalWidth width, bool sign) {
  return graph_.append(ExprGraph::Op::kConvert, width, sign, {operand});
}

ExprId ExprBuilder::create_concat(std::vector<ExprId> operands, bool sign) {
  assert(!operands.empty());
  SignalWidth width = 0;
  for (ExprId operand : operands) {
    width += operand == kInvalidExprId ? 1 : graph_.outer_dim(operand);
  }
  return graph_.append(ExprGraph::Op::kConcat, width, sign, std::move(operands));
}

BitIndex ExprBuilder::normalize_packed_index(BitIndex index, BitIndex msb, BitIndex lsb) {
  if (lsb == 0 && msb >= lsb) {
    return index;
  }
  if (msb >= lsb) {
    return index - lsb;
  }
  return lsb - index;
}
ExprId ExprBuilder::normalize_packed_index_expr(ExprId index, BitIndex msb, BitIndex lsb,
                                                BitIndex index_offset) {
  if (index_offset == 0 && lsb == 0 && msb >= lsb) {
    return index;
  }

  SignalWidth index_width = graph_.outer_dim(index);
  if (!graph_.sign(index)) {
    assert(index_width < std::numeric_limits<SignalWidth>::max());
    ++index_width;
  }
  SignalWidth arithmetic_width =
      std::max({index_width, minimum_signed_width(msb), minimum_signed_width(lsb),
                minimum_signed_width(index_offset)});
  assert(arithmetic_width < std::numeric_limits<SignalWidth>::max());
  ++arithmetic_width;
  if (index_offset != 0) {
    assert(arithmetic_width < std::numeric_limits<SignalWidth>::max());
    ++arithmetic_width;
  }

  index = create_convert(index, arithmetic_width, true);
  if (index_offset != 0) {
    const ExprId offset = find_or_create_const(index_offset, arithmetic_width, true);
    index = create_add(index, offset);
  }
  const ExprId offset = find_or_create_const(lsb, arithmetic_width, true);
  if (msb >= lsb) {
    return create_sub(index, offset);
  }
  return create_sub(offset, index);
}

BitIndex ExprBuilder::normalize_unpacked_index(BitIndex index, BitIndex left, BitIndex right) {
  return index - std::min(left, right);
}

ExprId ExprBuilder::normalize_unpacked_index_expr(ExprId index, BitIndex left, BitIndex right,
                                                  BitIndex index_offset) {
  const BitIndex minimum = std::min(left, right);
  if (minimum == 0 && index_offset == 0) {
    assert(graph_.outer_dim(index) < std::numeric_limits<SignalWidth>::max());
    return create_convert(index, graph_.outer_dim(index) + 1, true);
  }
  return normalize_packed_index_expr(index, minimum, minimum, index_offset);
}
ExprId ExprBuilder::create_select(ExprId data, ExprId index, BitIndex msb, BitIndex lsb) {
  return create_range(data, normalize_packed_element_index(index, msb, lsb, 1), 1, false);
}

ExprId ExprBuilder::create_packed_array_select(ExprId data, ExprId index, BitIndex left,
                                               BitIndex right, SignalWidth element_width,
                                               bool sign) {
  if (get_outer_dim(data) == element_width) {
    return data;
  }
  if (element_width == 1) {
    return create_select(data, index, left, right);
  }
  const AffineIndex base = normalize_packed_element_index(index, left, right, element_width);
  return create_range(data, base, element_width, sign);
}
ExprId ExprBuilder::create_reverse(ExprId data) {
  const ExprId id = create_unary(ExprGraph::Op::kReverse, data);
  graph_.copy_unpacked_shape(id, data);
  return id;
}

ExprId ExprBuilder::create_static_range(ExprId data, SignalWidth offset, SignalWidth width,
                                        bool sign) {
  if (offset == 0 && width == get_outer_dim(data)) {
    return data;
  }
  assert(offset <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
  const BitIndex base = static_cast<BitIndex>(offset);
  return create_range(data, AffineIndex{base, {}}, width, sign);
}

ExprId ExprBuilder::create_simple_range(ExprId data, BitIndex left, BitIndex right, BitIndex msb,
                                        BitIndex lsb) {
  BitIndex left_pos = normalize_packed_index(left, msb, lsb);
  BitIndex right_pos = normalize_packed_index(right, msb, lsb);
  bool is_reverse = false;
  if (left_pos < right_pos) {
    is_reverse = true;
    std::swap(left_pos, right_pos);
  }
  ExprId id = create_range(data, AffineIndex{right_pos, {}}, left_pos - right_pos + 1, false);
  if (is_reverse) {
    return create_reverse(id);
  }
  return id;
}
ExprId ExprBuilder::create_range(ExprId data, const AffineIndex &base, SignalWidth width,
                                 bool sign) {
  assert(width > 0);
  const ExprId id = graph_.append(ExprGraph::Op::kRange, width, sign, {data});
  set_affine_index(id, base);
  return id;
}

ExprId ExprBuilder::create_packed_array_range(ExprId data, BitIndex left, BitIndex right,
                                              BitIndex array_left, BitIndex array_right,
                                              SignalWidth element_width, bool sign) {
  const PackedArrayRange range =
      get_packed_array_range(left, right, array_left, array_right, element_width);
  if (range.width == get_outer_dim(data) && range.offset == 0 && !range.reverse) {
    return data;
  }
  ExprId result = create_range(data, AffineIndex{range.offset, {}}, range.width, sign);
  if (range.reverse) {
    result = create_reverse(result);
  }
  return result;
}

ExprId ExprBuilder::create_packed_array_part_select(ExprId data, ExprId base,
                                                    SignalWidth selected_elements, bool indexed_up,
                                                    BitIndex array_left, BitIndex array_right,
                                                    SignalWidth element_width, bool sign) {
  assert(selected_elements > 0);
  const AffineIndex pos = normalize_packed_range_index(base, selected_elements, indexed_up,
                                                       array_left, array_right, element_width);
  const SignalWidth result_width = selected_elements * element_width;
  if (result_width == get_outer_dim(data)) {
    if (pos.terms.empty() && pos.offset == 0) {
      return data;
    }
  }
  return create_range(data, pos, result_width, sign);
}

AffineIndex ExprBuilder::normalize_packed_element_index(ExprId index, BitIndex left, BitIndex right,
                                                        SignalWidth element_width) const {
  const BitIndex scale = static_cast<BitIndex>(element_width);
  const BitIndex offset = left >= right ? -right * scale : right * scale;
  const BitIndex stride = left >= right ? scale : -scale;
  if (const auto value = try_evaluate(index)) {
    return {offset + *value * stride, {}};
  }
  return {offset, {{index, stride}}};
}

AffineIndex ExprBuilder::normalize_packed_range_index(ExprId index, SignalWidth selected_elements,
                                                      bool indexed_up, BitIndex left,
                                                      BitIndex right,
                                                      SignalWidth element_width) const {
  BitIndex index_offset = 0;
  if (selected_elements > 1 && indexed_up && left < right) {
    index_offset = static_cast<BitIndex>(selected_elements - 1);
  } else if (selected_elements > 1 && !indexed_up && left >= right) {
    index_offset = -static_cast<BitIndex>(selected_elements - 1);
  }
  AffineIndex result = normalize_packed_element_index(index, left, right, element_width);
  const BitIndex direction = left >= right ? 1 : -1;
  result.add_offset(direction * index_offset * static_cast<BitIndex>(element_width));
  return result;
}

void ExprBuilder::set_affine_index(ExprId id, const AffineIndex &index) {
  auto operands = graph_.operands(id);
  std::vector<BitIndex> strides;
  operands.reserve(operands.size() + index.terms.size());
  strides.reserve(index.terms.size());
  for (const auto &term : index.terms) {
    operands.push_back(term.index);
    strides.push_back(term.stride);
  }
  graph_.set_operands(id, std::move(operands));
  graph_.set_affine_index(id, index.offset, std::move(strides));
}

AffineIndex ExprBuilder::decode_affine_index(ExprId id) const {
  const auto &operands = graph_.operands(id);
  const auto &strides = graph_.affine_strides(id);
  const size_t first = graph_.op(id) == ExprGraph::Op::kRange ? 1 : 2;
  assert(operands.size() == first + strides.size());
  AffineIndex result{graph_.affine_offset(id), {}};
  result.terms.reserve(strides.size());
  for (size_t i = 0; i < strides.size(); ++i) {
    result.terms.push_back({operands[first + i], strides[i]});
  }
  return result;
}

ExprBuilder::PackedArrayRange ExprBuilder::get_packed_array_range(BitIndex left, BitIndex right,
                                                                  BitIndex array_left,
                                                                  BitIndex array_right,
                                                                  SignalWidth element_width) {
  BitIndex left_pos =
      normalize_packed_index(left, array_left, array_right) * static_cast<BitIndex>(element_width);
  BitIndex right_pos =
      normalize_packed_index(right, array_left, array_right) * static_cast<BitIndex>(element_width);
  left_pos += static_cast<BitIndex>(element_width - 1);
  bool reverse = false;
  if (left_pos < right_pos) {
    reverse = true;
    std::swap(left_pos, right_pos);
  }
  return {right_pos, static_cast<SignalWidth>(left_pos - right_pos + 1), reverse};
}

ExprId ExprBuilder::create_unpacked_range(ExprId data, ExprId base, SignalWidth width,
                                          std::vector<SignalWidth> unpacked_dims,
                                          SignalWidth inner_dim, bool element_sign) {
  if (width == get_outer_dim(data)) {
    const auto base_value = try_evaluate(base);
    if (base_value && *base_value == 0) {
      return data;
    }
  }
  const ExprId id = graph_.append(ExprGraph::Op::kUnpackedRange, width, false, {data, base});
  graph_.set_unpacked_shape(id, {std::move(unpacked_dims), inner_dim, element_sign});
  return id;
}

ExprId ExprBuilder::create_unpacked_range(ExprId data, BitIndex left, BitIndex right,
                                          BitIndex array_left, BitIndex array_right,
                                          std::vector<SignalWidth> unpacked_dims,
                                          SignalWidth inner_dim, bool element_sign) {
  const BitIndex left_pos = normalize_unpacked_index(left, array_left, array_right);
  const BitIndex right_pos = normalize_unpacked_index(right, array_left, array_right);
  const BitIndex low_pos = std::min(left_pos, right_pos);
  const ExprId base = find_or_create_const(low_pos, minimum_unsigned_width(low_pos), false);
  const SignalWidth width = static_cast<SignalWidth>(std::abs(left_pos - right_pos) + 1);
  return create_unpacked_range(data, base, width, std::move(unpacked_dims), inner_dim,
                               element_sign);
}

ExprId ExprBuilder::create_unpacked_part_select(ExprId data, ExprId base, SignalWidth slice_width,
                                                bool indexed_up, BitIndex array_left,
                                                BitIndex array_right,
                                                std::vector<SignalWidth> unpacked_dims,
                                                SignalWidth inner_dim, bool element_sign) {
  assert(slice_width > 0);
  assert(slice_width <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
  const BitIndex index_offset = indexed_up ? 0 : -static_cast<BitIndex>(slice_width - 1);
  base = normalize_unpacked_index_expr(base, array_left, array_right, index_offset);
  return create_unpacked_range(data, base, slice_width, std::move(unpacked_dims), inner_dim,
                               element_sign);
}

ExprId ExprBuilder::create_gather(std::vector<ExprId> operands,
                                  std::vector<SignalWidth> unpacked_dims, SignalWidth inner_dim,
                                  bool element_sign) {
  assert(!operands.empty());
  assert(!unpacked_dims.empty());
  assert(inner_dim > 0);
  const SignalWidth width = operands.size();
  const ExprId id = graph_.append(ExprGraph::Op::kGather, width, false, std::move(operands));
  graph_.set_unpacked_shape(id, {std::move(unpacked_dims), inner_dim, element_sign});
  return id;
}

ExprId ExprBuilder::create_unpacked_concat(std::vector<ExprId> operands,
                                           std::vector<SignalWidth> unpacked_dims,
                                           SignalWidth inner_dim, bool element_sign) {
  assert(!operands.empty());
  assert(!unpacked_dims.empty());
  assert(unpacked_dims.front() > 0);
  assert(inner_dim > 0);
  const ExprId id = graph_.append(ExprGraph::Op::kUnpackedConcat, unpacked_dims.front(), false,
                                  std::move(operands));
  graph_.set_unpacked_shape(id, {std::move(unpacked_dims), inner_dim, element_sign});
  return id;
}

ExprId ExprBuilder::create_unpacked_flatten(ExprId data) {
  const auto *unpacked_shape = graph_.unpacked_shape(data);
  assert(unpacked_shape != nullptr);
  assert(!unpacked_shape->unpacked_dims.empty());
  assert(unpacked_shape->inner_dim > 0);
  assert(graph_.outer_dim(data) == unpacked_shape->unpacked_dims.front());
  const SignalWidth width = unpacked_shape->flattened_width();
  const ExprId id =
      graph_.append(ExprGraph::Op::kUnpackedFlatten, width, unpacked_shape->sign, {data});
  return id;
}

ExprId ExprBuilder::create_unpacked_fold(ExprId data, std::vector<SignalWidth> unpacked_dims,
                                         SignalWidth inner_dim, bool element_sign) {
  assert(!unpacked_dims.empty());
  assert(inner_dim > 0);
  UnpackedShape shape{std::move(unpacked_dims), inner_dim, element_sign};
  assert(graph_.outer_dim(data) == shape.flattened_width());
  const ExprId id =
      graph_.append(ExprGraph::Op::kUnpackedFold, shape.unpacked_dims.front(), false, {data});
  graph_.set_unpacked_shape(id, std::move(shape));
  return id;
}

ExprId ExprBuilder::create_sequence(ExprId next, ExprId base,
                                    std::vector<SignalWidth> unpacked_dims, SignalWidth width,
                                    bool sign) {
  assert(next != kInvalidExprId);
  const ExprId id =
      graph_.append(ExprGraph::Op::kSequence, graph_.outer_dim(next), graph_.sign(next), {base, next});
  graph_.set_unpacked_shape(id, {std::move(unpacked_dims), width, sign});
  return id;
}

ExprId ExprBuilder::create_sequence(ExprId current, ExprId next) {
  assert(current != kInvalidExprId);
  assert(next != kInvalidExprId);
  assert(graph_.op(current) == ExprGraph::Op::kSequence);
  assert(graph_.outer_dim(next) == graph_.outer_dim(current));
  std::vector<ExprId> operands = graph_.operands(current);
  operands.push_back(next);
  assert(graph_.unpacked_shape(current));
  const ExprId id = graph_.append(ExprGraph::Op::kSequence, graph_.outer_dim(next), graph_.sign(next),
                                  std::move(operands));
  graph_.copy_unpacked_shape(id, current);
  return id;
}

ExprId ExprBuilder::create_sequence(ExprId current, ExprId next, ExprId unpacked_shape_source) {
  assert(next != kInvalidExprId);
  std::vector<ExprId> operands;
  if (current != kInvalidExprId) {
    assert(graph_.op(current) == ExprGraph::Op::kSequence);
    assert(graph_.outer_dim(next) == graph_.outer_dim(current));
    operands = graph_.operands(current);
  } else {
    operands.push_back(kInvalidExprId);
  }
  operands.push_back(next);
  assert(graph_.unpacked_shape(unpacked_shape_source));
  const ExprId id = graph_.append(ExprGraph::Op::kSequence, graph_.outer_dim(next), graph_.sign(next),
                                  std::move(operands));
  graph_.copy_unpacked_shape(id, unpacked_shape_source);
  return id;
}
ExprId ExprBuilder::create_unpacked_assign(ExprId next, ExprId index,
                                           std::vector<SignalWidth> unpacked_dims,
                                           SignalWidth inner_dim, bool element_sign) {
  assert(!unpacked_dims.empty());
  const ExprId id =
      graph_.append(ExprGraph::Op::kUnpackedAssign, unpacked_dims.front(), false, {next, index});
  graph_.set_unpacked_shape(id, {std::move(unpacked_dims), inner_dim, element_sign});
  return id;
}

ExprId ExprBuilder::create_unpacked_range_assign(ExprId next, ExprId base, SignalWidth slice_width,
                                                 std::vector<SignalWidth> unpacked_dims,
                                                 SignalWidth inner_dim, bool element_sign) {
  assert(!unpacked_dims.empty());
  std::vector<ExprId> operands{next};
  if (base != kInvalidExprId) {
    operands.push_back(base);
  }
  const auto id = graph_.append(ExprGraph::Op::kUnpackedRangeAssign, unpacked_dims.front(), false,
                                std::move(operands));
  graph_.set_slice_width(id, slice_width);
  graph_.set_unpacked_shape(id, {std::move(unpacked_dims), inner_dim, element_sign});
  return id;
}
ExprId ExprBuilder::create_masked_assign(ExprId current, ExprId next, const AffineIndex &base,
                                         SignalWidth slice_width, SignalWidth width, bool sign) {
  assert(current != kInvalidExprId);
  assert(slice_width > 0);
  const ExprId id = graph_.append(ExprGraph::Op::kMaskedAssign, width, sign, {current, next});
  graph_.set_slice_width(id, slice_width);
  set_affine_index(id, base);
  return id;
}

ExprId ExprBuilder::unpacked_assign_select(ExprId next, ExprId index, BitIndex left, BitIndex right,
                                           std::vector<SignalWidth> unpacked_dims,
                                           SignalWidth inner_dim, bool element_sign) {
  const ExprId pos =
      std::min(left, right) == 0 ? index : normalize_unpacked_index_expr(index, left, right);
  return create_unpacked_assign(next, pos, std::move(unpacked_dims), inner_dim, element_sign);
}

ExprId ExprBuilder::unpacked_assign_range(ExprId next, BitIndex left, BitIndex right,
                                          BitIndex array_left, BitIndex array_right,
                                          std::vector<SignalWidth> unpacked_dims,
                                          SignalWidth inner_dim, bool element_sign) {
  BitIndex left_pos = normalize_unpacked_index(left, array_left, array_right);
  BitIndex right_pos = normalize_unpacked_index(right, array_left, array_right);
  if (left_pos > right_pos) {
    next = create_reverse(next);
    std::swap(left_pos, right_pos);
  }
  const SignalWidth range_width = right_pos - left_pos + 1;
  const auto id = create_unpacked_range_assign(
      next, kInvalidExprId, range_width, std::move(unpacked_dims), inner_dim, element_sign);
  assert(left_pos >= 0);
  graph_.set_unpacked_range_assign_base(id, left_pos);
  return id;
}

ExprId ExprBuilder::unpacked_assign_part_select(ExprId next, ExprId base, SignalWidth slice_width,
                                                bool indexed_up, BitIndex left, BitIndex right,
                                                std::vector<SignalWidth> unpacked_dims,
                                                SignalWidth inner_dim, bool element_sign) {
  assert(slice_width > 0);
  assert(slice_width <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
  const BitIndex index_offset = indexed_up ? 0 : -static_cast<BitIndex>(slice_width - 1);
  const ExprId base_id = normalize_unpacked_index_expr(base, left, right, index_offset);
  return create_unpacked_range_assign(next, base_id, slice_width, std::move(unpacked_dims),
                                      inner_dim, element_sign);
}

ExprId ExprBuilder::create_call(SubrId subr_id, std::string name, std::vector<ExprId> operands,
                                SignalWidth width, bool sign) {
  const ExprId id = graph_.append(ExprGraph::Op::kCall, width, sign, std::move(operands));
  graph_.set_call(id, subr_id, std::move(name));
  return id;
}

ExprId ExprBuilder::create_both_edge(ExprId operand) {
  return graph_.append(ExprGraph::Op::kBothEdge, 1, false, {operand});
}

ExprId ExprBuilder::create_unpacked_select(ExprId data, ExprId index, BitIndex left, BitIndex right,
                                           SignalWidth width, bool sign,
                                           std::vector<SignalWidth> unpacked_dims,
                                           SignalWidth inner_dim, bool element_sign) {
  if (std::min(left, right) != 0) {
    index = normalize_unpacked_index_expr(index, left, right);
  }
  const ExprId id = graph_.append(ExprGraph::Op::kUnpackedSelect, width, sign, {data, index});
  if (!unpacked_dims.empty()) {
    graph_.set_unpacked_shape(id, {std::move(unpacked_dims), inner_dim, element_sign});
  }
  return id;
}

ExprId ExprBuilder::get_current_value(std::string_view name) const {
  auto it = name_map_.find(name);
  if (it != name_map_.end()) {
    return it->second;
  }
  return kInvalidExprId;
}
void ExprBuilder::update_value(std::string name, ExprId id) {
  name_map_.insert_or_assign(std::move(name), id);
}
void ExprBuilder::remove_input(std::string_view name) {
  const auto input = input_map_->find(name);
  if (input != input_map_->end()) {
    input_map_->erase(input);
  }
}

bool ExprBuilder::get_input_spec(ExprId id, ExprId &input_id, std::string &name, SignalWidth &width,
                                 bool &sign) const {
  input_id = id;
  if (graph_.op(input_id) == ExprGraph::Op::kConvert) {
    input_id = graph_.operands(input_id)[0];
  }
  if (graph_.op(input_id) != ExprGraph::Op::kInput) {
    return false;
  }
  bool found = false;
  for (const auto &kv : *input_map_) {
    if (kv.second == input_id) {
      name = kv.first;
      found = true;
      break;
    }
  }
  if (!found) {
    return false;
  }
  width = graph_.outer_dim(input_id);
  sign = graph_.sign(input_id);
  return true;
}

bool ExprBuilder::check_dependency_rec(ExprId id, ExprId target,
                                       std::unordered_set<ExprId> &visited) const {
  if (id == target) {
    return true;
  }
  if (id == kInvalidExprId) {
    return false;
  }
  if (!visited.insert(id).second) {
    return false;
  }
  for (ExprId operand : graph_.operands(id)) {
    if (check_dependency_rec(operand, target, visited)) {
      return true;
    }
  }
  return false;
}

bool ExprBuilder::check_dependency(ExprId id, ExprId target) const {
  std::unordered_set<ExprId> visited;
  return check_dependency_rec(id, target, visited);
}

std::optional<BitIndex> ExprBuilder::try_evaluate(ExprId id) const {
  assert(id != kInvalidExprId);
  switch (graph_.op(id)) {
  case ExprGraph::Op::kCount:
  case ExprGraph::Op::kUnknown:
  case ExprGraph::Op::kInput:
    return std::nullopt;
  case ExprGraph::Op::kConst: {
    return parse_integral_constant(graph_.constant_value(id));
  }
  case ExprGraph::Op::kLogicalNot:
    if (auto opr = try_evaluate(graph_.operands(id)[0])) {
      return !*opr;
    }
    return std::nullopt;
  case ExprGraph::Op::kAndReduce: {
    const ExprId op_id = graph_.operands(id)[0];
    auto opr = try_evaluate(op_id);
    if (!opr) {
      return std::nullopt;
    }
    for (SignalWidth i = 0; i < graph_.outer_dim(op_id); ++i) {
      if (((*opr >> i) & 1) == 0) {
        return 0;
      }
    }
    return 1;
  }
  case ExprGraph::Op::kOrReduce: {
    const ExprId op_id = graph_.operands(id)[0];
    auto opr = try_evaluate(op_id);
    if (!opr) {
      return std::nullopt;
    }
    for (SignalWidth i = 0; i < graph_.outer_dim(op_id); ++i) {
      if ((*opr >> i) & 1) {
        return 1;
      }
    }
    return 0;
  }
  case ExprGraph::Op::kXorReduce: {
    const ExprId op_id = graph_.operands(id)[0];
    auto opr = try_evaluate(op_id);
    if (!opr) {
      return std::nullopt;
    }
    BitIndex value = 0;
    for (SignalWidth i = 0; i < graph_.outer_dim(op_id); ++i) {
      value ^= (*opr >> i) & 1;
    }
    return value;
  }
  case ExprGraph::Op::kBitwiseNot:
    if (auto opr = try_evaluate(graph_.operands(id)[0])) {
      return ~*opr;
    }
    return std::nullopt;
  case ExprGraph::Op::kAnd:
  case ExprGraph::Op::kOr:
  case ExprGraph::Op::kXor: {
    if (graph_.operands(id).empty()) {
      return std::nullopt;
    }
    auto value = try_evaluate(graph_.operands(id)[0]);
    if (!value) {
      return std::nullopt;
    }
    for (size_t i = 1; i < graph_.operands(id).size(); ++i) {
      auto opr = try_evaluate(graph_.operands(id)[i]);
      if (!opr) {
        return std::nullopt;
      }
      if (graph_.op(id) == ExprGraph::Op::kAnd) {
        *value &= *opr;
      } else if (graph_.op(id) == ExprGraph::Op::kOr) {
        *value |= *opr;
      } else {
        *value ^= *opr;
      }
    }
    return value;
  }
  case ExprGraph::Op::kUnaryMinus:
    if (auto opr = try_evaluate(graph_.operands(id)[0])) {
      return -*opr;
    }
    return std::nullopt;
  case ExprGraph::Op::kAdd:
  case ExprGraph::Op::kSub:
  case ExprGraph::Op::kMul:
  case ExprGraph::Op::kDiv:
  case ExprGraph::Op::kMod:
  case ExprGraph::Op::kPow:
  case ExprGraph::Op::kShl:
  case ExprGraph::Op::kShr:
  case ExprGraph::Op::kEq:
  case ExprGraph::Op::kLt:
  case ExprGraph::Op::kLe: {
    auto lhs = try_evaluate(graph_.operands(id)[0]);
    auto rhs = try_evaluate(graph_.operands(id)[1]);
    if (!lhs || !rhs) {
      return std::nullopt;
    }
    switch (graph_.op(id)) {
    case ExprGraph::Op::kAdd:
      return *lhs + *rhs;
    case ExprGraph::Op::kSub:
      return *lhs - *rhs;
    case ExprGraph::Op::kMul:
      return *lhs * *rhs;
    case ExprGraph::Op::kDiv:
      return *rhs == 0 ? std::nullopt : std::optional<BitIndex>(*lhs / *rhs);
    case ExprGraph::Op::kMod:
      return *rhs == 0 ? std::nullopt : std::optional<BitIndex>(*lhs % *rhs);
    case ExprGraph::Op::kPow:
      return static_cast<BitIndex>(std::pow(*lhs, *rhs));
    case ExprGraph::Op::kShl:
      return *lhs << *rhs;
    case ExprGraph::Op::kShr:
      return *lhs >> *rhs;
    case ExprGraph::Op::kEq:
      return *lhs == *rhs;
    case ExprGraph::Op::kLt:
      return *lhs < *rhs;
    case ExprGraph::Op::kLe:
      return *lhs <= *rhs;
    default:
      assert(0);
    }
  }
  case ExprGraph::Op::kAshr: {
    auto value = try_evaluate(graph_.operands(id)[0]);
    auto shamt = try_evaluate(graph_.operands(id)[1]);
    if (!value || !shamt) {
      return std::nullopt;
    }
    BitIndex shifted = *value;
    const BitIndex mask = shifted & (BitIndex{1} << (graph_.outer_dim(id) - 1));
    for (BitIndex i = 0; i < *shamt; ++i) {
      shifted = (shifted >> 1) | mask;
    }
    return shifted;
  }
  case ExprGraph::Op::kMux:
    if (auto cond = try_evaluate(graph_.operands(id)[0])) {
      return try_evaluate(*cond ? graph_.operands(id)[1] : graph_.operands(id)[2]);
    }
    return std::nullopt;
  case ExprGraph::Op::kUnpackedMux:
    return std::nullopt;
  case ExprGraph::Op::kConvert: {
    const auto value = try_evaluate(graph_.operands(id)[0]);
    return value
               ? std::optional(apply_integral_conversion(*value, graph_.outer_dim(id), graph_.sign(id)))
               : std::nullopt;
  }
    // TODO: extend these cases after constant evaluation supports structured arbitrary-width
    // values.
  case ExprGraph::Op::kList:
  case ExprGraph::Op::kCase:
  case ExprGraph::Op::kUnpackedCase:
  case ExprGraph::Op::kPmux:
  case ExprGraph::Op::kConcat:
  case ExprGraph::Op::kGather:
  case ExprGraph::Op::kUnpackedConcat:
  case ExprGraph::Op::kUnpackedFlatten:
  case ExprGraph::Op::kUnpackedFold:
  case ExprGraph::Op::kSequence:
  case ExprGraph::Op::kUnpackedAssign:
  case ExprGraph::Op::kUnpackedRangeAssign:
  case ExprGraph::Op::kMaskedAssign:
  case ExprGraph::Op::kReverse:
  case ExprGraph::Op::kRange:
  case ExprGraph::Op::kUnpackedRange:
  case ExprGraph::Op::kUnpackedSelect:
  case ExprGraph::Op::kBothEdge:
  case ExprGraph::Op::kCall:
    return std::nullopt;
  }
  assert(0);
}

BitIndex ExprBuilder::evaluate(ExprId id) const {
  assert(id != kInvalidExprId);
  switch (graph_.op(id)) {
  case ExprGraph::Op::kCount:
  case ExprGraph::Op::kUnknown:
    assert(0);
    break;
  case ExprGraph::Op::kConst: {
    const auto value = parse_integral_constant(graph_.constant_value(id));
    assert(value.has_value());
    return *value;
  }
  case ExprGraph::Op::kInput:
    assert(0);
    break;
  case ExprGraph::Op::kLogicalNot:
    return !evaluate(graph_.operands(id)[0]);
  case ExprGraph::Op::kAndReduce: {
    const ExprId op_id = graph_.operands(id)[0];
    BitIndex opr = evaluate(op_id);
    for (SignalWidth i = 0; i < graph_.outer_dim(op_id); ++i) {
      if (((opr >> i) & 1) == 0) {
        return 0;
      }
    }
    return 1;
  }
  case ExprGraph::Op::kOrReduce: {
    const ExprId op_id = graph_.operands(id)[0];
    BitIndex opr = evaluate(op_id);
    for (SignalWidth i = 0; i < graph_.outer_dim(op_id); ++i) {
      if ((opr >> i) & 1) {
        return 1;
      }
    }
    return 0;
  }
  case ExprGraph::Op::kXorReduce: {
    const ExprId op_id = graph_.operands(id)[0];
    BitIndex opr = evaluate(op_id);
    int value = 0;
    for (SignalWidth i = 0; i < graph_.outer_dim(op_id); ++i) {
      value ^= (opr >> i) & 1;
    }
    return value;
  }
  case ExprGraph::Op::kBitwiseNot:
    return ~evaluate(graph_.operands(id)[0]);
  case ExprGraph::Op::kAnd:
    return evaluate(graph_.operands(id)[0]) & evaluate(graph_.operands(id)[1]);
  case ExprGraph::Op::kOr:
    return evaluate(graph_.operands(id)[0]) | evaluate(graph_.operands(id)[1]);
  case ExprGraph::Op::kXor:
    return evaluate(graph_.operands(id)[0]) ^ evaluate(graph_.operands(id)[1]);
  case ExprGraph::Op::kUnaryMinus:
    return -evaluate(graph_.operands(id)[0]);
  case ExprGraph::Op::kAdd:
    return evaluate(graph_.operands(id)[0]) + evaluate(graph_.operands(id)[1]);
  case ExprGraph::Op::kSub:
    return evaluate(graph_.operands(id)[0]) - evaluate(graph_.operands(id)[1]);
  case ExprGraph::Op::kMul:
    return evaluate(graph_.operands(id)[0]) * evaluate(graph_.operands(id)[1]);
  case ExprGraph::Op::kDiv:
    return evaluate(graph_.operands(id)[0]) / evaluate(graph_.operands(id)[1]);
  case ExprGraph::Op::kMod:
    return evaluate(graph_.operands(id)[0]) % evaluate(graph_.operands(id)[1]);
  case ExprGraph::Op::kPow:
    return static_cast<BitIndex>(
        std::pow(evaluate(graph_.operands(id)[0]), evaluate(graph_.operands(id)[1])));
  case ExprGraph::Op::kShl:
    return evaluate(graph_.operands(id)[0]) << evaluate(graph_.operands(id)[1]);
  case ExprGraph::Op::kShr:
    return evaluate(graph_.operands(id)[0]) >> evaluate(graph_.operands(id)[1]);
  case ExprGraph::Op::kAshr: {
    BitIndex value = evaluate(graph_.operands(id)[0]);
    BitIndex shamt = evaluate(graph_.operands(id)[1]);
    const BitIndex mask = value & (BitIndex{1} << (graph_.outer_dim(id) - 1));
    for (BitIndex i = 0; i < shamt; ++i) {
      value = (value >> 1) | mask;
    }
    return value;
  }
  case ExprGraph::Op::kEq:
    return evaluate(graph_.operands(id)[0]) == evaluate(graph_.operands(id)[1]);
  case ExprGraph::Op::kLt:
    return evaluate(graph_.operands(id)[0]) < evaluate(graph_.operands(id)[1]);
  case ExprGraph::Op::kLe:
    return evaluate(graph_.operands(id)[0]) <= evaluate(graph_.operands(id)[1]);
  case ExprGraph::Op::kMux:
    return evaluate(graph_.operands(id)[0]) ? evaluate(graph_.operands(id)[1])
                                            : evaluate(graph_.operands(id)[2]);
  case ExprGraph::Op::kUnpackedMux:
    assert(0);
    break;
  case ExprGraph::Op::kList:
  case ExprGraph::Op::kCase:
  case ExprGraph::Op::kUnpackedCase:
  case ExprGraph::Op::kPmux:
    assert(0);
    break;
  case ExprGraph::Op::kConvert:
    return apply_integral_conversion(evaluate(graph_.operands(id)[0]), graph_.outer_dim(id),
                                     graph_.sign(id));
  case ExprGraph::Op::kConcat:
  case ExprGraph::Op::kGather:
  case ExprGraph::Op::kUnpackedConcat:
  case ExprGraph::Op::kUnpackedFlatten:
  case ExprGraph::Op::kUnpackedFold:
  case ExprGraph::Op::kSequence:
  case ExprGraph::Op::kUnpackedAssign:
  case ExprGraph::Op::kUnpackedRangeAssign:
  case ExprGraph::Op::kMaskedAssign:
  case ExprGraph::Op::kReverse:
  case ExprGraph::Op::kRange:
  case ExprGraph::Op::kUnpackedRange:
  case ExprGraph::Op::kUnpackedSelect:
  case ExprGraph::Op::kBothEdge:
  case ExprGraph::Op::kCall:
    assert(0);
    break;
  }
  return -1;
}

} // namespace abys::ir
