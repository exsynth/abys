#include "../command.h"

#include "abys/ir/tig_transformer.h"

namespace abys::cli {

int decompose_dynamic_access_command(void *client_data, Tcl_Interp *interp, int objc,
                                     Tcl_Obj *const objv[]) {
  auto &state = *static_cast<State *>(client_data);
  if (objc != 1) {
    Tcl_WrongNumArgs(interp, 1, objv, "");
    return TCL_ERROR;
  }
  if (!state.design) {
    return set_error(interp, "DECOMPOSE_DYNAMIC_ACCESS", "no current design");
  }
  ir::TigTransformer(*state.design, state.diagnostics, state.naming).decompose_dynamic_access();
  return TCL_OK;
}

} // namespace abys::cli
