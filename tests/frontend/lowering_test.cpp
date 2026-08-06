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

TEST_CASE("initial and final statement-block locals remain module signals", "[frontend]") {
  constexpr std::string_view source = R"(
module top;
  initial begin : init_scope
    logic initial_local;
    initial_local = 1'b0;
  end

  final begin : final_scope
    logic final_local;
    final_local = 1'b1;
  end
endmodule
)";

  std::ostringstream diagnostic_output;
  abys::Diagnostics diagnostics(diagnostic_output);
  abys::NamingOptions naming;
  const auto result = abys::build_tig_from_systemverilog_text(
      source, "non_always_statement_blocks.sv", "top", diagnostics, naming);
  REQUIRE(result.ok);
  REQUIRE(result.design.modules.size() == 1);

  const auto &signals = result.design.modules.front().signals;
  CHECK(std::ranges::any_of(
      signals, [](const auto &signal) { return signal.name == "initial_local_abys_init_scope"; }));
  CHECK(std::ranges::any_of(
      signals, [](const auto &signal) { return signal.name == "final_local_abys_final_scope"; }));
}
