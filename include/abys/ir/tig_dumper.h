#pragma once

#include <map>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "abys/infra/diagnostics.h"
#include "abys/infra/naming.h"
#include "abys/ir/tig.h"

namespace abys::ir {

class TigDumper {
public:
  TigDumper(const Tig &design, Diagnostics &diagnostics, const NamingOptions &naming);
  void dump(std::ostream &os) const;
  // std::string emit_to_string(const Design& design) const;

  using Module = Tig::Module;

private:
  // use shift/mask selects for non-direct expression bases for yosys compatibility
  static constexpr bool kUseShiftMaskForExpressionSelects = true;
  static constexpr bool kUsePartialAssignmentForMaskedAssign = true;

  const Tig &design_;
  Diagnostics &diagnostics_;
  const NamingOptions &naming_;

  void emit_module(const Module &module, std::ostream &os) const;
  static void emit_module_header(const Module &module, std::ostream &os);
  static void emit_signal_decls(const Module &module, std::ostream &os);
  void emit_instances(const Module &module, std::ostream &os) const;
  void emit_combinational(const Module &module, std::ostream &os) const;
  void emit_sequential(const Module &module, std::ostream &os) const;
  void emit_output_binds(const Module &module, std::ostream &os) const;
  static void emit_module_footer(std::ostream &os);

  void emit_expr(std::string_view lhs, bool is_nonblocking, bool is_merge,
                 const ExprGraph &expr_graph, ExprId id, std::ostream &os, std::string_view indent,
                 const std::unordered_map<std::string, bool> *assumptions = nullptr) const;
  void emit_exprs(const std::vector<std::string> &lhs_names, bool is_nonblocking, bool is_merge,
                  const ExprGraph &expr_graph, const std::vector<ExprId> &expr_ids,
                  std::ostream &os, std::string_view indent,
                  const std::unordered_map<std::string, bool> *assumptions = nullptr) const;
  void emit_expr_unpacked(const std::string &lhs, bool is_nonblocking, bool is_merge,
                          const ExprGraph &expr_graph, ExprId id,
                          std::map<ExprId, std::string> &names, std::ostream &decl_os,
                          std::ostream &os, std::ostream &assign_os, std::string_view indent,
                          const std::unordered_map<std::string, bool> *assumptions = nullptr) const;
  std::string
  emit_expr_packed(const ExprGraph &expr_graph, ExprId id, std::map<ExprId, std::string> &names,
                   std::ostream &decl_os, std::ostream &os, std::string_view indent,
                   const std::unordered_map<std::string, bool> *assumptions = nullptr) const;
  bool lookup_assumed_condition(const ExprGraph &expr_graph, ExprId id,
                                const std::unordered_map<std::string, bool> *assumptions,
                                bool &value) const;
  static bool can_emit_direct_range_base(const ExprGraph &expr_graph, ExprId id);
};

} // namespace abys::ir
