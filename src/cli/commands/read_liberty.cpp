#include "../command.h"

#include <cmath>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>

#include "abys/liberty/parser.h"

namespace abys::cli {
namespace {

std::string_view exclusion_reason(mapping::Library::ExclusionReason reason) {
  switch (reason) {
  case mapping::Library::ExclusionReason::kTooManyInputs:
    return "more than six inputs";
  case mapping::Library::ExclusionReason::kMissingFunction:
    return "missing output function";
  case mapping::Library::ExclusionReason::kThreeState:
    return "three-state output";
  case mapping::Library::ExclusionReason::kUnsupportedFunction:
    return "unsupported output function";
  }
  return "unknown reason";
}

} // namespace

int read_liberty_command(void *client_data, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
  if (objc != 2 && objc != 4) {
    Tcl_WrongNumArgs(interp, 1, objv, "file ?-input_slew value?");
    return TCL_ERROR;
  }

  mapping::Library::Options options;
  if (objc == 4) {
    if (std::string_view(Tcl_GetString(objv[2])) != "-input_slew") {
      return set_error(interp, "READ_LIBERTY",
                       "unknown option: " + std::string(Tcl_GetString(objv[2])));
    }
    double input_slew;
    if (Tcl_GetDoubleFromObj(interp, objv[3], &input_slew) != TCL_OK) {
      return TCL_ERROR;
    }
    if (!std::isfinite(input_slew) || input_slew < 0.0) {
      return set_error(interp, "READ_LIBERTY", "input slew must be finite and nonnegative");
    }
    options.input_slew = input_slew;
  }

  const std::string path = Tcl_GetString(objv[1]);
  std::ifstream input(path);
  if (!input) {
    return set_error(interp, "READ_LIBERTY", "cannot open input file: " + path);
  }
  const std::string source(std::istreambuf_iterator<char>(input), {});

  auto &state = *static_cast<State *>(client_data);
  auto ast = liberty::parse(source, state.library_diagnostics, path);
  if (!ast) {
    return set_error(interp, "READ_LIBERTY", "failed to parse Liberty file: " + path);
  }
  auto library = liberty::build_cell_library(std::move(*ast), state.library_diagnostics, path);
  if (!library) {
    return set_error(interp, "READ_LIBERTY", "failed to build Liberty library: " + path);
  }
  auto mapping_library =
      std::make_shared<mapping::Library>(*library, state.library_diagnostics, options);
  for (const auto &cell : mapping_library->excluded_cells()) {
    state.library_diagnostics.warning(DiagnosticId::kLibertyCellExcluded,
                                      cell.name + ": " +
                                          std::string(exclusion_reason(cell.reason)));
  }
  state.mapping_library = std::move(mapping_library);
  state.library = std::move(*library);
  return TCL_OK;
}

} // namespace abys::cli
