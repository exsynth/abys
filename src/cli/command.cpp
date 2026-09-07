#include "command.h"

#include <array>
#include <iostream>
#include <string>

namespace abys::cli {
namespace {

constexpr std::array commands{
    CommandSpec{"read_slang", "file ?file ...? ?-top module?",
                "Read one or more SystemVerilog files and lower into IR.", read_slang_command,
                true},
    CommandSpec{"flatten_subroutine", "", "Inline subroutine calls into their callers.",
                flatten_subroutine_command, true},
    CommandSpec{"infer_memory", "", "Infer memory reads and writes.", infer_memory_command, true},
    CommandSpec{"decompose_dynamic_access", "",
                "Decompose variable shifts and indexed accesses into muxes.",
                decompose_dynamic_access_command, true},
    CommandSpec{"blast", "", "Blast interfaces of non-memory nodes and simplify fixed accesses.",
                blast_command, true},
    CommandSpec{"dump", "?file?", "Dump IR as SystemVerilog, returning it or writing it to path.",
                dump_command, true},
    CommandSpec{"version", "", "Show the Abys version.", version_command, false},
    CommandSpec{"help", "?command?", "Show general or command-specific help.", help_command, false},
};

} // namespace

State::State() : diagnostics(std::cerr) {}

int set_error(Tcl_Interp *interp, const char *command, const std::string &message) {
  Tcl_SetObjResult(interp, Tcl_NewStringObj(message.data(), message.size()));
  Tcl_SetErrorCode(interp, "ABYS", command, nullptr);
  return TCL_ERROR;
}

std::span<const CommandSpec> command_specs() {
  return commands;
}

int register_commands(Tcl_Interp *interp) {
  auto *state = new State;
  Tcl_SetAssocData(
      interp, "abys.cli", [](void *data, Tcl_Interp *) { delete static_cast<State *>(data); },
      state);

  for (const auto &command : commands) {
    Tcl_CreateObjCommand(interp, command.name, command.handler,
                         command.uses_state ? state : nullptr, nullptr);
  }
  return TCL_OK;
}

} // namespace abys::cli
