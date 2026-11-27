#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "abys/ir/expr.h"
#include "abys/ir/signal_table.h"
#include "abys/ir/type.h"

namespace boop {
class AndNetwork;
class BoundNetwork;
class CellLibrary;
} // namespace boop

namespace abys::ir {

struct Tig {
  class Module;
  struct Subroutine;

  std::string top_module_name;
  std::vector<Module> modules;
  std::vector<Subroutine> subroutines;
};

class Tig::Module {
public:
  enum class NodeKind : uint8_t {
    // Hierarchy.
    kInstance,
    // Module ports.
    kPi,
    kPo,
    // Expression logic.
    kOp,
    // Logic networks.
    kAndNetwork,
    kBoundNetwork,
    // Driver resolution.
    kMultiDriver,
    kEdgeMultiDriver,
    kJoin,
    // Sequential logic.
    kFf,
    kLatch,
    // Memories and their access ports.
    kMemory,
    kMemoryRead,
    kMemoryWrite,
    // Fixed-interface adapters.
    kFlatten,
    kFold,
    // Constant source.
    kConstant,
    // Other node kinds.
    kMacro,
    kUnknown,
    // Node kind count.
    kCount,
  };

  // Number of nodes.
  NodeId size() const;

  // Fields shared by every node.
  NodeKind node_kind(NodeId id) const;
  const std::vector<EdgeRef> &node_inputs(NodeId id) const;
  std::vector<EdgeRef> &node_inputs(NodeId id);
  PortIndex node_output_count(NodeId id) const;
  const std::string &node_output_name(NodeId id, PortIndex port) const;
  std::string &node_output_name(NodeId id, PortIndex port);
  SignalWidth node_output_width(NodeId id, PortIndex port) const;
  bool node_output_sign(NodeId id, PortIndex port) const;
  const std::vector<bool> &node_output_combs(NodeId id) const;
  std::vector<bool> &node_output_combs(NodeId id);

  // Node insertion and rewrites.
  NodeId append_node(NodeKind kind);
  void set_node_kind(NodeId id, NodeKind kind);

  // Output port updates.
  void append_node_output(NodeId id, std::string name, SignalWidth width, bool sign);
  void clear_node_outputs(NodeId id);
  void set_node_outputs(NodeId id, std::vector<std::string> names, std::vector<SignalWidth> widths,
                        std::vector<bool> signs);

  // kConstant literal.
  const std::string &node_constant_value(NodeId id) const;
  std::string &node_constant_value(NodeId id);

  // Instance name for kInstance and kBoundNetwork.
  const std::string &node_name(NodeId id) const;
  std::string &node_name(NodeId id);

  // Module instantiated by kInstance.
  const ModuleId &node_module_id(NodeId id) const;
  ModuleId &node_module_id(NodeId id);

  // Clock and reset edge kinds for kFf and kMemory.
  const EdgeKind &node_clock_edge(NodeId id) const;
  EdgeKind &node_clock_edge(NodeId id);
  const EdgeKind &node_reset_edge(NodeId id) const;
  EdgeKind &node_reset_edge(NodeId id);

  // kOp graph, with expression IDs parallel to the node inputs and outputs.
  const std::vector<ExprId> &node_input_expr_ids(NodeId id) const;
  std::vector<ExprId> &node_input_expr_ids(NodeId id);
  const ExprGraph &node_expr_graph(NodeId id) const;
  ExprGraph &node_expr_graph(NodeId id);
  const std::vector<ExprId> &node_output_expr_ids(NodeId id) const;
  std::vector<ExprId> &node_output_expr_ids(NodeId id);

  // Region flags for kMemoryRead and kMemoryWrite.
  const std::vector<bool> &node_memory_region_ranges(NodeId id) const;
  std::vector<bool> &node_memory_region_ranges(NodeId id);

  // kAndNetwork payload.
  const std::shared_ptr<boop::AndNetwork> &node_and_network(NodeId id) const;
  std::shared_ptr<boop::AndNetwork> &node_and_network(NodeId id);

  // kBoundNetwork payload.
  const std::shared_ptr<boop::BoundNetwork> &node_bound_network(NodeId id) const;
  std::shared_ptr<boop::BoundNetwork> &node_bound_network(NodeId id);
  const std::shared_ptr<const boop::CellLibrary> &node_cell_library(NodeId id) const;
  std::shared_ptr<const boop::CellLibrary> &node_cell_library(NodeId id);

  // Module data.
  std::string name;
  std::string variant_suffix;
  uint64_t transform_name_count = 0;

  // Names and declared shapes of ports and signals.
  SignalTable input_ports;
  SignalTable output_ports;
  SignalTable signals;

private:
  using KindIndex = uint32_t;

  // Reserves the next payload index for this kind and initializes its fields.
  KindIndex allocate_payload(NodeKind kind);

  // Next payload index for each node kind.
  std::array<KindIndex, static_cast<size_t>(NodeKind::kCount)> next_kind_indices_{};

  // Indexed by NodeId.
  std::vector<NodeKind> node_kinds_;
  std::vector<KindIndex> node_kind_indices_;
  std::vector<std::vector<EdgeRef>> node_inputs_;
  std::vector<std::vector<std::string>> node_output_names_;
  std::vector<std::vector<SignalWidth>> node_output_widths_;
  std::vector<std::vector<bool>> node_output_signs_;
  std::vector<std::vector<bool>> node_output_combs_;

  // Indexed by KindIndex within the corresponding kind.
  std::vector<std::string> instance_names_;
  std::vector<ModuleId> instance_module_ids_;

  // kFf and kMemory payloads.
  std::vector<EdgeKind> ff_clock_edges_;
  std::vector<EdgeKind> ff_reset_edges_;
  std::vector<EdgeKind> memory_clock_edges_;
  std::vector<EdgeKind> memory_reset_edges_;

  // kOp payload.
  std::vector<ExprGraph> op_graphs_;
  std::vector<std::vector<ExprId>> op_input_expr_ids_;
  std::vector<std::vector<ExprId>> op_output_expr_ids_;

  // kConstant payload.
  std::vector<std::string> constant_values_;

  // Memory access payloads.
  std::vector<std::vector<bool>> memory_read_ranges_;
  std::vector<std::vector<bool>> memory_write_ranges_;

  // Logic network payloads.
  std::vector<std::shared_ptr<boop::AndNetwork>> and_networks_;
  std::vector<std::string> bound_network_names_;
  std::vector<std::shared_ptr<boop::BoundNetwork>> bound_networks_;
  std::vector<std::shared_ptr<const boop::CellLibrary>> bound_libraries_;
};

struct Tig::Subroutine {
  ModuleId module_id = kInvalidModuleId;
  std::string name;
  std::string variant_suffix;
  SignalTable inputs;
  std::vector<ExprId> input_expr_ids;
  std::vector<EdgeRef> captures;
  std::vector<ExprId> capture_expr_ids;
  ExprGraph expr_graph;
  ExprId output_expr_id = kInvalidExprId;
};

} // namespace abys::ir
