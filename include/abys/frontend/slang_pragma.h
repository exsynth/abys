#pragma once

#include <array>
#include <string_view>
#include <unordered_map>

#include "abys/infra/diagnostics.h"
#include "slang/driver/Driver.h"
#include "slang/syntax/SyntaxNode.h"

namespace abys::frontend {

inline constexpr std::array<std::string_view, 3> kSynthesisPragmaPrefixes = {
    "synopsys",
    "synthesis",
    "pragma",
};

struct PragmaInfo {
  bool full_case = false;
  bool parallel_case = false;
};

struct PragmaMap {
  std::unordered_map<const slang::syntax::SyntaxNode *, PragmaInfo> by_node;
};

PragmaMap collect_pragmas(slang::driver::Driver &driver, Diagnostics &diagnostics);

} // namespace abys::frontend
