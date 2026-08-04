#pragma once

#include <cassert>
#include <cctype>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

#include "slang/ast/ASTVisitor.h"
#include "slang/ast/Compilation.h"
#include "slang/ast/SemanticFacts.h"
#include "slang/ast/expressions/AssignmentExpressions.h"
#include "slang/ast/expressions/ConversionExpression.h"
#include "slang/ast/expressions/MiscExpressions.h"
#include "slang/ast/expressions/OperatorExpressions.h"
#include "slang/ast/symbols/CompilationUnitSymbols.h"
#include "slang/ast/symbols/InstanceSymbols.h"
#include "slang/ast/symbols/MemberSymbols.h"
#include "slang/ast/symbols/ParameterSymbols.h"
#include "slang/ast/symbols/PortSymbols.h"
#include "slang/ast/types/Type.h"
#include "slang/driver/Driver.h"
#include "slang/parsing/KnownSystemName.h"
#include "slang/syntax/AllSyntax.h"

#include "abys/frontend/slang_pragma.h"
#include "abys/ir/expr_builder.h"
#include "abys/ir/stmt_builder.h"
#include "abys/ir/tig_builder.h"

namespace abys::frontend {

using namespace abys::ir;

struct SlangLoweringContext {
  explicit SlangLoweringContext(Diagnostics &diagnostics) : diagnostics(diagnostics) {}

  Diagnostics &diagnostics;
  std::unordered_map<const slang::ast::Symbol *, std::string> special_symbols;
  std::unordered_map<const slang::ast::SubroutineSymbol *, SubrId> subr_ids;

