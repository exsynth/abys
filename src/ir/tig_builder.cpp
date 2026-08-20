#include "abys/ir/tig_builder.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace abys::ir {
TigBuilder::TigBuilder(Tig &design, Diagnostics &diagnostics, const NamingOptions &naming)
    : design_(design), diagnostics_(diagnostics), naming_(naming),
      subroutine_name_counts_(design.modules.size()), signal_maps_(design.modules.size()),
      pending_edge_writes_(design.modules.size()), input_specs_(design.modules.size()) {}

void TigBuilder::set_top_module(std::string name) {
  design_.top_module_name = std::move(name);
}

std::string TigBuilder::generate_temporary_name() {
  return naming_.builder_temporary_signal_prefix + std::to_string(temporary_name_count_++);
}

std::string TigBuilder::create_temporary_signal(ModuleId module_id, SignalWidth width, bool sign,
                                                std::vector<SignalWidth> unpacked_dims) {
  std::string name = generate_temporary_name();
  create_signal(module_id, name, width, sign, std::move(unpacked_dims));
  return name;
}

TigBuilder::NodeId TigBuilder::create_node(ModuleId module_id, NodeKind kind) {
  Module &module = design_.modules[module_id];
  NodeId node_id = static_cast<NodeId>(module.nodes.size());
  module.nodes.emplace_back();
  module.nodes.back().kind = kind;
  return node_id;
}

void TigBuilder::add_signal(ModuleId module_id, std::string name, Signal signal) {
  auto [it, inserted] = signal_maps_[module_id].emplace(std::move(name), signal);
  assert(inserted);
  (void)it;
}

void TigBuilder::add_input_spec(ModuleId module_id, NodeId node_id, SignalSpec input_spec) {
  if (module_id >= static_cast<ModuleId>(input_specs_.size())) {
    input_specs_.resize(module_id + 1);
  }
  if (node_id >= static_cast<NodeId>(input_specs_[module_id].size())) {
    input_specs_[module_id].resize(node_id + 1);
  }
  input_specs_[module_id][node_id].push_back(std::move(input_spec));
}

TigBuilder::ModuleId TigBuilder::create_module(std::string name) {
  size_t &count = module_name_counts_[name];
  std::string variant_suffix;
  if (count == 0) {
    ++count;
  } else {
    variant_suffix = naming_.builder_module_variant_prefix + std::to_string(count++);
  }
  ModuleId module_id = static_cast<ModuleId>(design_.modules.size());
  design_.modules.emplace_back();
  design_.modules.back().name = std::move(name);
  design_.modules.back().variant_suffix = std::move(variant_suffix);
  signal_maps_.emplace_back();
  pending_edge_writes_.emplace_back();
  input_specs_.emplace_back();
  subroutine_name_counts_.emplace_back();
  return module_id;
}

TigBuilder::NodeId TigBuilder::create_module_input(ModuleId module_id, std::string name,
                                                   SignalWidth width, bool sign,
                                                   std::vector<SignalWidth> unpacked_dims) {
  Module &module = design_.modules[module_id];
  const SignalWidth interface_width = unpacked_dims.empty() ? width : unpacked_dims.front();
  const bool interface_sign = unpacked_dims.empty() ? sign : false;
  module.input_ports.push_back({name, std::move(unpacked_dims), width, sign});
  NodeId node_id = create_node(module_id, NodeKind::kPi);
  Node &node = module.nodes[node_id];
  node.outputs.push_back({name, interface_width, interface_sign});
  add_signal(module_id, std::move(name), {node_id, 0});
  return node_id;
}

TigBuilder::NodeId TigBuilder::create_module_output(ModuleId module_id, std::string name,
                                                    SignalWidth width, bool sign,
                                                    std::string input_name, NodeId input_id,
                                                    PortIndex port_idx,
                                                    std::vector<SignalWidth> unpacked_dims) {
  Module &module = design_.modules[module_id];
  const SignalWidth interface_width = unpacked_dims.empty() ? width : unpacked_dims.front();
  const bool interface_sign = unpacked_dims.empty() ? sign : false;
  module.output_ports.push_back({std::move(name), std::move(unpacked_dims), width, sign});
  NodeId node_id = create_node(module_id, NodeKind::kPo);
  Node &node = module.nodes[node_id];
  if (!input_name.empty()) {
    add_input_spec(module_id, node_id, {std::move(input_name), interface_width, interface_sign});
  }
  node.inputs.push_back({input_id, port_idx});
  return node_id;
}

