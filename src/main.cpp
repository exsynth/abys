#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "abys/frontend/api.h"
#include "abys/ir/tig_dumper.h"
#include "abys/version.h"

namespace {

bool is_version_flag(std::string_view arg) {
  return arg == "-v" || arg == "--version";
}

bool is_help_flag(std::string_view arg) {
  return arg == "-h" || arg == "--help";
}

void print_help() {
  std::cout << "abys: logic synthesis toolchain (scaffold)\n";
  std::cout << "Usage:\n";
  std::cout << "  abys --version\n";
  std::cout << "  abys parse <files...> [--top <module>]\n";
  std::cout << "  abys emit <files...> [--top <module>] [--out <file>]\n";
}

} // namespace

int main(int argc, char **argv) {
  if (argc <= 1) {
    print_help();
    return 0;
  }

  for (int i = 1; i < argc; ++i) {
    if (is_version_flag(argv[i])) {
      std::cout << "abys " << abys::version() << '\n';
      return 0;
    }
    if (is_help_flag(argv[i])) {
      print_help();
      return 0;
    }
  }

  const std::string_view command = argv[1];
  if (command == "parse") {
    std::vector<std::string> files;
    std::string top;

    for (int i = 2; i < argc; ++i) {
      const std::string_view arg = argv[i];
      if (arg == "--top") {
        if (i + 1 >= argc) {
          std::cerr << "parse failed: --top requires a module name\n";
          return 2;
        }
        top = argv[++i];
        continue;
      }
      files.emplace_back(arg);
    }

    abys::Diagnostics diagnostics(std::cerr);
    const auto result = abys::parse_systemverilog(files, top, diagnostics);
    if (!result.ok) {
      std::cerr << "parse failed: " << result.message << '\n';
      return 2;
    }

    std::cout << "parse ok\n";
    return 0;
  }

  if (command == "emit") {
    std::vector<std::string> files;
    std::string top;
    std::optional<std::string> out_path;

    for (int i = 2; i < argc; ++i) {
      const std::string_view arg = argv[i];
      if (arg == "--top") {
        if (i + 1 >= argc) {
          std::cerr << "emit failed: --top requires a module name\n";
          return 2;
        }
        top = argv[++i];
        continue;
      }
      if (arg == "--out") {
        if (i + 1 >= argc) {
          std::cerr << "emit failed: --out requires a file path\n";
          return 2;
        }
        out_path = argv[++i];
        continue;
      }
      files.emplace_back(arg);
    }

    abys::Diagnostics diagnostics(std::cerr);
    abys::NamingOptions naming;
    const auto result = abys::build_tig_from_systemverilog(files, top, diagnostics, naming);
    if (!result.ok) {
      std::cerr << "emit failed: " << result.message << '\n';
      return 2;
    }

    abys::ir::TigDumper dumper(result.design, diagnostics, naming);
    if (out_path) {
      std::ofstream output_stream(*out_path);
      if (!output_stream) {
        std::cerr << "emit failed: cannot open output file: " << *out_path << '\n';
        return 3;
      }
      dumper.dump(output_stream);
    } else {
      dumper.dump(std::cout);
    }

    return 0;
  }

  print_help();
  return 1;
}
