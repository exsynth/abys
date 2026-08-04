#pragma once

#include <string_view>

#include "abys/infra/diagnostics.h"
#include "abys/infra/naming.h"
#include "abys/ir/tig.h"

namespace slang::ast {
class RootSymbol;
}

namespace abys::frontend {

struct PragmaMap;

ir::Tig lower_slang_ast_to_ir(const slang::ast::RootSymbol &root, Diagnostics &diagnostics,
                              const PragmaMap &pragmas, std::string_view top,
                              const NamingOptions &naming);

} // namespace abys::frontend
