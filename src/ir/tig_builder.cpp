#include "abys/ir/tig_builder.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "abys/ir/expr_builder.h"

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

std::string TigBuilder::create_temporary_signal(ModuleId module_id, SignalWidth outer_dim, bool outer_sign,
                                                const UnpackedShape *shape) {
  std::string name = generate_temporary_name();
  create_signal(module_id, name, outer_dim, outer_sign, shape);
  return name;
}

NodeId TigBuilder::create_node(ModuleId module_id, NodeKind kind) {
  Module &module = design_.modules[module_id];
  return module.append_node(kind);
}

EdgeRef TigBuilder::create_zero_signal(ModuleId module_id, const EdgeSpec &spec) {
  const NodeId node_id = create_operation(module_id);
  ExprBuilder expr_builder(get_expr_graph(module_id, node_id), diagnostics_);
  const ExprId expr_id = expr_builder.find_or_create_const(
      std::to_string(spec.outer_dim) + "'b" + std::string(spec.outer_dim, '0'), spec.outer_dim, spec.outer_sign);
  add_node_output_expr(module_id, node_id, spec.name, expr_id, true);
  return {node_id, 0};
}

void TigBuilder::add_signal(ModuleId module_id, std::string name, EdgeRef ref) {
  auto [it, inserted] = signal_maps_[module_id].emplace(std::move(name), ref);
  assert(inserted);
  (void)it;
}

void TigBuilder::add_input_spec(ModuleId module_id, NodeId node_id, EdgeSpec input_spec) {
  if (module_id >= static_cast<ModuleId>(input_specs_.size())) {
    input_specs_.resize(module_id + 1);
  }
  if (node_id >= static_cast<NodeId>(input_specs_[module_id].size())) {
    input_specs_[module_id].resize(node_id + 1);
  }
  input_specs_[module_id][node_id].push_back(std::move(input_spec));
}

