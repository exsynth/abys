#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

#include "catch_amalgamated.hpp"

#include "abys/frontend/api.h"
#include "abys/infra/naming.h"
#include "abys/ir/tig_dumper.h"
#include "abys/ir/tig_transformer.h"

TEST_CASE("flatten subroutines removes calls and definitions", "[transform]") {
  const std::string fixture_dir = ABYS_FLATTEN_SUBROUTINE_FIXTURE_DIR;
  std::ifstream input_file(fixture_dir + "/input.sv");
  REQUIRE(input_file.is_open());
  const std::string source{std::istreambuf_iterator<char>(input_file),
                           std::istreambuf_iterator<char>()};

  std::ostringstream diagnostic_output;
  abys::Diagnostics diagnostics(diagnostic_output);
  abys::NamingOptions naming;
  auto result = abys::build_tig_from_systemverilog_text(
      source, "flatten_subroutine.sv", "top", diagnostics, naming);
  REQUIRE(result.ok);
  REQUIRE_FALSE(result.design.subroutines.empty());

  abys::ir::TigTransformer(result.design, diagnostics).flatten_subroutine();

  CHECK(result.design.subroutines.empty());
  for (const auto &module : result.design.modules) {
    for (const auto &node : module.nodes) {
      CHECK(node.expr_graph.calls.empty());
      CHECK(std::ranges::none_of(node.expr_graph.nodes, [](const auto &expr) {
        return expr.op == abys::ir::ExprGraph::Op::kCall;
      }));
    }
  }

  std::ostringstream output;
  abys::ir::TigDumper(result.design, diagnostics, naming).dump(output);
  std::ifstream expected_file(fixture_dir + "/expected.sv");
  REQUIRE(expected_file.is_open());
  const std::string expected{std::istreambuf_iterator<char>(expected_file),
                             std::istreambuf_iterator<char>()};
  CHECK(output.str() == expected);
}