void TigBuilder::create_signal(ModuleId module_id, std::string name, SignalWidth width, bool sign,
                               std::vector<SignalWidth> unpacked_dims) {
  Module &module = design_.modules[module_id];
  module.signals.push_back({std::move(name), std::move(unpacked_dims), width, sign});
}

TigBuilder::NodeId TigBuilder::create_instance(ModuleId module_id, std::string name,
                                               ModuleId instance_module_id) {
  Module &module = design_.modules[module_id];
  NodeId node_id = create_node(module_id, NodeKind::kInstance);
  Node &node = module.nodes[node_id];
  node.name = std::move(name);
  node.module_id = instance_module_id;
  return node_id;
}

TigBuilder::NodeId TigBuilder::create_operation(ModuleId module_id) {
  NodeId node_id = create_node(module_id, NodeKind::kOp);
  return node_id;
}

TigBuilder::NodeId TigBuilder::create_multi_driver(ModuleId module_id) {
  NodeId node_id = create_node(module_id, NodeKind::kMultiDriver);
  return node_id;
}

TigBuilder::NodeId TigBuilder::create_join(ModuleId module_id) {
  NodeId node_id = create_node(module_id, NodeKind::kJoin);
  return node_id;
}

void TigBuilder::record_edge_write(ModuleId module_id, std::string name, SignalSpec clk_spec,
                                   EdgeKind clk_edge, SignalSpec rst_spec, EdgeKind rst_edge,
                                   NodeId node_id, PortIndex port_idx) {
  assert(!clk_spec.name.empty());
  pending_edge_writes_[module_id].emplace_back(
      PendingEdgeWrite{std::move(name), std::move(clk_spec), clk_edge, std::move(rst_spec),
                       rst_edge, node_id, port_idx});
}

void TigBuilder::add_node_input(ModuleId module_id, NodeId node_id, NodeId input_id,
                                PortIndex port_idx, ExprId expr_id) {
  Module &module = design_.modules[module_id];
  Node &node = module.nodes[node_id];
  node.inputs.push_back({input_id, port_idx});
  if (node.kind == NodeKind::kOp) {
    assert(expr_id != kInvalidExprId);
    node.input_expr_ids.push_back(expr_id);
  }
  add_input_spec(module_id, node_id, {"", 0, false});
}

void TigBuilder::add_node_input_spec(ModuleId module_id, NodeId node_id, std::string name,
                                     SignalWidth width, bool sign, ExprId expr_id) {
  Module &module = design_.modules[module_id];
  Node &node = module.nodes[node_id];
  node.inputs.emplace_back();
  if (node.kind == NodeKind::kOp) {
    assert(expr_id != kInvalidExprId);
    node.input_expr_ids.push_back(expr_id);
  }
  add_input_spec(module_id, node_id, {std::move(name), width, sign});
}

void TigBuilder::finalize_node_input(ModuleId module_id, NodeId node_id) {
  if (module_id >= static_cast<ModuleId>(input_specs_.size())) {
    return;
  }
  if (node_id >= static_cast<NodeId>(input_specs_[module_id].size())) {
    return;
  }
  bool fUseSpec = false;
  for (SignalSpec const &input_spec : input_specs_[module_id][node_id]) {
    if (!input_spec.name.empty()) {
      fUseSpec = true;
      break;
    }
  }
  if (!fUseSpec) {
    input_specs_[module_id][node_id].clear();
  }
}

