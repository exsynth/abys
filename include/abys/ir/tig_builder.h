#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "abys/infra/diagnostics.h"
#include "abys/infra/naming.h"
#include "abys/ir/tig.h"

namespace abys::ir {

class TigBuilder {
private:
  using Module = Tig::Module;
  using NodeKind = Tig::Module::NodeKind;

  Tig &design_;
  Diagnostics &diagnostics_;
  const NamingOptions &naming_;
  uint32_t temporary_name_count_ = 0;
  std::unordered_map<std::string, size_t> module_name_counts_;
  std::unordered_map<std::string, size_t> global_subroutine_name_counts_;
  std::vector<std::unordered_map<std::string, size_t>> subroutine_name_counts_;

public:
  struct EdgeSpec {
    std::string name;
    SignalWidth outer_dim = 0;
    bool outer_sign = false;
  };

private:
  struct PendingEdgeWrite {
    std::string name;
    EdgeSpec clock_spec;
    EdgeKind clock_kind = EdgeKind::kNone;
    EdgeSpec reset_spec;
    EdgeKind reset_kind = EdgeKind::kNone;
    NodeId node_id = kInvalidNodeId;
    PortIndex port_idx = 0;
  };

  std::vector<std::unordered_map<std::string, EdgeRef>> signal_maps_;
  std::vector<std::vector<PendingEdgeWrite>> pending_edge_writes_;
  std::vector<std::vector<std::vector<EdgeSpec>>> input_specs_;
  std::vector<std::vector<EdgeSpec>> subroutine_capture_specs_;

  NodeId create_node(ModuleId module_id, NodeKind kind);
  EdgeRef create_zero_signal(ModuleId module_id, const EdgeSpec &spec);
  void add_signal(ModuleId module_id, std::string name, EdgeRef ref);
  void add_input_spec(ModuleId module_id, NodeId node_id, EdgeSpec input_spec);

public:
  TigBuilder(Tig &design, Diagnostics &diagnostics, const NamingOptions &naming);
  void set_top_module(std::string name);
  std::string generate_temporary_name();
  std::string create_temporary_signal(ModuleId module_id, SignalWidth outer_dim, bool outer_sign,
                                      const UnpackedShape *shape = nullptr);

  ModuleId create_module(std::string name);

  NodeId create_module_input(ModuleId module_id, std::string name, SignalWidth outer_dim,
                             bool outer_sign, const UnpackedShape *shape = nullptr);
  NodeId create_module_output(ModuleId module_id, std::string name, SignalWidth outer_dim,
                              bool outer_sign, std::string input_name,
                              NodeId input_id = kInvalidNodeId, PortIndex port_idx = 0,
                              const UnpackedShape *shape = nullptr);

  void create_signal(ModuleId module_id, std::string name, SignalWidth outer_dim, bool outer_sign,
                     const UnpackedShape *shape = nullptr);

  NodeId create_instance(ModuleId module_id, std::string name, ModuleId instance_module_id);
  NodeId create_operation(ModuleId module_id);
  NodeId create_multi_driver(ModuleId module_id);
  NodeId create_edge_multi_driver(ModuleId module_id);
  NodeId create_join(ModuleId module_id);
  void record_edge_write(ModuleId module_id, std::string name, EdgeSpec clock_spec,
                         EdgeKind clock_kind, EdgeSpec reset_spec, EdgeKind reset_kind,
                         NodeId node_id, PortIndex port_idx);

  void add_node_input(ModuleId module_id, NodeId node_id, NodeId input_id, PortIndex port_idx = 0,
                      ExprId expr_id = kInvalidExprId);
  void add_node_input_spec(ModuleId module_id, NodeId node_id, std::string name,
                           SignalWidth outer_dim, bool outer_sign, ExprId expr_id = kInvalidExprId);
  void finalize_node_inputs(ModuleId module_id, NodeId node_id);
  PortIndex add_node_output(ModuleId module_id, NodeId node_id, std::string name,
                            SignalWidth outer_dim, bool outer_sign, ExprId expr_id = kInvalidExprId,
                            bool comb = false);
  PortIndex add_node_output_expr(ModuleId module_id, NodeId node_id, std::string name,
                                 ExprId expr_id, bool comb);

  ExprGraph &get_expr_graph(ModuleId module_id, NodeId node_id);
  void resolve_edge_writes(ModuleId module_id);
  void wire_connections(ModuleId module_id);

  ExprGraph *create_subroutine(SubrId id, ModuleId module_id, std::string name);
  void add_subroutine_input(SubrId id, std::string name, SignalWidth outer_dim, bool outer_sign,
                            ExprId expr_id, const UnpackedShape *shape = nullptr);
  bool add_subroutine_capture_spec(SubrId id, std::string name, SignalWidth outer_dim,
                                   bool outer_sign, ExprId expr_id);
  void set_subroutine_output_expr_id(SubrId id, ExprId output_expr_id);
  void set_node_input(ModuleId module_id, NodeId node_id, PortIndex port_idx, EdgeRef input_ref);

  EdgeRef get_node_input(ModuleId module_id, NodeId node_id, PortIndex port_idx) const;

  EdgeSpec get_edge_spec(ModuleId module_id, EdgeRef ref) const;
};

} // namespace abys::ir
