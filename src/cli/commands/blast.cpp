#include "../command.h"

#include "abys/ir/tig_transformer.h"

namespace abys::cli {

int blast_command(void *client_data, Tcl_Interp *interp, int objc, Tcl_Obj *const[]) {
  if (objc != 1) {
    return set_error(interp, "blast", "usage: blast");
  }
  auto &state = *static_cast<State *>(client_data);
  if (!state.design.has_value()) {
    return set_error(interp, "blast", "no design has been read");
  }
  ir::TigTransformer(*state.design, state.diagnostics, state.naming).blast();
  return TCL_OK;
}

} // namespace abys::cli
