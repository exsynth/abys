#include "cli.h"

#include <iostream>
#include <string>
#include <vector>

#include <tcl.h>

#include "abys/version.h"
#include "command.h"
#include "options.h"

namespace abys::cli {
namespace {

int initialize(Tcl_Interp *interp) {
  if (Tcl_Init(interp) != TCL_OK || register_commands(interp) != TCL_OK) {
    return TCL_ERROR;
  }

  Tcl_SetVar(interp, "tcl_rcFileName", "~/.abysrc", TCL_GLOBAL_ONLY);
  constexpr const char *readline_setup = R"tcl(
if {$tcl_interactive && ![catch {package require tclreadline}]} {
    set abys_rc [file normalize ~/.abysrc]
    if {[file exists $abys_rc]} {
        source $abys_rc
    }
    set tcl_rcFileName {}
    if {[info exists env(ABYS_HISTORY_FILE)] &&
        [string length $env(ABYS_HISTORY_FILE)] > 0} {
        ::tclreadline::Loop [file normalize $env(ABYS_HISTORY_FILE)]
    } elseif {$tcl_platform(platform) eq "windows"} {
        ::tclreadline::Loop NUL
    } else {
        ::tclreadline::Loop /dev/null
    }
}
)tcl";
  return Tcl_EvalEx(interp, readline_setup, -1, TCL_EVAL_GLOBAL);
}

void print_help() {
  std::cout << "abys: logic synthesis tool\n\n"
               "Usage:\n"
               "  abys                              Start an interactive Tcl shell\n"
               "  abys <script.tcl> [args ...]      Run a Tcl script\n"
               "  abys -c, --script <script.tcl>    Run a Tcl script\n"
               "  abys -p, --commands <commands>    Evaluate Tcl commands\n"
               "  abys -v, --version                Show the Abys version\n"
               "  abys -h, --help                   Show this help\n";
}

int run_tcl_commands(const char *executable, const std::string &commands) {
  Tcl_FindExecutable(executable);
  Tcl_Interp *interp = Tcl_CreateInterp();
  Tcl_SetVar(interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

  int status = initialize(interp);
  if (status == TCL_OK) {
    status =
        Tcl_EvalEx(interp, commands.data(), static_cast<int>(commands.size()), TCL_EVAL_GLOBAL);
  }

  std::string result = Tcl_GetStringResult(interp);
  if (status != TCL_OK) {
    if (Tcl_Obj *error_info = Tcl_GetVar2Ex(interp, "errorInfo", nullptr, TCL_GLOBAL_ONLY)) {
      result = Tcl_GetString(error_info);
    }
  }
  if (!result.empty()) {
    Tcl_Channel channel = Tcl_GetStdChannel(status == TCL_OK ? TCL_STDOUT : TCL_STDERR);
    if (channel != nullptr) {
      Tcl_WriteChars(channel, result.c_str(), static_cast<int>(result.size()));
      Tcl_WriteChars(channel, "\n", 1);
    }
  }

  Tcl_DeleteInterp(interp);
  Tcl_Finalize();
  return status == TCL_OK ? 0 : 1;
}

int run_tcl_main(const char *executable, const Options &options) {
  std::vector<char *> argv;
  argv.reserve(options.arguments.size() + 2);
  argv.push_back(const_cast<char *>(executable));
  if (options.action == Action::kScript) {
    argv.push_back(const_cast<char *>(options.input.c_str()));
    for (const auto &argument : options.arguments) {
      argv.push_back(const_cast<char *>(argument.c_str()));
    }
  }
  Tcl_Main(static_cast<int>(argv.size()), argv.data(), initialize);
  return 0;
}

} // namespace

int run(int argc, char **argv) {
  Options options;
  std::string error;
  if (!parse_options(argc, argv, options, error)) {
    std::cerr << "abys: " << error << '\n';
    return 2;
  }

  switch (options.action) {
  case Action::kHelp:
    print_help();
    return 0;
  case Action::kVersion:
    std::cout << "abys " << version() << '\n';
    return 0;
  case Action::kCommands:
    return run_tcl_commands(argv[0], options.input);
  case Action::kInteractive:
  case Action::kScript:
    return run_tcl_main(argv[0], options);
  }
  return 2;
}

} // namespace abys::cli
