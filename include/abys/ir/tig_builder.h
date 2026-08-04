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
  Tig &design_;
  Diagnostics &diagnostics_;
  const NamingOptions &naming_;
  uint32_t temporary_name_count_ = 0;
  std::unordered_map<std::string, size_t> module_name_counts_;

public:
  using NodeId = Tig::NodeId;
  using ModuleId = Tig::ModuleId;
  static constexpr NodeId kInvalidNodeId = Tig::kInvalidNodeId;
  static constexpr ModuleId kInvalidModuleId = Tig::kInvalidModuleId;
  using Module = Tig::Module;
  using Node = Tig::Module::Node;
  using NodeKind = Tig::Module::NodeKind;
  using EdgeRef = Tig::Module::EdgeRef;
  using Signal = EdgeRef;
  using SignalSpec = Tig::Module::Node::Output;

private:
  struct PendingFf {
    std::string name;
    SignalSpec clk_spec;
    EdgeKind clk_edge = EdgeKind::kNone;
    SignalSpec rst_spec;
    EdgeKind rst_edge = EdgeKind::kNone;
    NodeId node_id = kInvalidNodeId;
    PortIndex port_idx = 0;
  };

  std::vector<std::unordered_map<std::string, Signal>> signal_maps_;
  std::vector<std::vector<PendingFf>> pending_ffs_;
  std::vector<std::vector<std::vector<SignalSpec>>> input_specs_;

  NodeId create_node(ModuleId module_id, NodeKind kind);
  void add_signal(ModuleId module_id, std::string name, Signal signal);
  void add_input_spec(ModuleId module_id, NodeId node_id, SignalSpec input_spec);

public:
  TigBuilder(Tig &design, Diagnostics &diagnostics, const NamingOptions &naming);
  void set_top_module(std::string name);
  std::string generate_temporary_name();
  std::string create_temporary_signal(ModuleId module_id, SignalWidth width, bool sign,
                                      std::vector<SignalWidth> unpacked_dims = {});

  ModuleId create_module(std::string name);

  NodeId create_module_input(ModuleId module_id, std::string name, SignalWidth width, bool sign,
                             std::vector<SignalWidth> unpacked_dims = {});
  NodeId create_module_output(ModuleId module_id, std::string name, SignalWidth width, bool sign,
                              std::string input_name, NodeId input_id = kInvalidNodeId,
                              PortIndex port_idx = 0, std::vector<SignalWidth> unpacked_dims = {});

  void create_signal(ModuleId module_id, std::string name, SignalWidth width, bool sign,
                     std::vector<SignalWidth> unpacked_dims = {});

  NodeId create_instance(ModuleId module_id, std::string name, ModuleId instance_module_id);
  NodeId create_operation(ModuleId module_id);
  NodeId create_merge(ModuleId module_id);
  NodeId create_ff_merge(ModuleId module_id);
  void record_ff(ModuleId module_id, std::string name, SignalSpec clk_spec, EdgeKind clk_edge,
                 SignalSpec rst_spec, EdgeKind rst_edge, NodeId node_id, PortIndex port_idx);

  void add_node_input(ModuleId module_id, NodeId node_id, NodeId input_id, PortIndex port_idx = 0);
  void add_node_input_spec(ModuleId module_id, NodeId node_id, std::string name, SignalWidth width,
                           bool sign);
  void finalize_node_input(ModuleId module_id, NodeId node_id);
  PortIndex add_node_output(ModuleId module_id, NodeId node_id, std::string name, SignalWidth width,
                            bool sign, ExprId expr_id = kInvalidExprId, bool comb = false);
  PortIndex add_node_output_expr(ModuleId module_id, NodeId node_id, std::string name,
                                 ExprId expr_id, bool comb);

  ExprGraph &get_expr_graph(ModuleId module_id, NodeId node_id);
  void insert_ffs(ModuleId module_id);
  void wire_connections(ModuleId module_id);

  ExprGraph *create_subroutine(SubrId id, std::string name);
  void add_subroutine_input(SubrId id, std::string name, SignalWidth width, bool sign,
                            std::vector<SignalWidth> unpacked_dims = {});
  void set_subroutine_root(SubrId id, ExprId root);
  void flatten_calls();

  void set_node_input(ModuleId module_id, NodeId node_id, PortIndex port_idx, Signal input);

  Signal get_node_input(ModuleId module_id, NodeId node_id, PortIndex port_idx) const;

  SignalSpec get_signal_spec(ModuleId module_id, Signal signal) const;
};

} // namespace abys::ir
