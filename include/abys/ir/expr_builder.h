#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "abys/infra/diagnostics.h"
#include "abys/ir/expr.h"
#include "abys/util/string_map.h"

namespace abys::ir {

class ExprBuilder {
public:
  ExprBuilder(ExprGraph &graph, Diagnostics &diagnostics);
  explicit ExprBuilder(const ExprBuilder &parent);

  static ExprId get_constant_zero();
  static ExprId get_constant_one();

  ExprGraph::Node &get_node(ExprId id);
  SignalWidth get_width(ExprId id) const;
  bool get_sign(ExprId id) const;
  bool is_sequence(ExprId id) const;

  ExprId find_or_create_input(std::string name, SignalWidth width, bool sign);
  ExprId find_or_create_const(std::string value, SignalWidth width, bool sign);
  ExprId find_or_create_const(BitIndex index, SignalWidth width, bool sign);
  static SignalWidth minimum_unsigned_width(BitIndex index);

  ExprId create_logical_not(ExprId operand);
  ExprId create_and_reduce(ExprId operand);
  ExprId create_or_reduce(ExprId operand);
  ExprId create_xor_reduce(ExprId operand);

  ExprId create_bitwise_not(ExprId operand);

  ExprId create_and(std::vector<ExprId> operands);
  ExprId create_or(std::vector<ExprId> operands);
  ExprId create_xor(std::vector<ExprId> operands);

  static ExprId create_unary_plus(ExprId a); // nop
  ExprId create_unary_minus(ExprId a);

  ExprId create_logical_and(ExprId a, ExprId b);
  ExprId create_logical_or(ExprId a, ExprId b);

  ExprId create_add(ExprId a, ExprId b);
  ExprId create_sub(ExprId a, ExprId b);
  ExprId create_mul(ExprId a, ExprId b);
  ExprId create_div(ExprId a, ExprId b);
  ExprId create_mod(ExprId a, ExprId b);
  ExprId create_pow(ExprId a, ExprId b);

  ExprId create_shl(ExprId data, ExprId shamt);
  ExprId create_shr(ExprId data, ExprId shamt);
  ExprId create_ashr(ExprId data, ExprId shamt);

  ExprId create_eq(ExprId a, ExprId b);
  ExprId create_neq(ExprId a, ExprId b); // map to logical_not(eq)
  ExprId create_lt(ExprId a, ExprId b);
  ExprId create_le(ExprId a, ExprId b);
  ExprId create_gt(ExprId a, ExprId b); // map to lt(b, a)
  ExprId create_ge(ExprId a, ExprId b); // map to le(b, a)

  ExprId create_mux(ExprId cond, ExprId then, ExprId else_id); // cond ? then : else;

  ExprId create_list(std::vector<ExprId> operands);
  ExprId create_match(ExprId selector, ExprId case_value); // case_value can be a list
  ExprId create_case(ExprId selector, std::vector<ExprId> case_values,
                     std::vector<ExprId> data_ids);

  ExprId create_convert(ExprId a, SignalWidth width, bool sign);

  ExprId create_concat(std::vector<ExprId> operands, bool sign = false);

  static BitIndex normalize_index(BitIndex index, BitIndex msb, BitIndex lsb);
  ExprId normalize_index_expr(ExprId index, BitIndex msb, BitIndex lsb, BitIndex index_offset = 0);

  ExprId create_select(ExprId data, ExprId index, BitIndex msb,
                       BitIndex lsb); // normalize and maps to kRange
  ExprId create_reverse(ExprId data);
  ExprId create_simple_range(ExprId data, BitIndex left, BitIndex right, BitIndex msb,
                             BitIndex lsb); // normalize and stores the low base as operands[1]
  ExprId create_range(ExprId data, ExprId base, SignalWidth width, bool sign);
  ExprId create_unpacked_range(ExprId data, ExprId base, SignalWidth width);

  ExprId create_gather(std::vector<ExprId> operands);
  ExprId create_sequence(ExprId next, ExprId base, std::vector<SignalWidth> unpacked_dims,
                         SignalWidth width, bool sign);
  ExprId create_sequence(ExprId current, ExprId next);
  ExprId create_sequence(ExprId current, ExprId next, ExprId unpacked_properties_source);
  ExprId create_unpacked_assign(ExprId next, ExprId base, ExprId slice_width, SignalWidth width,
                                bool sign);
  ExprId create_masked_assign(ExprId current, ExprId next, ExprId base, SignalWidth slice_width,
                              SignalWidth width, bool sign);

  ExprId unpacked_assign_select(ExprId next, ExprId index, BitIndex msb, BitIndex lsb,
                                SignalWidth width, bool sign);
  ExprId unpacked_assign_range(ExprId next, BitIndex left, BitIndex right, BitIndex msb,
                               BitIndex lsb, SignalWidth width, bool sign);
  ExprId unpacked_assign_part_select(ExprId next, ExprId base, SignalWidth slice_width, bool dir,
                                     BitIndex msb, BitIndex lsb, SignalWidth width, bool sign);

  ExprId create_unpacked_select(ExprId data, ExprId index, BitIndex msb, BitIndex lsb,
                                SignalWidth width, bool sign,
                                std::vector<SignalWidth> unpacked_dims = {},
                                SignalWidth element_width = 0, bool element_sign = false);

  ExprId create_call(SubrId subr_id, std::string name, std::vector<ExprId> operands,
                     SignalWidth width, bool sign);

  ExprId create_both_edge(ExprId operand);

  template <typename Func> void for_each_input(Func &&func) const {
    for (auto const &input : graph_.inputs) {
      auto const &node = graph_.nodes[input.second];
      func(input.first, node.width, node.sign);
    }
  }

  ExprId get_current_value(std::string_view name) const;
  void update_value(std::string name, ExprId id);
  void remove_input(std::string_view name);

  bool get_input_spec(ExprId id, ExprId &input_id, std::string &name, SignalWidth &width,
                      bool &sign) const;
  bool check_dependency(ExprId id, ExprId target) const;

  std::optional<int> try_evaluate(ExprId id) const;
  int evaluate(ExprId id) const;

private:
  ExprGraph &graph_;
  Diagnostics &diagnostics_;
  abys::util::StringMap<ExprId> name_map_;

  ExprId create_node();

  ExprId create_unary_reduce(ExprGraph::Op op, ExprId operand);
  ExprId create_unary(ExprGraph::Op op, ExprId operand);
  ExprId create_nary(ExprGraph::Op op, std::vector<ExprId> operands);
  ExprId create_logical_binary(ExprGraph::Op op, ExprId a, ExprId b);
  ExprId create_binary(ExprGraph::Op op, ExprId a, ExprId b);
  ExprId create_shift(ExprGraph::Op op, ExprId data, ExprId shamt);
  ExprId create_compare(ExprGraph::Op op, ExprId a, ExprId b);
  bool check_dependency_rec(ExprId id, ExprId target, std::unordered_set<ExprId> &visited) const;
};

} // namespace abys::ir
