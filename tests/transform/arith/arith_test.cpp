#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "catch_amalgamated.hpp"

#include "abys/frontend/api.h"
#include "abys/infra/naming.h"
#include "abys/ir/tig_dumper.h"
#include "abys/ir/tig_transformer.h"
#include "abys/liberty/library.h"
#include "abys/liberty/parser.h"
#include "abys/mapping/library.h"

TEST_CASE("map arithmetic logic", "[transform]") {
  const std::filesystem::path fixture_root = ABYS_ARITH_FIXTURE_DIR;
  for (const auto &entry : std::filesystem::recursive_directory_iterator(fixture_root)) {
    if (!entry.is_regular_file() || entry.path().filename() != "input.sv") {
      continue;
    }
    const auto fixture_dir = entry.path().parent_path();
    DYNAMIC_SECTION(fixture_dir.lexically_relative(fixture_root).string()) {
      std::ifstream source_file(entry.path());
      std::ifstream library_file(fixture_dir / "library.lib");
      REQUIRE(source_file.is_open());
      REQUIRE(library_file.is_open());
      const std::string source{std::istreambuf_iterator<char>(source_file), {}};
      const std::string library_source{std::istreambuf_iterator<char>(library_file), {}};

      std::ostringstream diagnostic_output;
      abys::Diagnostics diagnostics(diagnostic_output);
      abys::NamingOptions naming;
      auto ast = abys::liberty::parse(library_source, diagnostics, "library.lib");
      REQUIRE(ast.has_value());
      auto cell_library =
          abys::liberty::build_cell_library(std::move(*ast), diagnostics, "library.lib");
      REQUIRE(cell_library.has_value());
      auto mapping_library = std::make_shared<abys::mapping::Library>(*cell_library, diagnostics);

      auto result =
          abys::build_tig_from_systemverilog_text(source, "arith.sv", "top", diagnostics, naming);
      REQUIRE(result.ok);
      abys::ir::TigTransformer transformer(result.design, diagnostics, naming);
      std::shared_ptr<const boop::CellLibrary> cells(mapping_library, &mapping_library->cells());
      REQUIRE(transformer.arith(std::move(cells)));

      std::ostringstream output;
      abys::ir::TigDumper(result.design, diagnostics, naming).dump(output);
      std::ifstream expected_file(fixture_dir / "expected.sv");
      REQUIRE(expected_file.is_open());
      const std::string expected{std::istreambuf_iterator<char>(expected_file), {}};
      CHECK(output.str() == expected);
    }
  }
}
