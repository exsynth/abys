#include "slang_lowering_internal.h"

namespace abys::frontend {

SubrId SlangLoweringContext::get_or_create_subr_id(const slang::ast::SubroutineSymbol &symbol) {
  const auto it = subr_ids.find(&symbol);
  if (it != subr_ids.end()) {
    return it->second;
  }
  if (subr_ids.size() >= kInvalidSubrId) {
    diagnostics.error(DiagnosticId::kLoweringSubroutineLimitExceeded);
    return kInvalidSubrId;
  }
  const SubrId id = static_cast<SubrId>(subr_ids.size());
  subr_ids.emplace(&symbol, id);
  return id;
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

} // namespace abys::frontend
