#include "../command.h"

#include <string_view>

#include "abys/ir/tig_transformer.h"

namespace abys::cli {

int fast_command(void *client_data, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
  bool bus = false;
  if (objc == 2 && std::string_view(Tcl_GetString(objv[1])) == "-bus") {
    bus = true;
  } else if (objc != 1) {
    return set_error(interp, "fast", "usage: fast ?-bus?");
  }
  auto &state = *static_cast<State *>(client_data);
  if (!state.design.has_value()) {
    return set_error(interp, "fast", "no design has been read");
  }
  ir::TigTransformer(*state.design, state.diagnostics, state.naming).fast(bus);
  return TCL_OK;
}

} // namespace abys::cli
