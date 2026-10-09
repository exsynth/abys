#pragma once

#include <optional>
#include <span>
#include <string>

#include <tcl.h>

#include "abys/infra/diagnostics.h"
#include "abys/infra/naming.h"
#include "abys/ir/tig.h"
#include "abys/liberty/library.h"
#include "abys/mapping/library.h"

namespace abys::cli {

struct State {
  State();

  std::optional<ir::Tig> design;
  std::optional<liberty::CellLibrary> library;
  std::optional<mapping::Library> mapping_library;
  NamingOptions naming;
  Diagnostics diagnostics;
  Diagnostics library_diagnostics;
};

struct CommandSpec {
  const char *name;
  const char *usage;
  const char *description;
  Tcl_ObjCmdProc *handler;
  bool uses_state;
};

int set_error(Tcl_Interp *interp, const char *command, const std::string &message);
std::span<const CommandSpec> command_specs();

int read_slang_command(void *, Tcl_Interp *, int, Tcl_Obj *const[]);
int read_liberty_command(void *, Tcl_Interp *, int, Tcl_Obj *const[]);
int write_liberty_command(void *, Tcl_Interp *, int, Tcl_Obj *const[]);
int write_genlib_command(void *, Tcl_Interp *, int, Tcl_Obj *const[]);
int flatten_subroutine_command(void *, Tcl_Interp *, int, Tcl_Obj *const[]);
int infer_memory_command(void *, Tcl_Interp *, int, Tcl_Obj *const[]);
int decompose_dynamic_access_command(void *, Tcl_Interp *, int, Tcl_Obj *const[]);
int blast_command(void *, Tcl_Interp *, int, Tcl_Obj *const[]);
int aig_command(void *, Tcl_Interp *, int, Tcl_Obj *const[]);
int fast_command(void *, Tcl_Interp *, int, Tcl_Obj *const[]);
int dump_command(void *, Tcl_Interp *, int, Tcl_Obj *const[]);
int version_command(void *, Tcl_Interp *, int, Tcl_Obj *const[]);
int help_command(void *, Tcl_Interp *, int, Tcl_Obj *const[]);

int register_commands(Tcl_Interp *interp);

} // namespace abys::cli
