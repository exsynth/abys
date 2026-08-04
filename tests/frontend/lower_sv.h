#pragma once

#include <sstream>
#include <string>
#include <string_view>

#include "catch_amalgamated.hpp"

#include "abys/frontend/api.h"
#include "abys/infra/naming.h"
#include "abys/ir/tig_dumper.h"

namespace abys::test {

inline std::string lower_and_dump(std::string_view source,
                                  std::string_view source_name = "test.sv") {
  std::ostringstream diagnostic_output;
  Diagnostics diagnostics(diagnostic_output);
  NamingOptions naming;
  const auto result =
      build_tig_from_systemverilog_text(source, source_name, "top", diagnostics, naming);
  INFO(diagnostic_output.str());
  REQUIRE(result.ok);
  REQUIRE(diagnostic_output.str().empty());

  std::ostringstream output;
  ir::TigDumper(result.design, diagnostics, naming).dump(output);
  REQUIRE(diagnostic_output.str().empty());
  return output.str();
}

} // namespace abys::test