  SubrId get_or_create_subr_id(const slang::ast::SubroutineSymbol &symbol);
};

struct SignalType {
  std::vector<SignalWidth> unpacked_dims;
  SignalWidth width = 0;
  bool sign = false;
};

const char *definition_kind_to_string(slang::ast::DefinitionKind kind);
SignalWidth expr_width(const slang::ast::Expression &expr);
bool expr_sign(const slang::ast::Expression &expr);
std::string make_verilog_identifier(std::string_view name);
std::string
lower_symbol_name(const slang::ast::Symbol &symbol,
                  std::unordered_map<const slang::ast::Symbol *, std::string> &special_symbols);
std::string
register_symbol_name(const slang::ast::Symbol &symbol,
                     std::unordered_map<const slang::ast::Symbol *, std::string> &special_symbols,
                     std::string_view suffix = "");
std::string
extract_named_value(const slang::ast::Expression &expr,
                    std::unordered_map<const slang::ast::Symbol *, std::string> &special_symbols);
std::optional<BitIndex> try_extract_constant_index(const slang::ast::Expression &expr);
void get_width_sign(const slang::ast::Type &type, SignalWidth &width, bool &sign,
                    Diagnostics &diagnostics);
SignalType get_signal_type(const slang::ast::Type &type, Diagnostics &diagnostics);

ExprId build_expr(const slang::ast::Expression &expr, ExprBuilder &expr_builder,
                  SlangLoweringContext &context, ExprId compound_lhs_id = kInvalidExprId);

void lower_statement(const slang::ast::Statement &statement, StmtBuilder &builder,
                     SlangLoweringContext &context, const PragmaMap &pragmas);

template <typename EmitFn>
void lower_lhs_assignment(const slang::ast::Expression &whole_lhs, ExprId rhs_id,
                          SignalWidth rhs_width, ExprBuilder &expr_builder,
                          SlangLoweringContext &context,
                          const std::unordered_map<std::string, ExprId> *scheduled_assignments,
                          EmitFn &&record) {
  auto &special_symbols = context.special_symbols;
  assert(rhs_width > 0);
  assert(rhs_width <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
  SignalWidth remaining = rhs_width;

  auto extract_lhs_base_name = [&](auto &&self, const slang::ast::Expression &lhs) -> std::string {
    if (lhs.kind == slang::ast::ExpressionKind::NamedValue) {
      return extract_named_value(lhs, special_symbols);
    }
    if (lhs.kind == slang::ast::ExpressionKind::ElementSelect) {
      const auto &sel = lhs.as<slang::ast::ElementSelectExpression>();
      return self(self, sel.value());
    }
    if (lhs.kind == slang::ast::ExpressionKind::RangeSelect) {
      const auto &sel = lhs.as<slang::ast::RangeSelectExpression>();
      return self(self, sel.value());
    }
    return {};
  };

  auto get_rhs_id = [&](const slang::ast::Expression &lhs) -> ExprId {
    ExprId expr_id = rhs_id;
    SignalWidth width;
    bool sign;
    get_width_sign(*lhs.type, width, sign, context.diagnostics);
    assert(remaining >= width);
    if (width != rhs_width) {
      const BitIndex left = static_cast<BitIndex>(remaining - 1);
      const BitIndex right = static_cast<BitIndex>(remaining - width);
      expr_id = expr_builder.create_simple_range(rhs_id, left, right,
                                                 static_cast<BitIndex>(rhs_width - 1), 0);
    }
    remaining -= width;
    return expr_id;
  };

  auto finalize_packed_update = [&](const slang::ast::Expression &lhs, const ExprId expr_id,
                                    const ExprId base_id, auto &&get_fallback) -> ExprId {
    SignalWidth width;
    bool sign;
    get_width_sign(*lhs.type, width, sign, context.diagnostics);
    const SignalWidth slice_width = expr_builder.get_width(expr_id);
    bool is_full_width = false;
    if (slice_width == width) {
      const auto base_value = expr_builder.try_evaluate(base_id);
      is_full_width = base_value && *base_value == 0;
    }
    if (!is_full_width) {
      const ExprId fallback_id = get_fallback(width, sign);
      return expr_builder.create_masked_assign(fallback_id, expr_id, base_id, slice_width, width,
                                               sign);
    }
    if (expr_builder.get_sign(expr_id) != sign) {
      return expr_builder.create_convert(expr_id, width, sign);
    }
    return expr_id;
  };

  auto get_range = [&](const slang::ast::Type &type, slang::ConstantRange &range) -> bool {
    if (type.isUnpackedArray()) {
      const auto &ct = type.getCanonicalType();
      if (ct.kind != slang::ast::SymbolKind::FixedSizeUnpackedArrayType) {
        context.diagnostics.error(DiagnosticId::kLoweringUnsupportedAssignmentIgnored,
                                  "dynamic-size unpacked array");
        return false;
      }
      range = ct.as<slang::ast::FixedSizeUnpackedArrayType>().range;
    } else {
      range = type.getFixedRange();
    }
    return true;
  };

  auto assign_rec = [&](auto &&self, const slang::ast::Expression &lhs, const ExprId expr_id,
                        const ExprId current_id, const ExprId base_id) -> ExprId {
    if (lhs.kind == slang::ast::ExpressionKind::NamedValue) {
      if (lhs.type->isUnpackedArray()) {
        ExprId sequence_id = kInvalidExprId;
        if (current_id != kInvalidExprId &&
            expr_builder.get_node(current_id).op == ExprGraph::Op::kSequence) {
          sequence_id = current_id;
        }
        if (sequence_id != kInvalidExprId) {
          return expr_builder.create_sequence(sequence_id, expr_id);
        }
        const SignalType signal_type = get_signal_type(*lhs.type, context.diagnostics);
        const ExprId sequence_base = expr_builder.find_or_create_input(
            extract_named_value(lhs, special_symbols), expr_builder.get_width(expr_id),
            expr_builder.get_sign(expr_id));
        return expr_builder.create_sequence(expr_id, sequence_base, signal_type.unpacked_dims,
                                            signal_type.width, signal_type.sign);
      }
      return finalize_packed_update(lhs, expr_id, base_id, [&](SignalWidth width, bool sign) {
        if (current_id != kInvalidExprId) {
          return current_id;
        }
        const std::string name = extract_named_value(lhs, special_symbols);
        return expr_builder.find_or_create_input(name, width, sign);
      });
    }
    if (lhs.kind == slang::ast::ExpressionKind::ElementSelect) {
      const auto &sel = lhs.as<slang::ast::ElementSelectExpression>();
      const ExprId index_id = build_expr(sel.selector(), expr_builder, context);
      slang::ConstantRange range;
      if (!get_range(*sel.value().type, range)) {
        return kInvalidExprId;
      }
      ExprId updated_expr_id = expr_id;
      ExprId updated_base_id = base_id;
      if (sel.value().type->isUnpackedArray()) {
        if (!lhs.type->isUnpackedArray()) {
          updated_expr_id =
              finalize_packed_update(lhs, updated_expr_id, updated_base_id, [&](SignalWidth, bool) {
                return build_expr(lhs, expr_builder, context);
              });
          updated_base_id = expr_builder.get_constant_zero();
        }
        SignalWidth width;
        bool sign;
        get_width_sign(*sel.value().type, width, sign, context.diagnostics);
        updated_expr_id = expr_builder.unpacked_assign_select(updated_expr_id, index_id, range.left,
                                                              range.right, width, sign);
      } else {
        SignalWidth selected_width;
        bool selected_sign;
        get_width_sign(*lhs.type, selected_width, selected_sign, context.diagnostics);
        const SignalWidth data_width = expr_width(sel.value());
        ExprId offset_id = expr_builder.normalize_index_expr(index_id, range.left, range.right);
        if (selected_width != 1) {
          const ExprId selected_width_id = expr_builder.find_or_create_const(
              std::to_string(data_width) + "'d" + std::to_string(selected_width), data_width,
              false);
          offset_id = expr_builder.create_mul(offset_id, selected_width_id);
        }
        updated_base_id = expr_builder.create_add(updated_base_id, offset_id);
      }
      return self(self, sel.value(), updated_expr_id, current_id, updated_base_id);
    }
    if (lhs.kind == slang::ast::ExpressionKind::RangeSelect) {
      const auto &sel = lhs.as<slang::ast::RangeSelectExpression>();
      slang::ConstantRange range;
      if (!get_range(*sel.value().type, range)) {
        return kInvalidExprId;
      }
      const auto kind = sel.getSelectionKind();
      ExprId updated_expr_id = expr_id;
      ExprId updated_base_id = base_id;
      if (sel.value().type->isUnpackedArray()) {
        SignalWidth width;
        bool sign;
        get_width_sign(*sel.value().type, width, sign, context.diagnostics);
        if (kind == slang::ast::RangeSelectionKind::Simple) {
          const auto left_index = try_extract_constant_index(sel.left());
          const auto right_index = try_extract_constant_index(sel.right());
          if (!left_index || !right_index) {
            context.diagnostics.error(
                DiagnosticId::kLoweringUnsupportedAssignmentIgnored,
                "unpacked range bounds are not representable integer constants");
            return kInvalidExprId;
          }
          updated_expr_id = expr_builder.unpacked_assign_range(
              expr_id, *left_index, *right_index, range.left, range.right, width, sign);
        } else if (kind == slang::ast::RangeSelectionKind::IndexedUp ||
                   kind == slang::ast::RangeSelectionKind::IndexedDown) {
          const auto slice_width_index = try_extract_constant_index(sel.right());
          if (!slice_width_index || *slice_width_index <= 0) {
            context.diagnostics.error(DiagnosticId::kLoweringUnsupportedAssignmentIgnored,
                                      "unpacked range width is not a positive integer constant");
            return kInvalidExprId;
          }
          const SignalWidth slice_width = static_cast<SignalWidth>(*slice_width_index);
          const ExprId base = build_expr(sel.left(), expr_builder, context);
          const bool dir = kind == slang::ast::RangeSelectionKind::IndexedUp;
          updated_expr_id = expr_builder.unpacked_assign_part_select(
              expr_id, base, slice_width, dir, range.left, range.right, width, sign);
        } else {
          context.diagnostics.error(DiagnosticId::kLoweringUnsupportedAssignmentIgnored,
                                    "unsupported unpacked range selection kind");
          return kInvalidExprId;
        }
      } else {
        if (kind == slang::ast::RangeSelectionKind::Simple) {
          const auto left_index = try_extract_constant_index(sel.left());
          const auto right_index = try_extract_constant_index(sel.right());
          if (!left_index || !right_index) {
            context.diagnostics.error(
                DiagnosticId::kLoweringUnsupportedAssignmentIgnored,
                "packed range bounds are not representable integer constants");
            return kInvalidExprId;
          }
          BitIndex left_pos = expr_builder.normalize_index(*left_index, range.left, range.right);
          BitIndex right_pos = expr_builder.normalize_index(*right_index, range.left, range.right);
          if (left_pos < right_pos) {
            updated_expr_id = expr_builder.create_reverse(updated_expr_id);
            std::swap(left_pos, right_pos);
          }
          assert(left_pos >= right_pos);
          const SignalWidth range_width = static_cast<SignalWidth>(left_pos - right_pos + 1);
          assert(expr_builder.get_width(updated_expr_id) == range_width);
          updated_base_id = expr_builder.create_add(
              updated_base_id,
              expr_builder.find_or_create_const(
                  right_pos, ExprBuilder::minimum_unsigned_width(right_pos), false));
        } else if (kind == slang::ast::RangeSelectionKind::IndexedUp ||
                   kind == slang::ast::RangeSelectionKind::IndexedDown) {
          const auto width_index = try_extract_constant_index(sel.right());
          if (!width_index || *width_index <= 0) {
            context.diagnostics.error(DiagnosticId::kLoweringUnsupportedAssignmentIgnored,
                                      "packed range width is not a positive integer constant");
            return kInvalidExprId;
          }
          const SignalWidth width = static_cast<SignalWidth>(*width_index);
          const ExprId index_id = build_expr(sel.left(), expr_builder, context);
          assert(expr_builder.get_width(updated_expr_id) == width);
          BitIndex index_offset = 0;
          if (width > 1 && kind == slang::ast::RangeSelectionKind::IndexedUp &&
              range.left < range.right) {
            index_offset = static_cast<BitIndex>(width - 1);
          } else if (width > 1 && kind == slang::ast::RangeSelectionKind::IndexedDown &&
                     range.left >= range.right) {
            index_offset = -static_cast<BitIndex>(width - 1);
          }
          const ExprId normalized_index =
              expr_builder.normalize_index_expr(index_id, range.left, range.right, index_offset);
          updated_base_id = expr_builder.create_add(updated_base_id, normalized_index);
        } else {
          context.diagnostics.error(DiagnosticId::kLoweringUnsupportedAssignmentIgnored,
                                    "unsupported packed range selection kind");
          return kInvalidExprId;
        }
      }
      return self(self, sel.value(), updated_expr_id, current_id, updated_base_id);
    }
    context.diagnostics.error(DiagnosticId::kLoweringUnsupportedAssignmentIgnored);
    return kInvalidExprId;
  };

  std::vector<const slang::ast::Expression *> lhs_stack{&whole_lhs};
  bool in_concat = false;
  while (!lhs_stack.empty()) {
    const slang::ast::Expression &lhs = *lhs_stack.back();
    lhs_stack.pop_back();
    if (lhs.kind == slang::ast::ExpressionKind::Concatenation) {
      const auto &cat = lhs.as<slang::ast::ConcatenationExpression>();
      for (auto it = cat.operands().rbegin(); it != cat.operands().rend(); ++it) {
        lhs_stack.push_back(*it);
      }
      in_concat = true;
      continue;
    }
    std::string output_name = extract_lhs_base_name(extract_lhs_base_name, lhs);
    if (output_name.empty()) {
      context.diagnostics.error(DiagnosticId::kLoweringUnsupportedAssignmentIgnored);
      continue;
    }
    ExprId expr_id = get_rhs_id(lhs);
    if (in_concat) {
      SignalWidth width;
      bool sign;
      get_width_sign(*lhs.type, width, sign, context.diagnostics);
      if (expr_builder.get_sign(expr_id) != sign) {
        expr_id = expr_builder.create_convert(expr_id, width, sign);
      }
    }
    ExprId saved_current_id = kInvalidExprId;
    bool restore_current = false;
    if (scheduled_assignments != nullptr) {
      auto it = scheduled_assignments->find(output_name);
      if (it != scheduled_assignments->end()) {
        saved_current_id = expr_builder.get_current_value(output_name);
        expr_builder.update_value(output_name, it->second);
        restore_current = true;
      }
    }
    const ExprId current_id = expr_builder.get_current_value(output_name);
    expr_id = assign_rec(assign_rec, lhs, expr_id, current_id, expr_builder.get_constant_zero());
    if (expr_id == kInvalidExprId) {
      if (restore_current) {
        expr_builder.update_value(output_name, saved_current_id);
      }
      continue;
    }
    if (restore_current) {
      expr_builder.update_value(output_name, saved_current_id);
    }
    record(output_name, expr_id);
  }
  if (remaining != 0) {
    context.diagnostics.error(DiagnosticId::kLoweringUnsupportedAssignmentIgnored,
                              std::to_string(remaining) + " RHS bits were not consumed");
  }
}

} // namespace abys::frontend
