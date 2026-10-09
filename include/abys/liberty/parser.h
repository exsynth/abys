#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include "abys/infra/diagnostics.h"
#include "abys/liberty/ast.h"

namespace abys::liberty {

std::optional<Ast> parse(std::string_view source, Diagnostics &diagnostics,
                         std::string_view source_name = {});

} // namespace abys::liberty
