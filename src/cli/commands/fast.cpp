#include "../command.h"

#include "abys/ir/tig_transformer.h"

namespace abys::cli {

int fast_command(void *client_data, Tcl_Interp *interp, int objc, Tcl_Obj *const[]) {
  if (objc != 1) {
    return set_error(interp, "fast", "usage: fast");
  }
  auto &state = *static_cast<State *>(client_data);
  if (!state.design.has_value()) {
    return set_error(interp, "fast", "no design has been read");
  }
  ir::TigTransformer(*state.design, state.diagnostics, state.naming).fast();
  return TCL_OK;
}

} // namespace abys::cli