PortIndex TigBuilder::add_node_output(ModuleId module_id, NodeId node_id, std::string name,
                                      SignalWidth width, bool sign, ExprId expr_id, bool comb) {
  Module &module = design_.modules[module_id];
  const PortIndex port_idx = static_cast<PortIndex>(module.nodes[node_id].outputs.size());
  std::string final_name;
  if (!name.empty()) {
    auto &signal_map = signal_maps_[module_id];
    auto it = signal_map.find(name);
    if (it != signal_map.end()) {
      assert(get_signal_spec(module_id, it->second).width == width);
      assert(get_signal_spec(module_id, it->second).sign == sign);
      assert(it->second.node_id != kInvalidNodeId);
      Node &current_node = module.nodes[it->second.node_id];
      if (current_node.kind != NodeKind::kMultiDriver) {
        if (current_node.kind == NodeKind::kOp) {
          // TODO: handle multiple drivers
          current_node.outputs[it->second.port_idx].name.clear();
        }
        NodeId multi_driver_id =
            create_multi_driver(module_id); // current_node may be invalidated here
        Node &multi_driver_node = module.nodes[multi_driver_id];
        multi_driver_node.outputs.push_back({std::move(name), width, sign});
        multi_driver_node.expr_roots.push_back(kInvalidExprId);
        multi_driver_node.combs.push_back(false);
        add_node_input(module_id, multi_driver_id, it->second.node_id, it->second.port_idx);
        it->second = Signal{multi_driver_id, 0};
      }
      add_node_input(module_id, it->second.node_id, node_id, port_idx);
    } else {
      add_signal(module_id, name, {node_id, port_idx});
      final_name = std::move(name);
    }
  }
  Node &node = module.nodes[node_id];
  node.outputs.push_back({std::move(final_name), width, sign});
  node.expr_roots.push_back(expr_id);
  node.combs.push_back(comb);
  return port_idx;
}

PortIndex TigBuilder::add_node_output_expr(ModuleId module_id, NodeId node_id, std::string name,
                                           ExprId expr_id, bool comb) {
  const Module &module = design_.modules[module_id];
  const Node &node = module.nodes[node_id];
  const auto &expr_node = node.expr_graph.nodes[expr_id];
  return add_node_output(module_id, node_id, std::move(name), expr_node.width, expr_node.sign,
                         expr_id, comb);
}

ExprGraph &TigBuilder::get_expr_graph(ModuleId module_id, NodeId node_id) {
  Module &module = design_.modules[module_id];
  Node &node = module.nodes[node_id];
  return node.expr_graph;
}