ModuleId TigBuilder::create_module(std::string name) {
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

NodeId TigBuilder::create_module_input(ModuleId module_id, std::string name,
                                                   SignalWidth outer_dim, bool outer_sign,
                                                   const UnpackedShape *shape) {
  Module &module = design_.modules[module_id];
  if (!module.input_ports.append(name, outer_dim, outer_sign, shape)) {
    diagnostics_.error(DiagnosticId::kDuplicateSignalName, name);
    return kInvalidNodeId;
  }
  NodeId node_id = create_node(module_id, NodeKind::kPi);
  module.append_node_output(node_id, name, outer_dim, outer_sign);
  add_signal(module_id, std::move(name), {node_id, 0});
  return node_id;
}

NodeId TigBuilder::create_module_output(ModuleId module_id, std::string name,
                                                    SignalWidth outer_dim, bool outer_sign,
                                                    std::string input_name, NodeId input_id,
                                                    PortIndex port_idx,
                                                    const UnpackedShape *shape) {
  Module &module = design_.modules[module_id];
  if (!module.output_ports.append(name, outer_dim, outer_sign, shape)) {
    diagnostics_.error(DiagnosticId::kDuplicateSignalName, name);
    return kInvalidNodeId;
  }
  NodeId node_id = create_node(module_id, NodeKind::kPo);
  if (!input_name.empty()) {
    add_input_spec(module_id, node_id, {std::move(input_name), outer_dim, outer_sign});
  }
  module.node_inputs(node_id).push_back({input_id, port_idx});
  return node_id;
}

void TigBuilder::create_signal(ModuleId module_id, std::string name, SignalWidth outer_dim, bool outer_sign,
                               const UnpackedShape *shape) {
  Module &module = design_.modules[module_id];
  if (!module.signals.append(name, outer_dim, outer_sign, shape)) {
    diagnostics_.error(DiagnosticId::kDuplicateSignalName, name);
  }
}

NodeId TigBuilder::create_instance(ModuleId module_id, std::string name,
                                               ModuleId instance_module_id) {
  Module &module = design_.modules[module_id];
  NodeId node_id = create_node(module_id, NodeKind::kInstance);
  module.node_name(node_id) = std::move(name);
  module.node_module_id(node_id) = instance_module_id;
  return node_id;
}

NodeId TigBuilder::create_operation(ModuleId module_id) {
  NodeId node_id = create_node(module_id, NodeKind::kOp);
  return node_id;
}

NodeId TigBuilder::create_multi_driver(ModuleId module_id) {
  NodeId node_id = create_node(module_id, NodeKind::kMultiDriver);
  return node_id;
}

NodeId TigBuilder::create_edge_multi_driver(ModuleId module_id) {
  NodeId node_id = create_node(module_id, NodeKind::kEdgeMultiDriver);
  return node_id;
}

NodeId TigBuilder::create_join(ModuleId module_id) {
  NodeId node_id = create_node(module_id, NodeKind::kJoin);
  return node_id;
}

void TigBuilder::record_edge_write(ModuleId module_id, std::string name, EdgeSpec clock_spec,
                                   EdgeKind clock_kind, EdgeSpec reset_spec, EdgeKind reset_kind,
                                   NodeId node_id, PortIndex port_idx) {
  assert(!clock_spec.name.empty());
  pending_edge_writes_[module_id].emplace_back(
      PendingEdgeWrite{std::move(name), std::move(clock_spec), clock_kind, std::move(reset_spec),
                       reset_kind, node_id, port_idx});
}

void TigBuilder::add_node_input(ModuleId module_id, NodeId node_id, NodeId input_id,
                                PortIndex port_idx, ExprId expr_id) {
  Module &module = design_.modules[module_id];
  module.node_inputs(node_id).push_back({input_id, port_idx});
  if (module.node_kind(node_id) == NodeKind::kOp) {
    assert(expr_id != kInvalidExprId);
    module.node_input_expr_ids(node_id).push_back(expr_id);
  }
  add_input_spec(module_id, node_id, {"", 0, false});
}

void TigBuilder::add_node_input_spec(ModuleId module_id, NodeId node_id, std::string name,
                                     SignalWidth outer_dim, bool outer_sign, ExprId expr_id) {
  Module &module = design_.modules[module_id];
  module.node_inputs(node_id).emplace_back();
  if (module.node_kind(node_id) == NodeKind::kOp) {
    assert(expr_id != kInvalidExprId);
    module.node_input_expr_ids(node_id).push_back(expr_id);
  }
  add_input_spec(module_id, node_id, {std::move(name), outer_dim, outer_sign});
}

void TigBuilder::finalize_node_inputs(ModuleId module_id, NodeId node_id) {
  if (module_id >= static_cast<ModuleId>(input_specs_.size())) {
    return;
  }
  if (node_id >= static_cast<NodeId>(input_specs_[module_id].size())) {
    return;
  }
  bool fUseSpec = false;
  for (EdgeSpec const &input_spec : input_specs_[module_id][node_id]) {
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
                                      SignalWidth outer_dim, bool outer_sign, ExprId expr_id, bool comb) {
  Module &module = design_.modules[module_id];
  const PortIndex port_idx = static_cast<PortIndex>(module.node_output_count(node_id));
  std::string final_name;
  if (!name.empty()) {
    auto &signal_map = signal_maps_[module_id];
    auto it = signal_map.find(name);
    if (it != signal_map.end()) {
      assert(get_edge_spec(module_id, it->second).outer_dim == outer_dim);
      assert(get_edge_spec(module_id, it->second).outer_sign == outer_sign);
      assert(it->second.node_id != kInvalidNodeId);
      const auto current_node_id = it->second.node_id;
      if (module.node_kind(current_node_id) != NodeKind::kMultiDriver) {
        if (module.node_kind(current_node_id) == NodeKind::kOp) {
          // TODO: handle multiple drivers
          module.node_output_name(current_node_id, it->second.port_idx).clear();
        }
        NodeId multi_driver_id = create_multi_driver(module_id);
        module.append_node_output(multi_driver_id, std::move(name), outer_dim, outer_sign);
        module.node_output_combs(multi_driver_id).push_back(false);
        add_node_input(module_id, multi_driver_id, it->second.node_id, it->second.port_idx);
        it->second = EdgeRef{multi_driver_id, 0};
      }
      add_node_input(module_id, it->second.node_id, node_id, port_idx);
    } else {
      add_signal(module_id, name, {node_id, port_idx});
      final_name = std::move(name);
    }
  }
  module.append_node_output(node_id, std::move(final_name), outer_dim, outer_sign);
  if (module.node_kind(node_id) == NodeKind::kOp) {
    module.node_output_expr_ids(node_id).push_back(expr_id);
  } else {
    assert(expr_id == kInvalidExprId);
  }
  module.node_output_combs(node_id).push_back(comb);
  return port_idx;
}

PortIndex TigBuilder::add_node_output_expr(ModuleId module_id, NodeId node_id, std::string name,
                                           ExprId expr_id, bool comb) {
  const Module &module = design_.modules[module_id];
  return add_node_output(module_id, node_id, std::move(name),
                         module.node_expr_graph(node_id).outer_dim(expr_id),
                         module.node_expr_graph(node_id).outer_sign(expr_id), expr_id, comb);
}

ExprGraph &TigBuilder::get_expr_graph(ModuleId module_id, NodeId node_id) {
  Module &module = design_.modules[module_id];
  return module.node_expr_graph(node_id);
}

void TigBuilder::resolve_edge_writes(ModuleId module_id) {
  // TODO: detect overlapping sequential drivers.
  auto same_event_control = [](const PendingEdgeWrite &a, const PendingEdgeWrite &b) {
    if (a.clock_spec.name != b.clock_spec.name || a.clock_spec.outer_dim != b.clock_spec.outer_dim ||
        a.clock_spec.outer_sign != b.clock_spec.outer_sign || a.clock_kind != b.clock_kind) {
      return false;
    }
    const bool a_has_reset = !a.reset_spec.name.empty();
    const bool b_has_reset = !b.reset_spec.name.empty();
    if (a_has_reset != b_has_reset) {
      return false;
    }
    if (!a_has_reset) {
      return true;
    }
    if (a.reset_spec.name != b.reset_spec.name || a.reset_spec.outer_dim != b.reset_spec.outer_dim ||
        a.reset_spec.outer_sign != b.reset_spec.outer_sign || a.reset_kind != b.reset_kind) {
      return false;
    }
    return true;
  };

  auto &module = design_.modules[module_id];

  auto is_memory = [&](std::string_view name) {
    const auto index = module.signals.find_index(name);
    assert(index);
    return module.signals.shape(*index) != nullptr;
  };

  auto create_edge_state = [&](NodeKind kind, const PendingEdgeWrite &pending_edge_write,
                               const EdgeRef &data_ref, const EdgeSpec &spec) -> EdgeRef {
    assert(kind == NodeKind::kFf || kind == NodeKind::kMemory);
    NodeId state_id = create_node(module_id, kind);
    add_node_input(module_id, state_id, data_ref.node_id, data_ref.port_idx);
    add_node_input_spec(module_id, state_id, pending_edge_write.clock_spec.name,
                        pending_edge_write.clock_spec.outer_dim, pending_edge_write.clock_spec.outer_sign);
    module.node_clock_kind(state_id) = pending_edge_write.clock_kind;
    if (!pending_edge_write.reset_spec.name.empty()) {
      add_node_input_spec(module_id, state_id, pending_edge_write.reset_spec.name,
                          pending_edge_write.reset_spec.outer_dim, pending_edge_write.reset_spec.outer_sign);
      module.node_reset_kind(state_id) = pending_edge_write.reset_kind;
    }
    module.append_node_output(state_id, "", spec.outer_dim, spec.outer_sign);
    module.node_output_combs(state_id).push_back(false);
    return EdgeRef{state_id, 0};
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
    const auto spec = get_edge_spec(module_id, it->second);
    const NodeKind state_kind =
        is_memory(pending_edge_writes[begin].name) ? NodeKind::kMemory : NodeKind::kFf;
    const EdgeRef old_head_ref = it->second;
    const NodeKind old_head_kind = module.node_kind(old_head_ref.node_id);
    assert(old_head_kind == NodeKind::kOp || old_head_kind == NodeKind::kMultiDriver);

    auto is_edge_driver = [&](EdgeRef driver_ref) {
      return std::ranges::any_of(
          pending_edge_writes.begin() + begin, pending_edge_writes.begin() + end,
          [&](const PendingEdgeWrite &write) {
            return write.node_id == driver_ref.node_id && write.port_idx == driver_ref.port_idx;
          });
    };
    std::vector<EdgeRef> non_edge_driver_refs;
    size_t edge_driver_count = 0;
    if (old_head_kind == NodeKind::kMultiDriver) {
      for (const EdgeRef driver_ref : module.node_inputs(old_head_ref.node_id)) {
        if (is_edge_driver(driver_ref)) {
          ++edge_driver_count;
        } else {
          non_edge_driver_refs.push_back(driver_ref);
        }
      }
    } else {
      assert(is_edge_driver(old_head_ref));
      edge_driver_count = 1;
    }
    assert(edge_driver_count == end - begin);

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
    std::vector<EdgeRef> state_refs;
    for (const auto &cluster : clusters) {
      EdgeRef data_ref;
      if (cluster.size() == 1) {
        data_ref = {pending_edge_writes[cluster.front()].node_id,
                    pending_edge_writes[cluster.front()].port_idx};
      } else {
        NodeId multi_driver_id = create_edge_multi_driver(module_id);
        for (size_t i : cluster) {
          const auto &pending_edge_write = pending_edge_writes[i];
          add_node_input(module_id, multi_driver_id, pending_edge_write.node_id,
                         pending_edge_write.port_idx);
        }
        module.append_node_output(multi_driver_id, "", spec.outer_dim, spec.outer_sign);
        module.node_output_combs(multi_driver_id).push_back(false);
        data_ref = EdgeRef{multi_driver_id, 0};
      }
      state_refs.push_back(
          create_edge_state(state_kind, pending_edge_writes[cluster.front()], data_ref, spec));
    }

    EdgeRef state_ref;
    if (state_refs.size() == 1) {
      state_ref = state_refs.front();
    } else {
      NodeId join_id = create_join(module_id);
      for (const auto &input_ref : state_refs) {
        add_node_input(module_id, join_id, input_ref.node_id, input_ref.port_idx);
      }
      module.append_node_output(join_id, "", spec.outer_dim, spec.outer_sign);
      module.node_output_combs(join_id).push_back(false);
      state_ref = EdgeRef{join_id, 0};
    }

    if (non_edge_driver_refs.empty()) {
      module.node_output_name(old_head_ref.node_id, old_head_ref.port_idx).clear();
      module.node_output_name(state_ref.node_id, state_ref.port_idx) = pending_edge_writes[begin].name;
      it->second = state_ref;
    } else {
      const auto outer_multi_driver_node_id = old_head_ref.node_id;
      assert(module.node_kind(outer_multi_driver_node_id) == NodeKind::kMultiDriver);
      const auto index = module.signals.find_index(pending_edge_writes[begin].name);
      assert(index);
      const auto *shape = module.signals.shape(*index);
      module.node_output_name(state_ref.node_id, state_ref.port_idx) =
          create_temporary_signal(module_id, module.signals.outer_dim(*index),
                                  module.signals.outer_sign(*index), shape);
      module.node_inputs(outer_multi_driver_node_id).clear();
      add_node_input(module_id, old_head_ref.node_id, state_ref.node_id, state_ref.port_idx);
      module.node_inputs(outer_multi_driver_node_id)
          .insert(module.node_inputs(outer_multi_driver_node_id).end(), non_edge_driver_refs.begin(),
                  non_edge_driver_refs.end());
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
          const EdgeRef zero_ref = create_zero_signal(module_id, specs[i]);
          set_node_input(module_id, static_cast<NodeId>(node_id), static_cast<PortIndex>(i), zero_ref);
          continue;
        }
        assert(it->second.node_id != kInvalidNodeId);
        const auto spec = get_edge_spec(module_id, it->second);
        assert(specs[i].outer_dim == spec.outer_dim);
        assert(specs[i].outer_sign == spec.outer_sign);
        set_node_input(module_id, static_cast<NodeId>(node_id), static_cast<PortIndex>(i),
                       it->second);
      } else {
        const EdgeRef input_ref =
            get_node_input(module_id, static_cast<NodeId>(node_id), static_cast<PortIndex>(i));
        if (input_ref.node_id == kInvalidNodeId) {
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
      const auto signal_it = signal_maps_[module_id].find(specs[i].name);
      if (signal_it == signal_maps_[module_id].end()) {
        diagnostics_.warning(DiagnosticId::kLoweringUnresolvedSignalInput,
                             module.name + module.variant_suffix + "." + specs[i].name);
        subroutine.captures[i] = create_zero_signal(module_id, specs[i]);
        continue;
      }
      const auto output_spec = get_edge_spec(module_id, signal_it->second);
      assert(specs[i].outer_dim == output_spec.outer_dim);
      assert(specs[i].outer_sign == output_spec.outer_sign);
      subroutine.captures[i] = signal_it->second;
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
  if (subr.output_expr_id != kInvalidExprId) {
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

void TigBuilder::add_subroutine_input(SubrId id, std::string name, SignalWidth outer_dim, bool outer_sign,
                                      ExprId expr_id, const UnpackedShape *shape) {
  assert(id < design_.subroutines.size());
  Tig::Subroutine &subroutine = design_.subroutines[id];
  if (!subroutine.inputs.append(name, outer_dim, outer_sign, shape)) {
    diagnostics_.error(DiagnosticId::kDuplicateSignalName, name);
    return;
  }
  subroutine.input_expr_ids.push_back(expr_id);
}

bool TigBuilder::add_subroutine_capture_spec(SubrId id, std::string name, SignalWidth outer_dim,
                                             bool outer_sign, ExprId expr_id) {
  assert(id < design_.subroutines.size());
  Tig::Subroutine &subroutine = design_.subroutines[id];
  if (subroutine.module_id == kInvalidModuleId) {
    return false;
  }
  subroutine.captures.emplace_back();
  subroutine.capture_expr_ids.push_back(expr_id);
  subroutine_capture_specs_[id].push_back({std::move(name), outer_dim, outer_sign});
  return true;
}

void TigBuilder::set_subroutine_output_expr_id(SubrId id, ExprId root) {
  assert(id < design_.subroutines.size());
  design_.subroutines[id].output_expr_id = root;
}

void TigBuilder::set_node_input(ModuleId module_id, NodeId node_id, PortIndex port_idx,
                                EdgeRef input_ref) {
  Module &module = design_.modules[module_id];
  module.node_inputs(node_id)[port_idx] = input_ref;
}

EdgeRef TigBuilder::get_node_input(ModuleId module_id, NodeId node_id,
                                              PortIndex port_idx) const {
  const Module &module = design_.modules[module_id];
  return module.node_inputs(node_id)[port_idx];
}

TigBuilder::EdgeSpec TigBuilder::get_edge_spec(ModuleId module_id, EdgeRef ref) const {
  const Module &module = design_.modules[module_id];
  const auto node_id = ref.node_id;
  assert(ref.port_idx < module.node_output_count(node_id));
  return {module.node_output_name(node_id, ref.port_idx),
          module.node_output_outer_dim(node_id, ref.port_idx),
          module.node_output_outer_sign(node_id, ref.port_idx)};
}

} // namespace abys::ir
