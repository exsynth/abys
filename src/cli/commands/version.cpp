#include "../command.h"

#include <string>

#include "abys/version.h"

namespace abys::cli {

int version_command(void *, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
  if (objc != 1) {
    Tcl_WrongNumArgs(interp, 1, objv, nullptr);
    return TCL_ERROR;
  }
  const std::string text = "abys " + version();
  Tcl_SetObjResult(interp, Tcl_NewStringObj(text.data(), text.size()));
  return TCL_OK;
}

} // namespace abys::cli
