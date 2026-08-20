#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

#include "catch_amalgamated.hpp"

#include "abys/frontend/api.h"
#include "abys/infra/naming.h"
#include "abys/ir/tig_dumper.h"
#include "abys/ir/tig_transformer.h"

TEST_CASE("infer memory across subroutine flattening", "[transform]") {
  const std::filesystem::path fixture_root = ABYS_INFER_MEMORY_FIXTURE_DIR;
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
      auto result = abys::build_tig_from_systemverilog_text(source, "infer_memory.sv", "top",
                                                            diagnostics, naming);
      REQUIRE(result.ok);

      abys::ir::TigTransformer transformer(result.design, diagnostics, naming);
      transformer.infer_memory();
      transformer.flatten_subroutine();
      transformer.infer_memory();

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
