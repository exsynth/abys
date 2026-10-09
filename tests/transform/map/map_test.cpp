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

namespace {

void check_fixture(const std::filesystem::path &fixture_dir, bool bus,
                   const std::filesystem::path &expected_path) {
  std::ifstream source_file(fixture_dir / "input.sv");
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
      abys::build_tig_from_systemverilog_text(source, "map.sv", "top", diagnostics, naming);
  REQUIRE(result.ok);
  abys::ir::TigTransformer transformer(result.design, diagnostics, naming);
  transformer.aig(bus);
  std::shared_ptr<const boop::CellLibrary> cells(mapping_library, &mapping_library->cells());
  REQUIRE(transformer.map(std::move(cells)));

  std::ostringstream output;
  abys::ir::TigDumper(result.design, diagnostics, naming).dump(output);
  std::ifstream expected_file(expected_path);
  REQUIRE(expected_file.is_open());
  const std::string expected{std::istreambuf_iterator<char>(expected_file), {}};
  CHECK(output.str() == expected);
}

} // namespace

TEST_CASE("map and-inverter graphs", "[transform]") {
  const std::filesystem::path fixture_root = ABYS_MAP_FIXTURE_DIR;
  check_fixture(fixture_root / "basic", false, fixture_root / "basic" / "expected.sv");
  check_fixture(fixture_root / "bus", true, fixture_root / "bus" / "expected_bus.sv");
}
