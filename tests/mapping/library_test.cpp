#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "catch_amalgamated.hpp"

#include "abys/infra/diagnostics.h"
#include "abys/liberty/library.h"
#include "abys/liberty/parser.h"
#include "abys/mapping/library.h"

namespace {

std::string read_file(const std::filesystem::path &path) {
  std::ifstream file(path);
  REQUIRE(file.is_open());
  return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

} // namespace

TEST_CASE("construct mapping libraries from Liberty fixtures", "[mapping]") {
  const std::filesystem::path fixture_root = ABYS_LIBRARY_FIXTURE_DIR;
  std::vector<std::filesystem::path> input_paths;
  for (const auto &entry : std::filesystem::directory_iterator(fixture_root)) {
    if (entry.is_directory()) {
      const std::filesystem::path name = entry.path().filename();
      input_paths.push_back(entry.path() / (name.string() + ".lib"));
    }
  }
  std::ranges::sort(input_paths);

  for (const auto &input_path : input_paths) {
    DYNAMIC_SECTION(input_path.parent_path().lexically_relative(fixture_root).string()) {
      std::ostringstream diagnostic_output;
      abys::Diagnostics diagnostics(diagnostic_output);
      auto ast = abys::liberty::parse(read_file(input_path), diagnostics, input_path.string());
      REQUIRE(ast.has_value());
      auto liberty_library =
          abys::liberty::build_cell_library(std::move(*ast), diagnostics, input_path.string());
      REQUIRE(liberty_library.has_value());

      abys::mapping::Library library(*liberty_library, diagnostics);
      CHECK(library.cell_count() > 0);

      std::ostringstream output;
      library.write_genlib(output);
      const std::string expected = read_file(input_path.parent_path() / "expected.genlib");
      const size_t genlib_begin = expected.find("GATE ");
      REQUIRE(genlib_begin != std::string::npos);
      CHECK(output.str() == expected.substr(genlib_begin));
    }
  }
}
