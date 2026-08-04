#include "../command.h"

#include <fstream>
#include <sstream>
#include <string>

#include "abys/ir/tig_dumper.h"

namespace abys::cli {

int dump_command(void *client_data, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
  auto &state = *static_cast<State *>(client_data);
  if (objc > 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "?file?");
    return TCL_ERROR;
  }
  const std::string out_path = objc == 2 ? Tcl_GetString(objv[1]) : "";

  if (!state.design) {
    return set_error(interp, "DUMP", "no current design");
  }

  ir::TigDumper dumper(*state.design, state.diagnostics, state.naming);
  if (!out_path.empty()) {
    std::ofstream output(out_path);
    if (!output) {
      return set_error(interp, "DUMP", "cannot open output file: " + out_path);
    }
    dumper.dump(output);
    if (!output) {
      return set_error(interp, "DUMP", "failed to write output file: " + out_path);
    }
  } else {
    std::ostringstream output;
    dumper.dump(output);
    const std::string text = output.str();
    Tcl_SetObjResult(interp, Tcl_NewStringObj(text.data(), text.size()));
  }
  return TCL_OK;
}

} // namespace abys::cli
