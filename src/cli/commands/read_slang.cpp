#include "../command.h"

#include <fstream>
#include <string>
#include <vector>

#include "abys/frontend/api.h"

namespace abys::cli {
namespace {

struct ReadArguments {
  std::vector<std::string> files;
  std::string top;
};

int parse_arguments(Tcl_Interp *interp, int objc, Tcl_Obj *const objv[], ReadArguments &arguments) {
  bool positional_only = false;
  for (int i = 1; i < objc; ++i) {
    const std::string arg = Tcl_GetString(objv[i]);
    if (!positional_only && arg == "--") {
      positional_only = true;
    } else if (!positional_only && arg == "-top") {
      if (++i >= objc) {
        return set_error(interp, "READ_SLANG", "-top requires a module name");
      }
      arguments.top = Tcl_GetString(objv[i]);
    } else if (!positional_only && !arg.empty() && arg.front() == '-') {
      return set_error(interp, "READ_SLANG", "unknown option: " + arg);
    } else {
      arguments.files.push_back(arg);
    }
  }

  if (arguments.files.empty()) {
    Tcl_WrongNumArgs(interp, 1, objv, "file ?file ...? ?-top module?");
    return TCL_ERROR;
  }
  return TCL_OK;
}

} // namespace

int read_slang_command(void *client_data, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
  auto &state = *static_cast<State *>(client_data);
  ReadArguments arguments;
  if (parse_arguments(interp, objc, objv, arguments) != TCL_OK) {
    return TCL_ERROR;
  }

  for (const auto &path : arguments.files) {
    std::ifstream input(path);
    if (!input) {
      return set_error(interp, "READ_SLANG", "cannot open input file: " + path);
    }
  }

  auto result =
      build_tig_from_systemverilog(arguments.files, arguments.top, state.diagnostics, state.naming);
  if (!result.ok) {
    return set_error(interp, "READ_SLANG", result.message);
  }

  state.design = std::move(result.design);
  return TCL_OK;
}

} // namespace abys::cli
