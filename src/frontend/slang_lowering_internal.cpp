#include "slang_lowering_internal.h"

namespace abys::frontend {

std::pair<SubrId, bool>
SlangLoweringContext::get_or_create_subr_id(const slang::ast::SubroutineSymbol &symbol) {
  const auto it = subr_ids.find(&symbol);
  if (it != subr_ids.end()) {
    return {it->second, false};
  }
  if (subr_ids.size() >= kInvalidSubrId) {
    diagnostics.error(DiagnosticId::kLoweringSubroutineLimitExceeded);
    return {kInvalidSubrId, false};
  }
  const SubrId id = static_cast<SubrId>(subr_ids.size());
  subr_ids.emplace(&symbol, id);
  return {id, true};
}

void SlangLoweringContext::mark_subroutine_unsupported(const slang::ast::SubroutineSymbol &symbol) {
  subr_ids.at(&symbol) = kInvalidSubrId;
}

const char *definition_kind_to_string(slang::ast::DefinitionKind kind) {
  switch (kind) {
  case slang::ast::DefinitionKind::Module:
    return "Module";
  case slang::ast::DefinitionKind::Interface:
    return "Interface";
  case slang::ast::DefinitionKind::Program:
    return "Program";
  default:
    return "Unknown";
  }
}

abys::ir::SignalWidth expr_width(const slang::ast::Expression &expr) {
  return expr.type->getBitstreamWidth();
}

bool expr_sign(const slang::ast::Expression &expr) {
  return expr.type->isSigned();
}

std::string make_verilog_identifier(std::string_view name) {
  auto is_head = [](unsigned char character) {
    return std::isalpha(character) != 0 || character == '_';
  };
  auto is_body = [](unsigned char character) {
    return std::isalnum(character) != 0 || character == '_' || character == '$';
  };
  if (!name.empty() && is_head(static_cast<unsigned char>(name.front()))) {
    bool simple = true;
    for (char character : name) {
      if (!is_body(static_cast<unsigned char>(character))) {
        simple = false;
        break;
      }
    }
    if (simple) {
      return std::string(name);
    }
  }

  return "\\" + std::string(name) + " ";
}

std::string
lower_symbol_name(const slang::ast::Symbol &symbol,
                  std::unordered_map<const slang::ast::Symbol *, std::string> &special_symbols) {
  auto it = special_symbols.find(&symbol);
  if (it != special_symbols.end()) {
    return it->second;
  }
  return make_verilog_identifier(symbol.name);
}

std::string
register_symbol_name(const slang::ast::Symbol &symbol,
                     std::unordered_map<const slang::ast::Symbol *, std::string> &special_symbols,
                     std::string_view suffix) {
  std::string name = std::string(symbol.name);
  name += suffix;
  name = make_verilog_identifier(name);
  special_symbols[&symbol] = name;
  return name;
}

std::string
extract_named_value(const slang::ast::Expression &expr,
                    std::unordered_map<const slang::ast::Symbol *, std::string> &special_symbols) {
  assert(expr.kind == slang::ast::ExpressionKind::NamedValue);
  const auto &named = expr.as<slang::ast::NamedValueExpression>();
  return lower_symbol_name(named.symbol, special_symbols);
}

std::optional<BitIndex> try_extract_constant_index(const slang::ast::Expression &expr) {
  const auto *const constant_value = expr.getConstant();
  if (!constant_value || !*constant_value || !constant_value->isInteger()) {
    return std::nullopt;
  }
  return constant_value->integer().as<int64_t>();
}

void get_width_sign(const slang::ast::Type &type, SignalWidth &width, bool &sign,
                    Diagnostics &diagnostics) {
  if (type.isUnpackedArray()) {
    const auto &ct = type.getCanonicalType();
    if (ct.kind != slang::ast::SymbolKind::FixedSizeUnpackedArrayType) {
      diagnostics.error(DiagnosticId::kLoweringUnsupportedTypeReplacedWithBit);
      width = 1;
      sign = false;
      return;
    }
    const auto &arr = ct.as<slang::ast::FixedSizeUnpackedArrayType>();
    const auto range = arr.range;
    width = (range.left >= range.right) ? (range.left - range.right + 1)
                                        : (range.right - range.left + 1);
    sign = false;
  } else {
    width = type.getBitstreamWidth();
    sign = type.isSigned();
  }
}

SignalType get_signal_type(const slang::ast::Type &type, Diagnostics &diagnostics) {
  SignalType signal_type;
  const slang::ast::Type *element_type = &type.getCanonicalType();
  while (element_type->kind == slang::ast::SymbolKind::FixedSizeUnpackedArrayType) {
    const auto &array_type = element_type->as<slang::ast::FixedSizeUnpackedArrayType>();
    signal_type.unpacked_dims.push_back(array_type.range.width());
    element_type = &array_type.elementType.getCanonicalType();
  }
  if (element_type->isUnpackedArray()) {
    diagnostics.error(DiagnosticId::kLoweringUnsupportedTypeReplacedWithBit);
    return {{}, 1, false};
  }
  signal_type.width = element_type->getBitstreamWidth();
  signal_type.sign = element_type->isSigned();
  return signal_type;
}

static bool unpacked_array_directions_require_alignment(const slang::ast::Type &source_type,
                                                        const slang::ast::Type &target_type) {
  const auto &source = source_type.getCanonicalType();
  const auto &target = target_type.getCanonicalType();
  const bool source_is_array = source.kind == slang::ast::SymbolKind::FixedSizeUnpackedArrayType;
  const bool target_is_array = target.kind == slang::ast::SymbolKind::FixedSizeUnpackedArrayType;
  if (!source_is_array || !target_is_array) {
    return false;
  }
  const auto &source_array = source.as<slang::ast::FixedSizeUnpackedArrayType>();
  const auto &target_array = target.as<slang::ast::FixedSizeUnpackedArrayType>();
  const bool source_ascending = source_array.range.left < source_array.range.right;
  const bool target_ascending = target_array.range.left < target_array.range.right;
  return source_ascending != target_ascending ||
         unpacked_array_directions_require_alignment(source_array.elementType,
                                                     target_array.elementType);
}

ExprId align_unpacked_array_directions(ExprId value, const slang::ast::Type &source_type,
                                       const slang::ast::Type &target_type,
                                       ExprBuilder &expr_builder, Diagnostics &diagnostics) {
  if (!unpacked_array_directions_require_alignment(source_type, target_type)) {
    return value;
  }

  const auto &source = source_type.getCanonicalType();
  const auto &target = target_type.getCanonicalType();
  assert(source.kind == slang::ast::SymbolKind::FixedSizeUnpackedArrayType);
  assert(target.kind == slang::ast::SymbolKind::FixedSizeUnpackedArrayType);
  const auto &source_array = source.as<slang::ast::FixedSizeUnpackedArrayType>();
  const auto &target_array = target.as<slang::ast::FixedSizeUnpackedArrayType>();
  assert(source_array.range.width() == target_array.range.width());

  const bool source_ascending = source_array.range.left < source_array.range.right;
  const bool target_ascending = target_array.range.left < target_array.range.right;
  if (source_ascending != target_ascending) {
    value = expr_builder.create_reverse(value);
  }
  if (!unpacked_array_directions_require_alignment(source_array.elementType,
                                                   target_array.elementType)) {
    return value;
  }

  const SignalWidth size = static_cast<SignalWidth>(source_array.range.width());
  const SignalType element_type = get_signal_type(source_array.elementType, diagnostics);
  SignalWidth element_width;
  bool element_sign;
  get_width_sign(source_array.elementType, element_width, element_sign, diagnostics);
  std::vector<ExprId> elements;
  elements.reserve(size);
  for (SignalWidth i = 0; i < size; ++i) {
    const ExprId index =
        expr_builder.find_or_create_const(i, ExprBuilder::minimum_unsigned_width(i), false);
    ExprId element = expr_builder.create_unpacked_select(
        value, index, 0, static_cast<BitIndex>(size - 1), element_width, element_sign,
        element_type.unpacked_dims, element_type.width, element_type.sign);
    element = align_unpacked_array_directions(element, source_array.elementType,
                                              target_array.elementType, expr_builder, diagnostics);
    elements.push_back(element);
  }
  const SignalType result_type = get_signal_type(target_type, diagnostics);
  return expr_builder.create_gather(std::move(elements), result_type.unpacked_dims,
                                    result_type.width, result_type.sign);
}

} // namespace abys::frontend
