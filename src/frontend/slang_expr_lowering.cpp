#include "slang/ast/EvalContext.h"
#include "slang_lowering_internal.h"

namespace abys::frontend {

class SlangExprLoweringVisitor final
    : public slang::ast::ASTVisitor<SlangExprLoweringVisitor,
                                    slang::ast::VisitFlags::Expressions |
                                        slang::ast::VisitFlags::Canonical> {
private:
  ExprBuilder &builder_;
  SlangLoweringContext &context_;
  ExprId compound_lhs_id_;
  std::vector<ExprId> expr_stack_;

  ExprId create_filled(const slang::ast::Type &type, char bit) {
    const auto &canonical_type = type.getCanonicalType();
    if (canonical_type.kind == slang::ast::SymbolKind::FixedSizeUnpackedArrayType) {
      const auto &array_type = canonical_type.as<slang::ast::FixedSizeUnpackedArrayType>();
      std::vector<ExprId> elements(array_type.range.width());
      for (ExprId &element : elements) {
        element = create_filled(array_type.elementType, bit);
      }
      const SignalType signal_type = get_signal_type(type, context_.diagnostics);
      return builder_.create_gather(std::move(elements), signal_type.unpacked_dims,
                                    signal_type.width, signal_type.sign);
    }
    const SignalWidth width = type.getBitstreamWidth();
    return builder_.find_or_create_const(std::to_string(width) + "'b" + std::string(width, bit),
                                         width, type.isSigned());
  }

  ExprId create_zero(const slang::ast::Type &type) { return create_filled(type, '0'); }

public:
  ExprId create_unknown(const slang::ast::Type &type) { return create_filled(type, 'x'); }

private:
  void push_zero(const slang::ast::Expression &expr) {
    expr_stack_.push_back(create_zero(*expr.type));
  }

  void replace_with_zero(const slang::ast::Expression &expr, std::string detail) {
    if (context_.current_subroutine) {
      context_.current_subroutine_unsupported = true;
    } else {
      context_.diagnostics.error(DiagnosticId::kLoweringUnsupportedExpressionReplacedWithZero,
                                 std::move(detail));
    }
    push_zero(expr);
  }

  bool try_lower_integer_constant(const slang::ast::Expression &expr) {
    const auto *const value = expr.getConstant();
    if (!value || !*value || !value->isInteger()) {
      return false;
    }
    expr_stack_.push_back(builder_.find_or_create_const(
        value->integer().toString(slang::LiteralBase::Binary), expr_width(expr), expr_sign(expr)));
    return true;
  }

  std::optional<ExprId> lower_constant_value(const slang::ConstantValue &value,
                                             const slang::ast::Type &type) {
    if (value.isInteger()) {
      return builder_.find_or_create_const(value.integer().toString(slang::LiteralBase::Binary),
                                           type.getBitstreamWidth(), type.isSigned());
    }
    const auto &canonical_type = type.getCanonicalType();
    if (!value.isUnpacked() ||
        canonical_type.kind != slang::ast::SymbolKind::FixedSizeUnpackedArrayType) {
      return std::nullopt;
    }
    const auto &array_type = canonical_type.as<slang::ast::FixedSizeUnpackedArrayType>();
    std::vector<ExprId> elements;
    elements.reserve(value.elements().size());
    for (const auto &element : value.elements()) {
      const auto id = lower_constant_value(element, array_type.elementType);
      if (!id) {
        return std::nullopt;
      }
      elements.push_back(*id);
    }
    const SignalType signal_type = get_signal_type(type, context_.diagnostics);
    return builder_.create_gather(std::move(elements), signal_type.unpacked_dims, signal_type.width,
                                  signal_type.sign);
  }

  void lower_assignment_pattern(const slang::ast::AssignmentPatternExpressionBase &expr) {
    std::vector<ExprId> operands;
    operands.reserve(expr.elements().size());
    for (const auto *element : expr.elements()) {
      operands.push_back(build_expr(*element, builder_, context_));
    }
    if (expr.type->isUnpackedArray()) {
      const SignalType signal_type = get_signal_type(*expr.type, context_.diagnostics);
      expr_stack_.push_back(builder_.create_gather(std::move(operands), signal_type.unpacked_dims,
                                                   signal_type.width, signal_type.sign));
    } else {
      expr_stack_.push_back(builder_.create_concat(std::move(operands), expr_sign(expr)));
    }
  }

public:
  explicit SlangExprLoweringVisitor(ExprBuilder &builder, SlangLoweringContext &context,
                                    ExprId compound_lhs_id)
      : builder_(builder), context_(context), compound_lhs_id_(compound_lhs_id) {}

  template <typename T> void handle(const T &node) {
    if constexpr (std::is_base_of_v<slang::ast::Expression, T>) {
      replace_with_zero(node, typeid(T).name());
    } else {
      context_.diagnostics.error(DiagnosticId::kLoweringUnsupportedAstNode, typeid(T).name());
    }
  }

  void handle(const slang::ast::LValueReferenceExpression &expr) {
    if (compound_lhs_id_ == kInvalidExprId) {
      replace_with_zero(expr, "lvalue reference outside compound assignment");
      return;
    }
    expr_stack_.push_back(compound_lhs_id_);
  }

  void handle(const slang::ast::ConversionExpression &expr) {
    this->visitDefault(expr);
    ExprId operand = expr_stack_.back();
    expr_stack_.pop_back();
    ExprId id = operand;
    if (expr.conversionKind == slang::ast::ConversionKind::Propagated &&
        builder_.get_sign(id) != expr_sign(expr)) {
      id = builder_.create_convert(id, builder_.get_width(id), expr_sign(expr));
    }
    if (builder_.get_width(id) != expr_width(expr) || builder_.get_sign(id) != expr_sign(expr)) {
      id = builder_.create_convert(id, expr_width(expr), expr_sign(expr));
    }
    expr_stack_.push_back(id);
  }

  void handle(const slang::ast::NamedValueExpression &expr) {
    const auto &type = *expr.type;
    SignalWidth width;
    bool sign;
    get_width_sign(type, width, sign, context_.diagnostics);
    const slang::ConstantValue *constant_value = nullptr;
    if (expr.symbol.kind == slang::ast::SymbolKind::Parameter) {
      constant_value = &expr.symbol.as<slang::ast::ParameterSymbol>().getValue();
    } else if (expr.symbol.kind == slang::ast::SymbolKind::EnumValue) {
      constant_value = &expr.symbol.as<slang::ast::EnumValueSymbol>().getValue();
    }
    if (constant_value && *constant_value) {
      if (const auto id = lower_constant_value(*constant_value, type)) {
        expr_stack_.push_back(*id);
        return;
      }
    }
    ExprId id = builder_.find_or_create_input(
        lower_symbol_name(expr.symbol, context_.special_symbols), width, sign);
    expr_stack_.push_back(id);
  }

  void handle(const slang::ast::IntegerLiteral &expr) {
    const slang::SVInt v = expr.getValue();
    ExprId id = builder_.find_or_create_const(v.toString(slang::LiteralBase::Binary),
                                              expr_width(expr), expr_sign(expr));
    expr_stack_.push_back(id);
  }

  void handle(const slang::ast::StringLiteral &expr) {
    if (!try_lower_integer_constant(expr)) {
      replace_with_zero(expr, "string literal did not lower to integer constant");
    }
  }

  void handle(const slang::ast::UnbasedUnsizedIntegerLiteral &expr) {
    const slang::SVInt v = expr.getValue();
    ExprId id = builder_.find_or_create_const(v.toString(slang::LiteralBase::Binary),
                                              expr_width(expr), expr_sign(expr));
    expr_stack_.push_back(id);
  }

  void handle(const slang::ast::CallExpression &expr) {
    const slang::ast::SubroutineSymbol *subroutine = nullptr;
    if (const auto *found = std::get_if<const slang::ast::SubroutineSymbol *>(&expr.subroutine)) {
      subroutine = *found;
    }
    slang::ConstantValue evaluated;
    const slang::ConstantValue *value = expr.getConstant();
    if ((!value || !*value) && subroutine) {
      slang::ast::EvalContext eval_context(*subroutine);
      evaluated = expr.eval(eval_context);
      value = &evaluated;
    }
    if (value && *value) {
      if (const auto id = lower_constant_value(*value, *expr.type)) {
        expr_stack_.push_back(*id);
        return;
      }
    }
    if (expr.thisClass() != nullptr) {
      replace_with_zero(expr,
                        "unsupported class member call: " + std::string(expr.getSubroutineName()));
      return;
    }
    if (expr.isSystemCall()) {
      using slang::parsing::KnownSystemName;
      switch (expr.getKnownSystemName()) {
      case KnownSystemName::Signed:
      case KnownSystemName::Unsigned: {
        const auto arguments = expr.arguments();
        if (arguments.size() != 1) {
          replace_with_zero(expr,
                            "invalid argument count for " + std::string(expr.getSubroutineName()));
          return;
        }
        arguments.front()->visit(*this);
        ExprId operand = expr_stack_.back();
        expr_stack_.pop_back();
        expr_stack_.push_back(builder_.create_convert(operand, expr_width(expr), expr_sign(expr)));
        return;
      }
      case KnownSystemName::Bits: {
        const auto arguments = expr.arguments();
        if (arguments.size() != 1 || !arguments.front()->type->isFixedSize()) {
          replace_with_zero(expr, "unsupported $bits argument");
          return;
        }
        const uint64_t width = arguments.front()->type->getBitstreamWidth();
        const slang::SVInt value(32, width, true);
        expr_stack_.push_back(
            builder_.find_or_create_const(value.toString(slang::LiteralBase::Binary), 32, true));
        return;
      }
      case KnownSystemName::Clog2: {
        const auto arguments = expr.arguments();
        if (arguments.size() != 1) {
          replace_with_zero(expr, "invalid $clog2 argument count");
          return;
        }
        arguments.front()->visit(*this);
        const ExprId argument = expr_stack_.back();
        expr_stack_.pop_back();
        const auto value = builder_.try_evaluate(argument);
        if (!value || *value < 0) {
          replace_with_zero(expr, "non-constant $clog2 argument");
          return;
        }
        uint64_t remaining = static_cast<uint64_t>(*value);
        uint64_t result = 0;
        if (remaining > 1) {
          --remaining;
          while (remaining != 0) {
            ++result;
            remaining >>= 1;
          }
        }
        const slang::SVInt constant(32, result, true);
        expr_stack_.push_back(
            builder_.find_or_create_const(constant.toString(slang::LiteralBase::Binary), 32, true));
        return;
      }
      case KnownSystemName::FOpen:
      case KnownSystemName::FError:
      case KnownSystemName::FGets:
      case KnownSystemName::FScanf:
      case KnownSystemName::SScanf:
      case KnownSystemName::FRead:
      case KnownSystemName::FGetC:
      case KnownSystemName::UngetC:
      case KnownSystemName::FTell:
      case KnownSystemName::FSeek:
      case KnownSystemName::Rewind:
      case KnownSystemName::FEof:
      case KnownSystemName::TestPlusArgs:
      case KnownSystemName::ValuePlusArgs:
        context_.diagnostics.warning(DiagnosticId::kLoweringSystemFunctionReplacedWithZero,
                                     std::string(expr.getSubroutineName()));
        expr_stack_.push_back(builder_.find_or_create_const(
            std::to_string(expr_width(expr)) + "'b0", expr_width(expr), expr_sign(expr)));
        return;
      default:
        replace_with_zero(expr,
                          "unsupported system call: " + std::string(expr.getSubroutineName()));
        return;
      }
    }
    const SubrId subr_id = context_.get_or_create_subr_id(*subroutine).first;
    if (subr_id == kInvalidSubrId) {
      if (context_.current_subroutine) {
        context_.current_subroutine_unsupported = true;
      } else {
        context_.diagnostics.error(DiagnosticId::kLoweringUnsupportedExpressionReplacedWithZero,
                                   "call to unsupported subroutine: " +
                                       std::string(expr.getSubroutineName()));
      }
      push_zero(expr);
      return;
    }
    const size_t index = expr_stack_.size();
    this->visitDefault(expr);
    const size_t n = expr_stack_.size() - index;
    std::vector<ExprId> operands(n);
    for (size_t i = 0; i < n; ++i) {
      operands[n - 1 - i] = expr_stack_.back();
      expr_stack_.pop_back();
    }
    std::string name(expr.getSubroutineName());
    const ExprId id = builder_.create_call(subr_id, std::move(name), std::move(operands),
                                           expr_width(expr), expr_sign(expr));
    expr_stack_.push_back(id);
  }

  void handle(const slang::ast::ElementSelectExpression &expr) {
    if (try_lower_integer_constant(expr)) {
      return;
    }
    if (const auto key = try_get_packed_bit_key(expr)) {
      const auto it = context_.timing_bit_names.find(*key);
      if (it != context_.timing_bit_names.end()) {
        expr_stack_.push_back(builder_.find_or_create_input(it->second, 1, false));
        return;
      }
    }
    this->visitDefault(expr);
    const ExprId index = expr_stack_.back();
    expr_stack_.pop_back();
    const ExprId data = expr_stack_.back();
    expr_stack_.pop_back();
    const auto &type = *expr.value().type;
    if (type.isUnpackedArray()) {
      const auto &ct = type.getCanonicalType();
      if (ct.kind != slang::ast::SymbolKind::FixedSizeUnpackedArrayType) {
        replace_with_zero(expr, "unsupported dynamic-size unpacked array selection");
        return;
      }
      const auto &arr = ct.as<slang::ast::FixedSizeUnpackedArrayType>();
      const auto range = arr.range;
      const auto &elem = arr.elementType;
      SignalWidth width;
      bool sign;
      get_width_sign(elem, width, sign, context_.diagnostics);
      const SignalType signal_type = get_signal_type(elem, context_.diagnostics);
      ExprId id = builder_.create_unpacked_select(data, index, range.left, range.right, width, sign,
                                                  signal_type.unpacked_dims, signal_type.width,
                                                  signal_type.sign);
      expr_stack_.push_back(id);
    } else {
      const auto range = type.getFixedRange();
      const SignalWidth width = expr_width(expr);
      const SignalWidth data_width = expr_width(expr.value());
      if (data_width == width) {
        expr_stack_.push_back(data);
      } else if (width == 1) {
        const ExprId id = builder_.create_select(data, index, range.left, range.right);
        expr_stack_.push_back(id);
      } else {
        ExprId base = builder_.normalize_index_expr(index, range.left, range.right);
        const ExprId width_id = builder_.find_or_create_const(
            std::to_string(data_width) + "'d" + std::to_string(width), data_width, false);
        base = builder_.create_mul(base, width_id);
        expr_stack_.push_back(builder_.create_range(data, base, width, expr_sign(expr)));
      }
    }
  }

  void handle(const slang::ast::MemberAccessExpression &expr) {
    if (expr.member.kind != slang::ast::SymbolKind::Field || !expr.value().type->isIntegral()) {
      replace_with_zero(expr, "unsupported non-packed member access");
      return;
    }
    expr.value().visit(*this);
    const ExprId data = expr_stack_.back();
    expr_stack_.pop_back();
    const auto &field = expr.member.as<slang::ast::FieldSymbol>();
    const SignalWidth width = expr_width(expr);
    const SignalWidth data_width = expr_width(expr.value());
    if (width == data_width && field.bitOffset == 0) {
      expr_stack_.push_back(data);
      return;
    }
    const BitIndex right = static_cast<BitIndex>(field.bitOffset);
    const BitIndex left = right + static_cast<BitIndex>(width - 1);
    expr_stack_.push_back(
        builder_.create_simple_range(data, left, right, static_cast<BitIndex>(data_width - 1), 0));
  }

  void handle(const slang::ast::RangeSelectExpression &expr) {
    expr.value().visit(*this);
    const ExprId data = expr_stack_.back();
    expr_stack_.pop_back();
    const auto &type = *expr.value().type;
    const auto kind = expr.getSelectionKind();
    const auto &left = expr.left();
    const auto &right = expr.right();
    if (type.isUnpackedArray()) {
      const auto &ct = type.getCanonicalType();
      if (ct.kind != slang::ast::SymbolKind::FixedSizeUnpackedArrayType) {
        replace_with_zero(expr, "unsupported dynamic-size unpacked array range selection");
        return;
      }
      const auto &arr = ct.as<slang::ast::FixedSizeUnpackedArrayType>();
      const slang::ConstantRange range = arr.range;
      ExprId base;
      SignalWidth width;
      if (kind == slang::ast::RangeSelectionKind::Simple) {
        const auto left_index = try_extract_constant_index(left);
        const auto right_index = try_extract_constant_index(right);
        if (!left_index || !right_index) {
          replace_with_zero(expr, "unpacked range bounds are not representable integer constants");
          return;
        }
        const BitIndex left_pos = builder_.normalize_index(*left_index, range.left, range.right);
        const BitIndex right_pos = builder_.normalize_index(*right_index, range.left, range.right);
        assert(left_pos >= right_pos);
        base = builder_.find_or_create_const(right_pos,
                                             ExprBuilder::minimum_unsigned_width(right_pos), false);
        width = static_cast<SignalWidth>(left_pos - right_pos + 1);
      } else if (kind == slang::ast::RangeSelectionKind::IndexedUp ||
                 kind == slang::ast::RangeSelectionKind::IndexedDown) {
        const auto slice_width = try_extract_constant_index(right);
        if (!slice_width || *slice_width <= 0) {
          replace_with_zero(expr, "unpacked range width is not a positive integer constant");
          return;
        }
        width = static_cast<SignalWidth>(*slice_width);
        left.visit(*this);
        const ExprId index = expr_stack_.back();
        expr_stack_.pop_back();
        BitIndex index_offset = 0;
        if (width > 1 && kind == slang::ast::RangeSelectionKind::IndexedUp &&
            range.left < range.right) {
          index_offset = static_cast<BitIndex>(width - 1);
        } else if (width > 1 && kind == slang::ast::RangeSelectionKind::IndexedDown &&
                   range.left >= range.right) {
          index_offset = -static_cast<BitIndex>(width - 1);
        }
        base = builder_.normalize_index_expr(index, range.left, range.right, index_offset);
      } else {
        replace_with_zero(expr, "unsupported unpacked range selection kind");
        return;
      }
      if (width == builder_.get_width(data)) {
        const auto base_value = builder_.try_evaluate(base);
        if (base_value && *base_value == 0) {
          expr_stack_.push_back(data);
          return;
        }
      }
      expr_stack_.push_back(builder_.create_unpacked_range(data, base, width));
      return;
    }
    const slang::ConstantRange range = type.getFixedRange();
    if (kind == slang::ast::RangeSelectionKind::Simple) {
      const auto left_index = try_extract_constant_index(left);
      const auto right_index = try_extract_constant_index(right);
      if (!left_index || !right_index) {
        replace_with_zero(expr, "packed range bounds are not representable integer constants");
        return;
      }
      const BitIndex left_sw = *left_index;
      const BitIndex right_sw = *right_index;
      const BitIndex left_pos = builder_.normalize_index(left_sw, range.left, range.right);
      const BitIndex right_pos = builder_.normalize_index(right_sw, range.left, range.right);
      const SignalWidth data_width = builder_.get_width(data);
      const bool is_full_width =
          right_pos == 0 && left_pos == static_cast<BitIndex>(data_width - 1);
      if (is_full_width) {
        expr_stack_.push_back(data);
      } else {
        expr_stack_.push_back(
            builder_.create_simple_range(data, left_sw, right_sw, range.left, range.right));
      }
    } else if (kind == slang::ast::RangeSelectionKind::IndexedUp ||
               kind == slang::ast::RangeSelectionKind::IndexedDown) {
      left.visit(*this);
      const ExprId base = expr_stack_.back();
      expr_stack_.pop_back();
      const bool dir = (kind == slang::ast::RangeSelectionKind::IndexedUp);
      SignalWidth selected_width;
      bool selected_sign;
      get_width_sign(*expr.type, selected_width, selected_sign, context_.diagnostics);
      const SignalWidth data_width = builder_.get_width(data);
      BitIndex index_offset = 0;
      if (selected_width > 1 && dir && range.left < range.right) {
        index_offset = static_cast<BitIndex>(selected_width - 1);
      } else if (selected_width > 1 && !dir && range.left >= range.right) {
        index_offset = -static_cast<BitIndex>(selected_width - 1);
      }
      const ExprId pos = builder_.normalize_index_expr(base, range.left, range.right, index_offset);
      bool is_full_width = false;
      if (selected_width == data_width) {
        const auto pos_value = builder_.try_evaluate(pos);
        is_full_width = pos_value && *pos_value == 0;
      }
      if (is_full_width) {
        expr_stack_.push_back(data);
      } else {
        expr_stack_.push_back(builder_.create_range(data, pos, selected_width, selected_sign));
      }
    } else {
      replace_with_zero(expr, "unsupported packed range selection kind");
    }
  }

  void handle(const slang::ast::ConcatenationExpression &expr) {
    const size_t index = expr_stack_.size();
    for (const auto *operand : expr.operands()) {
      if (operand->kind == slang::ast::ExpressionKind::Replication) {
        const auto &replication = operand->as<slang::ast::ReplicationExpression>();
        const auto count = try_extract_constant_index(replication.count());
        if (count && *count == 0) {
          continue;
        }
      }
      operand->visit(*this);
    }
    const size_t n = expr_stack_.size() - index;
    if (n == 0) {
      push_zero(expr);
      return;
    }
    std::vector<ExprId> operands(n);
    for (size_t i = 0; i < n; ++i) {
      operands[n - 1 - i] = expr_stack_.back();
      expr_stack_.pop_back();
    }
    expr_stack_.push_back(builder_.create_concat(std::move(operands), expr_sign(expr)));
  }

  void handle(const slang::ast::ReplicationExpression &expr) {
    const auto rep = try_extract_constant_index(expr.count());
    if (!rep) {
      replace_with_zero(expr, "replication count is not a representable integer constant");
      return;
    }
    if (*rep < 0) {
      replace_with_zero(expr, "negative replication count");
      return;
    }
    if (*rep == 0) {
      push_zero(expr);
      return;
    }
    expr.concat().visit(*this);
    ExprId body = expr_stack_.back();
    expr_stack_.pop_back();
    std::vector<ExprId> operands;
    operands.reserve(*rep);
    for (size_t i = 0; i < static_cast<size_t>(*rep); ++i) {
      operands.push_back(body);
    }
    expr_stack_.push_back(builder_.create_concat(std::move(operands), expr_sign(expr)));
  }

  void handle(const slang::ast::ConditionalExpression &expr) {
    if (expr.conditions.size() != 1 || expr.conditions[0].pattern != nullptr) {
      replace_with_zero(expr, "unsupported conditional expression with pattern chain");
      return;
    }
    const size_t index = expr_stack_.size();
    this->visitDefault(expr);
    assert(expr_stack_.size() == index + 3);
    const ExprId else_id = expr_stack_.back();
    expr_stack_.pop_back();
    const ExprId then_id = expr_stack_.back();
    expr_stack_.pop_back();
    const ExprId cond = expr_stack_.back();
    expr_stack_.pop_back();
    expr_stack_.push_back(builder_.create_mux(cond, then_id, else_id));
  }

  void handle(const slang::ast::BinaryExpression &expr) {
    this->visitDefault(expr);
    assert(expr_stack_.size() >= 2);
    ExprId rhs = expr_stack_.back();
    expr_stack_.pop_back();
    ExprId lhs = expr_stack_.back();
    expr_stack_.pop_back();
    ExprId id;
    switch (expr.op) {
    case slang::ast::BinaryOperator::LogicalOr:
      id = builder_.create_logical_or(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::LogicalAnd:
      id = builder_.create_logical_and(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::BinaryAnd:
      id = builder_.create_and({lhs, rhs});
      break;
    case slang::ast::BinaryOperator::BinaryOr:
      id = builder_.create_or({lhs, rhs});
      break;
    case slang::ast::BinaryOperator::BinaryXor:
      id = builder_.create_xor({lhs, rhs});
      break;
    case slang::ast::BinaryOperator::Add:
      id = builder_.create_add(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::Subtract:
      id = builder_.create_sub(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::Multiply:
      id = builder_.create_mul(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::Divide:
      id = builder_.create_div(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::Mod:
      id = builder_.create_mod(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::Power:
      id = builder_.create_pow(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::LogicalShiftLeft:
    case slang::ast::BinaryOperator::ArithmeticShiftLeft:
      id = builder_.create_shl(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::LogicalShiftRight:
      id = builder_.create_shr(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::ArithmeticShiftRight:
      id = builder_.create_ashr(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::Equality:
    case slang::ast::BinaryOperator::CaseEquality:
      id = builder_.create_eq(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::Inequality:
    case slang::ast::BinaryOperator::CaseInequality:
      id = builder_.create_neq(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::LessThan:
      id = builder_.create_lt(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::LessThanEqual:
      id = builder_.create_le(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::GreaterThan:
      id = builder_.create_gt(lhs, rhs);
      break;
    case slang::ast::BinaryOperator::GreaterThanEqual:
      id = builder_.create_ge(lhs, rhs);
      break;
    default:
      replace_with_zero(expr, "unhandled binary operator: " +
                                  std::string(slang::ast::OpInfo::getText(expr.op)));
      return;
    }
    assert(id != kInvalidExprId);
    expr_stack_.push_back(id);
  }

  void handle(const slang::ast::UnaryExpression &expr) {
    this->visitDefault(expr);
    assert(!expr_stack_.empty());
    ExprId a = expr_stack_.back();
    expr_stack_.pop_back();
    ExprId id = kInvalidExprId;
    switch (expr.op) {
    case slang::ast::UnaryOperator::Plus:
      id = builder_.create_unary_plus(a);
      break;
    case slang::ast::UnaryOperator::Minus:
      id = builder_.create_unary_minus(a);
      break;
    case slang::ast::UnaryOperator::LogicalNot:
      id = builder_.create_logical_not(a);
      break;
    case slang::ast::UnaryOperator::BitwiseNot:
      id = builder_.create_bitwise_not(a);
      break;
    case slang::ast::UnaryOperator::BitwiseAnd:
      id = builder_.create_and_reduce(a);
      break;
    case slang::ast::UnaryOperator::BitwiseOr:
      id = builder_.create_or_reduce(a);
      break;
    case slang::ast::UnaryOperator::BitwiseXor:
      id = builder_.create_xor_reduce(a);
      break;
    case slang::ast::UnaryOperator::BitwiseNand:
      id = builder_.create_and_reduce(a);
      id = builder_.create_logical_not(id);
      break;
    case slang::ast::UnaryOperator::BitwiseNor:
      id = builder_.create_or_reduce(a);
      id = builder_.create_logical_not(id);
      break;
    case slang::ast::UnaryOperator::BitwiseXnor:
      id = builder_.create_xor_reduce(a);
      id = builder_.create_logical_not(id);
      break;
    case slang::ast::UnaryOperator::Preincrement:
    case slang::ast::UnaryOperator::Predecrement:
    case slang::ast::UnaryOperator::Postincrement:
    case slang::ast::UnaryOperator::Postdecrement:
      replace_with_zero(expr, "inc/dec unary operators are not yet supported");
      return;
    }
    assert(id != kInvalidExprId);
    expr_stack_.push_back(id);
  }

  void handle(const slang::ast::SimpleAssignmentPatternExpression &expr) {
    lower_assignment_pattern(expr);
  }

  void handle(const slang::ast::StructuredAssignmentPatternExpression &expr) {
    lower_assignment_pattern(expr);
  }

  ExprId get_root() {
    assert(!expr_stack_.empty());
    assert(expr_stack_.size() == 1);
    return expr_stack_.back();
  }
};

std::optional<SlangLoweringContext::PackedBitKey>
try_get_packed_bit_key(const slang::ast::Expression &expr) {
  if (expr.kind != slang::ast::ExpressionKind::ElementSelect) {
    return std::nullopt;
  }
  const auto &select = expr.as<slang::ast::ElementSelectExpression>();
  if (select.value().kind != slang::ast::ExpressionKind::NamedValue ||
      select.value().type->isUnpackedArray()) {
    return std::nullopt;
  }
  const auto index = try_extract_constant_index(select.selector());
  if (!index) {
    return std::nullopt;
  }
  const slang::ConstantRange range = select.value().type->getFixedRange();
  if (!range.containsPoint(*index)) {
    return std::nullopt;
  }
  const auto &named = select.value().as<slang::ast::NamedValueExpression>();
  return SlangLoweringContext::PackedBitKey{&named.symbol, range.translateIndex(*index)};
}

ExprId build_expr(const slang::ast::Expression &expr, ExprBuilder &expr_builder,
                  SlangLoweringContext &context, ExprId compound_lhs_id) {
  SlangExprLoweringVisitor expr_visitor(expr_builder, context, compound_lhs_id);
  expr.visit(expr_visitor);
  return expr_visitor.get_root();
}

ExprId build_unknown(const slang::ast::Type &type, ExprBuilder &expr_builder,
                     SlangLoweringContext &context) {
  SlangExprLoweringVisitor expr_visitor(expr_builder, context, kInvalidExprId);
  return expr_visitor.create_unknown(type);
}

} // namespace abys::frontend
