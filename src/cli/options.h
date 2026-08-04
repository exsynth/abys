#pragma once

#include <string>
#include <vector>

namespace abys::cli {

enum class Action {
  kInteractive,
  kScript,
  kCommands,
  kHelp,
  kVersion,
};

struct Options {
  Action action = Action::kInteractive;
  std::string input;
  std::vector<std::string> arguments;
};

bool parse_options(int argc, char **argv, Options &options, std::string &error);

} // namespace abys::cli
