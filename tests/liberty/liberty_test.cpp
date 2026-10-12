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
#include "abys/liberty/writer.h"

namespace {

std::string read_file(const std::filesystem::path &path) {
  std::ifstream file(path);
  REQUIRE(file.is_open());
  return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

} // namespace

TEST_CASE("read and write Liberty libraries", "[liberty]") {
  const std::filesystem::path fixture_root = ABYS_LIBERTY_FIXTURE_DIR;
  std::vector<std::filesystem::path> fixture_dirs;
  for (const auto &entry : std::filesystem::recursive_directory_iterator(fixture_root)) {
    if (entry.is_regular_file() && entry.path().filename() == "input.lib") {
      fixture_dirs.push_back(entry.path().parent_path());
    }
  }
  std::ranges::sort(fixture_dirs);

  for (const auto &fixture_dir : fixture_dirs) {
    DYNAMIC_SECTION(fixture_dir.lexically_relative(fixture_root).string()) {
      const auto input_path = fixture_dir / "input.lib";
      std::ostringstream diagnostic_output;
      abys::Diagnostics diagnostics(diagnostic_output);
      auto ast = abys::liberty::parse(read_file(input_path), diagnostics, input_path.string());
      REQUIRE(ast.has_value());
      auto library =
          abys::liberty::build_cell_library(std::move(*ast), diagnostics, input_path.string());
      REQUIRE(library.has_value());
      CHECK(diagnostic_output.str().empty());

      std::ostringstream output;
      abys::liberty::write(*library, output);
      const std::string expected = read_file(fixture_dir / "expected.lib");
      const size_t library_begin = expected.find("library (");
      REQUIRE(library_begin != std::string::npos);
      CHECK(output.str() == expected.substr(library_begin));
    }
  }
}
