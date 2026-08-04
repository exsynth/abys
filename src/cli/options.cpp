#include "options.h"

#include <string_view>

namespace abys::cli {
namespace {

void append_arguments(Options &options, int first, int argc, char **argv) {
  for (int i = first; i < argc; ++i) {
    options.arguments.emplace_back(argv[i]);
  }
}

} // namespace

bool parse_options(int argc, char **argv, Options &options, std::string &error) {
  if (argc == 1) {
    return true;
  }

  const std::string_view option = argv[1];
  if (option == "-h" || option == "--help") {
    options.action = Action::kHelp;
    return true;
  }
  if (option == "-v" || option == "--version") {
    options.action = Action::kVersion;
    return true;
  }
  if (option == "-c" || option == "--script") {
    if (argc < 3) {
      error = std::string(option) + " requires a Tcl script path";
      return false;
    }
    options.action = Action::kScript;
    options.input = argv[2];
    append_arguments(options, 3, argc, argv);
    return true;
  }
  if (option == "-p" || option == "--commands") {
    if (argc != 3) {
      error = std::string(option) + " requires one Tcl command string";
      return false;
    }
    options.action = Action::kCommands;
    options.input = argv[2];
    return true;
  }
  if (!option.empty() && option.front() == '-') {
    error = "unknown option: " + std::string(option);
    return false;
  }

  options.action = Action::kScript;
  options.input = argv[1];
  append_arguments(options, 2, argc, argv);
  return true;
}

} // namespace abys::cli
