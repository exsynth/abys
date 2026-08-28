#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "catch_amalgamated.hpp"

#include "abys/frontend/api.h"
#include "abys/infra/naming.h"
#include "abys/ir/expr_builder.h"
#include "abys/ir/tig_dumper.h"
#include "abys/ir/tig_transformer.h"

TEST_CASE("expand dynamic shifts and indexed accesses", "[transform]") {
  const std::filesystem::path fixture_root = ABYS_DECOMPOSE_DYNAMIC_ACCESS_FIXTURE_DIR;
  std::vector<std::filesystem::path> fixture_dirs;
  for (const auto &entry : std::filesystem::recursive_directory_iterator(fixture_root)) {
    if (entry.is_regular_file() && entry.path().filename() == "input.sv") {
      fixture_dirs.push_back(entry.path().parent_path());
    }
  }
  std::ranges::sort(fixture_dirs);

  for (const auto &fixture_dir : fixture_dirs) {
    DYNAMIC_SECTION(fixture_dir.lexically_relative(fixture_root).string()) {
      std::ifstream input_file(fixture_dir / "input.sv");
      REQUIRE(input_file.is_open());
      const std::string source{std::istreambuf_iterator<char>(input_file),
                               std::istreambuf_iterator<char>()};

      std::ostringstream diagnostic_output;
      abys::Diagnostics diagnostics(diagnostic_output);
      abys::NamingOptions naming;
      auto result = abys::build_tig_from_systemverilog_text(source, "decompose_dynamic_access.sv",
                                                            "top", diagnostics, naming);
      REQUIRE(result.ok);

      abys::ir::TigTransformer(result.design, diagnostics, naming).decompose_dynamic_access();

      if (fixture_dir.filename() == "shift") {
        for (const auto &module : result.design.modules) {
          for (const auto &node : module.nodes) {
            CHECK(std::ranges::none_of(node.expr_graph.nodes, [](const auto &expr) {
              return expr.op == abys::ir::ExprGraph::Op::kShl ||
                     expr.op == abys::ir::ExprGraph::Op::kShr ||
                     expr.op == abys::ir::ExprGraph::Op::kAshr;
            }));
          }
        }
      } else if (fixture_dir.filename().string().starts_with("packed")) {
        for (auto &module : result.design.modules) {
          for (auto &node : module.nodes) {
            abys::ir::ExprBuilder builder(node.expr_graph, diagnostics);
            for (const auto &expr : node.expr_graph.nodes) {
              CHECK(expr.op != abys::ir::ExprGraph::Op::kMaskedAssign);
              if (expr.op == abys::ir::ExprGraph::Op::kRange) {
                REQUIRE(expr.operands.size() == 2);
                CHECK(builder.try_evaluate(expr.operands[1]).has_value());
              }
            }
          }
        }
        if (fixture_dir.filename() == "packed") {
          CHECK(diagnostic_output.str().find(
                    "warning: affine index materialized before dynamic access decomposition") !=
                std::string::npos);
        }
      } else if (fixture_dir.filename().string().starts_with("unpacked")) {
        for (auto &module : result.design.modules) {
          for (auto &node : module.nodes) {
            abys::ir::ExprBuilder builder(node.expr_graph, diagnostics);
            for (const auto &expr : node.expr_graph.nodes) {
              CHECK(expr.op != abys::ir::ExprGraph::Op::kSequence);
              CHECK(expr.op != abys::ir::ExprGraph::Op::kUnpackedAssign);
              CHECK(expr.op != abys::ir::ExprGraph::Op::kUnpackedRangeAssign);
              if (expr.op == abys::ir::ExprGraph::Op::kUnpackedSelect) {
                REQUIRE(expr.operands.size() == 2);
                CHECK(builder.try_evaluate(expr.operands[1]).has_value());
              }
            }
          }
        }
      }

      std::ostringstream output;
      abys::ir::TigDumper(result.design, diagnostics, naming).dump(output);
      std::ifstream expected_file(fixture_dir / "expected.sv");
      REQUIRE(expected_file.is_open());
      const std::string expected{std::istreambuf_iterator<char>(expected_file),
                                 std::istreambuf_iterator<char>()};
      CHECK(output.str() == expected);
    }
  }
}
