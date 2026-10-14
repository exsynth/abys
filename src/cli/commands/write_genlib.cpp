#include "../command.h"

#include <fstream>
#include <sstream>
#include <string>

namespace abys::cli {

int write_genlib_command(void *client_data, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
  if (objc != 1 && objc != 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "?file?");
    return TCL_ERROR;
  }

  auto &state = *static_cast<State *>(client_data);
  if (!state.mapping_library) {
    return set_error(interp, "WRITE_GENLIB", "no Liberty library has been read");
  }

  if (objc == 2) {
    const std::string path = Tcl_GetString(objv[1]);
    std::ofstream output(path);
    if (!output) {
      return set_error(interp, "WRITE_GENLIB", "cannot open output file: " + path);
    }
    state.mapping_library->write_genlib(output);
    if (!output) {
      return set_error(interp, "WRITE_GENLIB", "failed to write output file: " + path);
    }
    return TCL_OK;
  }

  std::ostringstream output;
  state.mapping_library->write_genlib(output);
  const std::string result = std::move(output).str();
  Tcl_SetObjResult(interp, Tcl_NewStringObj(result.data(), result.size()));
  return TCL_OK;
}

} // namespace abys::cli
