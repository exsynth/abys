#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "catch_amalgamated.hpp"

#include "frontend/lower_sv.h"

namespace {

std::string read_file(const std::filesystem::path &path) {
  std::ifstream file(path);
  REQUIRE(file.is_open());
  return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

} // namespace

TEST_CASE("lower SystemVerilog to the expected TIG dump", "[frontend]") {
  std::vector<std::filesystem::path> tests;
  for (const auto &entry : std::filesystem::directory_iterator(ABYS_FRONTEND_FIXTURE_DIR)) {
    if (entry.is_directory() && std::filesystem::exists(entry.path() / "input.sv") &&
        std::filesystem::exists(entry.path() / "expected.sv")) {
      tests.push_back(entry.path());
    }
  }
  std::ranges::sort(tests);

  for (const auto &test : tests) {
    DYNAMIC_SECTION(test.filename().string()) {
      const auto input_path = test / "input.sv";
      const auto output = abys::test::lower_and_dump(read_file(input_path), input_path.string());
      CHECK(output == read_file(test / "expected.sv"));
    }
  }
}
