#include "../command.h"

#include <memory>
#include <utility>

#include "abys/ir/tig_transformer.h"

namespace abys::cli {

int map_command(void *client_data, Tcl_Interp *interp, int objc, Tcl_Obj *const[]) {
  if (objc != 1) {
    return set_error(interp, "map", "usage: map");
  }
  auto &state = *static_cast<State *>(client_data);
  if (!state.design.has_value()) {
    return set_error(interp, "map", "no design has been read");
  }
  if (!state.mapping_library) {
    return set_error(interp, "map", "no cell library has been read");
  }
  ir::TigTransformer transformer(*state.design, state.diagnostics, state.naming);
  std::shared_ptr<const boop::CellLibrary> library(state.mapping_library,
                                                   &state.mapping_library->cells());
  if (!transformer.map(std::move(library))) {
    return set_error(interp, "map", "technology mapping failed");
  }
  return TCL_OK;
}

} // namespace abys::cli