void TigBuilder::resolve_edge_writes(ModuleId module_id) {
  // TODO: detect overlapping sequential drivers.
  auto same_event_control = [](const PendingEdgeWrite &a, const PendingEdgeWrite &b) {
    if (a.clk_spec.name != b.clk_spec.name || a.clk_spec.width != b.clk_spec.width ||
        a.clk_spec.sign != b.clk_spec.sign || a.clk_edge != b.clk_edge) {
      return false;
    }
    const bool a_has_rst = !a.rst_spec.name.empty();
    const bool b_has_rst = !b.rst_spec.name.empty();
    if (a_has_rst != b_has_rst) {
      return false;
    }
    if (!a_has_rst) {
      return true;
    }
    if (a.rst_spec.name != b.rst_spec.name || a.rst_spec.width != b.rst_spec.width ||
        a.rst_spec.sign != b.rst_spec.sign || a.rst_edge != b.rst_edge) {
      return false;
    }
    return true;
  };

  auto &module = design_.modules[module_id];

  auto is_memory = [&](std::string_view name) {
    const auto it = std::ranges::find(module.signals, name, &Tig::SignalProperties::name);
    assert(it != module.signals.end());
    return !it->unpacked_dims.empty();
  };

  auto create_edge_state = [&](NodeKind kind, const PendingEdgeWrite &pending_edge_write,
                               const Signal &signal, const SignalSpec &spec, bool named) -> Signal {
    assert(kind == NodeKind::kFf || kind == NodeKind::kMemory);
    NodeId state_id = create_node(module_id, kind);
    Node &state_node = module.nodes[state_id];
    add_node_input(module_id, state_id, signal.node_id, signal.port_idx);
    add_node_input_spec(module_id, state_id, pending_edge_write.clk_spec.name,
                        pending_edge_write.clk_spec.width, pending_edge_write.clk_spec.sign);
    state_node.clk_edge = pending_edge_write.clk_edge;
    if (!pending_edge_write.rst_spec.name.empty()) {
      add_node_input_spec(module_id, state_id, pending_edge_write.rst_spec.name,
                          pending_edge_write.rst_spec.width, pending_edge_write.rst_spec.sign);
      state_node.rst_edge = pending_edge_write.rst_edge;
    }
    state_node.outputs.push_back({named ? pending_edge_write.name : "", spec.width, spec.sign});
    state_node.expr_roots.push_back(kInvalidExprId);
    state_node.combs.push_back(false);
    return Signal{state_id, 0};
  };

  auto &pending_edge_writes = pending_edge_writes_[module_id];
  auto &signal_map = signal_maps_[module_id];
  std::sort(pending_edge_writes.begin(), pending_edge_writes.end(),
            [](const PendingEdgeWrite &a, const PendingEdgeWrite &b) { return a.name < b.name; });
  for (size_t begin = 0; begin < pending_edge_writes.size();) {
    size_t end = begin + 1;
    while (end < pending_edge_writes.size() &&
           pending_edge_writes[end].name == pending_edge_writes[begin].name) {
      ++end;
    }
    auto it = signal_map.find(pending_edge_writes[begin].name);
    if (it == signal_map.end()) {
      diagnostics_.error(DiagnosticId::kLoweringInvalidFfTreatedAsCombinational,
                         pending_edge_writes[begin].name + " (signal not found)");
      begin = end;
      continue;
    }
    const auto spec = get_signal_spec(module_id, it->second);
    const NodeKind state_kind =
        is_memory(pending_edge_writes[begin].name) ? NodeKind::kMemory : NodeKind::kFf;
    assert(module.nodes[it->second.node_id].kind == NodeKind::kOp ||
           module.nodes[it->second.node_id].kind == NodeKind::kMultiDriver);
    module.nodes[it->second.node_id].outputs[it->second.port_idx].name.clear();
    if (begin + 1 == end) {
      it->second =
          create_edge_state(state_kind, pending_edge_writes[begin], it->second, spec, true);
      begin = end;
      continue;
    }
    std::vector<std::vector<size_t>> clusters;
    for (size_t i = begin; i < end; ++i) {
      bool f = false;
      for (auto &cluster : clusters) {
        if (same_event_control(pending_edge_writes[i], pending_edge_writes[cluster.front()])) {
          cluster.push_back(i);
          f = true;
          break;
        }
      }
      if (!f) {
        clusters.push_back({i});
      }
    }
    if (clusters.size() == 1) {
      it->second = create_edge_state(state_kind, pending_edge_writes[clusters.front().front()],
                                     it->second, spec, true);
    } else {
      std::vector<Signal> states;
      for (const auto &cluster : clusters) {
        Signal signal;
        if (cluster.size() == 1) {
          signal = {pending_edge_writes[cluster.front()].node_id,
                    pending_edge_writes[cluster.front()].port_idx};
        } else {
          NodeId multi_driver_id = create_multi_driver(module_id);
          Node &multi_driver_node = module.nodes[multi_driver_id];
          for (size_t i : cluster) {
            const auto &pending_edge_write = pending_edge_writes[i];
            add_node_input(module_id, multi_driver_id, pending_edge_write.node_id,
                           pending_edge_write.port_idx);
          }
          multi_driver_node.outputs.push_back({"", spec.width, spec.sign});
          multi_driver_node.expr_roots.push_back(kInvalidExprId);
          multi_driver_node.combs.push_back(false);
          signal = Signal{multi_driver_id, 0};
        }
        states.push_back(create_edge_state(state_kind, pending_edge_writes[cluster.front()], signal,
                                           spec, false));
      }
      NodeId join_id = create_join(module_id);
      Node &join_node = module.nodes[join_id];
      for (const auto &state : states) {
        add_node_input(module_id, join_id, state.node_id, state.port_idx);
      }
      join_node.outputs.push_back({pending_edge_writes[begin].name, spec.width, spec.sign});
      join_node.expr_roots.push_back(kInvalidExprId);
      join_node.combs.push_back(false);
      it->second = Signal{join_id, 0};
    }
    begin = end;
  }
  pending_edge_writes.clear();
}

