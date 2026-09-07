#pragma once

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "abys/ir/expr.h"
#include "abys/ir/type.h"

namespace abys::ir {

struct Tig {

  using NodeId = uint32_t;
  using ModuleId = uint32_t;
  static constexpr NodeId kInvalidNodeId = std::numeric_limits<NodeId>::max();
  static constexpr ModuleId kInvalidModuleId = std::numeric_limits<ModuleId>::max();

  struct SignalProperties {
    std::string name;
    std::vector<SignalWidth> unpacked_dims;
    SignalWidth width = 0;
    bool sign = false;
  };

  struct Module {
    struct EdgeRef {
      NodeId node_id = kInvalidNodeId;
      PortIndex port_idx = 0;
    };

    enum class NodeKind : uint8_t {
      kInstance,
      kPi,
      kPo,
      kOp,
      kMultiDriver,
      kEdgeMultiDriver,
      kJoin,
      kFf,
      kLatch,
      kMemory,
      kMemoryRead,
      kMemoryWrite,
      kFlatten,
      kFold,
      kMacro,
      kUnknown,
    };

    struct Node {
      NodeKind kind = NodeKind::kUnknown;
      std::string name; // instance name
      ModuleId module_id = kInvalidModuleId;
      EdgeKind clk_edge = EdgeKind::kNone; // for ff
      EdgeKind rst_edge = EdgeKind::kNone; // for ff with async reset
      std::vector<EdgeRef> inputs;
      std::vector<ExprId> input_expr_ids; // for kOp, parallel to inputs
      struct Output {
        std::string name;
        SignalWidth width = 0;
        bool sign = false;
      };
      std::vector<Output> outputs;
      std::vector<bool> memory_region_ranges;
      std::vector<ExprId> expr_roots;
      std::vector<bool> combs;
      ExprGraph expr_graph;
    };

    std::string name;
    std::string variant_suffix;
    uint64_t transform_name_count = 0;
    std::vector<SignalProperties> input_ports;
    std::vector<SignalProperties> output_ports;
    std::vector<Node> nodes;
    std::vector<SignalProperties> signals;
  };

  struct Subroutine {
    ModuleId module_id = kInvalidModuleId;
    std::string name;
    std::string variant_suffix;
    std::vector<SignalProperties> inputs;
    std::vector<ExprId> input_expr_ids;
    std::vector<Module::EdgeRef> captures;
    std::vector<ExprId> capture_expr_ids;
    ExprGraph expr_graph;
    ExprId expr_root = kInvalidExprId;
  };

  std::string top_module_name;
  std::vector<Module> modules;
  std::vector<Subroutine> subroutines;
};

} // namespace abys::ir
