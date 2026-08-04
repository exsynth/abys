#include "../command.h"

#include <sstream>
#include <string>
#include <string_view>

namespace abys::cli {

int help_command(void *, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
  if (objc > 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "?command?");
    return TCL_ERROR;
  }

  std::ostringstream help;
  if (objc == 1) {
    help << "abys: logic synthesis tool\n\nCommands:\n";
    for (const auto &command : command_specs()) {
      help << "  " << command.name;
      if (command.usage[0] != '\0') {
        help << ' ' << command.usage;
      }
      help << "\n      " << command.description << '\n';
    }
  } else {
    const std::string_view requested = Tcl_GetString(objv[1]);
    for (const auto &command : command_specs()) {
      if (requested == command.name) {
        help << command.name;
        if (command.usage[0] != '\0') {
          help << ' ' << command.usage;
        }
        help << '\n' << command.description << '\n';
        const std::string text = help.str();
        Tcl_SetObjResult(interp, Tcl_NewStringObj(text.data(), text.size()));
        return TCL_OK;
      }
    }
    return set_error(interp, "HELP", "unknown command: " + std::string(requested));
  }

  const std::string text = help.str();
  Tcl_SetObjResult(interp, Tcl_NewStringObj(text.data(), text.size()));
  return TCL_OK;
}

} // namespace abys::cli