void TigBuilder::wire_connections(ModuleId module_id) {
  Module &module = design_.modules[module_id];
  for (size_t node_id = 0; node_id < input_specs_[module_id].size(); ++node_id) {
    auto &specs = input_specs_[module_id][node_id];
    for (size_t i = 0; i < specs.size(); ++i) {
      const std::string &name = specs[i].name;
      if (!name.empty()) {
        const auto &signal_map = signal_maps_[module_id];
        const auto it = signal_map.find(name);
        if (it == signal_map.end()) {
          diagnostics_.warning(DiagnosticId::kLoweringUnresolvedSignalInput,
                               module.name + module.variant_suffix + "." + name);
          continue;
        }
        assert(it->second.node_id != kInvalidNodeId);
        const auto spec = get_signal_spec(module_id, it->second);
        assert(specs[i].width == spec.width);
        assert(specs[i].sign == spec.sign);
        set_node_input(module_id, static_cast<NodeId>(node_id), static_cast<PortIndex>(i),
                       it->second);
      } else {
        Signal input =
            get_node_input(module_id, static_cast<NodeId>(node_id), static_cast<PortIndex>(i));
        if (input.node_id == kInvalidNodeId) {
          continue;
        }
      }
    }
  }
  for (SubrId subr_id = 0; subr_id < design_.subroutines.size(); ++subr_id) {
    Tig::Subroutine &subroutine = design_.subroutines[subr_id];
    if (subroutine.module_id != module_id) {
      continue;
    }
    auto &specs = subroutine_capture_specs_[subr_id];
    assert(specs.size() == subroutine.captures.size());
    for (size_t i = 0; i < specs.size(); ++i) {
      const auto signal = signal_maps_[module_id].find(specs[i].name);
      if (signal == signal_maps_[module_id].end()) {
        diagnostics_.warning(DiagnosticId::kLoweringUnresolvedSignalInput,
                             module.name + module.variant_suffix + "." + specs[i].name);
        continue;
      }
      const auto output = get_signal_spec(module_id, signal->second);
      assert(specs[i].width == output.width);
      assert(specs[i].sign == output.sign);
      subroutine.captures[i] = signal->second;
    }
    specs.clear();
  }
}

ExprGraph *TigBuilder::create_subroutine(SubrId id, ModuleId module_id, std::string name) {
  if (id >= design_.subroutines.size()) {
    design_.subroutines.resize(static_cast<size_t>(id) + 1);
  }
  if (id >= subroutine_capture_specs_.size()) {
    subroutine_capture_specs_.resize(static_cast<size_t>(id) + 1);
  }
  Tig::Subroutine &subr = design_.subroutines[id];
  if (subr.expr_root != kInvalidExprId) {
    diagnostics_.error(DiagnosticId::kLoweringDuplicateSubroutineIgnored, name);
    return nullptr;
  }
  size_t &count = module_id == kInvalidModuleId ? global_subroutine_name_counts_[name]
                                                : subroutine_name_counts_.at(module_id)[name];
  if (count == 0) {
    ++count;
  } else {
    subr.variant_suffix = naming_.builder_module_variant_prefix + std::to_string(count++);
  }
  subr.module_id = module_id;
  subr.name = std::move(name);
  return &subr.expr_graph;
}

void TigBuilder::add_subroutine_input(SubrId id, std::string name, SignalWidth width, bool sign,
                                      ExprId expr_id, std::vector<SignalWidth> unpacked_dims) {
  assert(id < design_.subroutines.size());
  Tig::Subroutine &subroutine = design_.subroutines[id];
  subroutine.inputs.push_back({std::move(name), std::move(unpacked_dims), width, sign});
  subroutine.input_expr_ids.push_back(expr_id);
}

bool TigBuilder::add_subroutine_capture_spec(SubrId id, std::string name, SignalWidth width,
                                             bool sign, ExprId expr_id) {
  assert(id < design_.subroutines.size());
  Tig::Subroutine &subroutine = design_.subroutines[id];
  if (subroutine.module_id == kInvalidModuleId) {
    return false;
  }
  subroutine.captures.emplace_back();
  subroutine.capture_expr_ids.push_back(expr_id);
  subroutine_capture_specs_[id].push_back({std::move(name), width, sign});
  return true;
}

void TigBuilder::set_subroutine_root(SubrId id, ExprId root) {
  assert(id < design_.subroutines.size());
  design_.subroutines[id].expr_root = root;
}

void TigBuilder::set_node_input(ModuleId module_id, NodeId node_id, PortIndex port_idx,
                                Signal input) {
  Module &module = design_.modules[module_id];
  Node &node = module.nodes[node_id];
  node.inputs[port_idx] = input;
}

TigBuilder::Signal TigBuilder::get_node_input(ModuleId module_id, NodeId node_id,
                                              PortIndex port_idx) const {
  const Module &module = design_.modules[module_id];
  const Node &node = module.nodes[node_id];
  return node.inputs[port_idx];
}

TigBuilder::SignalSpec TigBuilder::get_signal_spec(ModuleId module_id, Signal signal) const {
  const Module &module = design_.modules[module_id];
  const Node &node = module.nodes[signal.node_id];
  assert(signal.port_idx < node.outputs.size());
  return node.outputs[signal.port_idx];
}

} // namespace abys::ir
