#include "../command.h"

#include "abys/ir/tig_transformer.h"

namespace abys::cli {

int flatten_subroutine_command(void *client_data, Tcl_Interp *interp, int objc,
                               Tcl_Obj *const objv[]) {
  auto &state = *static_cast<State *>(client_data);
  if (objc != 1) {
    Tcl_WrongNumArgs(interp, 1, objv, "");
    return TCL_ERROR;
  }
  if (!state.design) {
    return set_error(interp, "FLATTEN_SUBROUTINE", "no current design");
  }
  ir::TigTransformer(*state.design, state.diagnostics).flatten_subroutine();
  return TCL_OK;
}

} // namespace abys::cli
