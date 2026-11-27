#include "abys/ir/tig_transformer.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <span>
#include <utility>
#include <vector>

namespace abys::ir {

bool TigTransformer::collect_loop_terminals(Tig::Module &module, std::span<EdgeRef> input,
                                            std::vector<std::span<EdgeRef>> &terminals) {
  using NodeKind = Tig::Module::NodeKind;
  assert(!input.empty());
  const auto source_ref = std::ranges::find_if(
      input, [](const auto &input_ref) { return input_ref.node_id != kInvalidNodeId; });
  if (source_ref == input.end()) {
    return true;
  }
  const NodeId source_id = source_ref->node_id;
  assert(source_id < module.size());
  switch (module.node_kind(source_id)) {
  case NodeKind::kJoin: {
    assert(input.size() == module.node_output_count(source_id));
    assert(module.node_inputs(source_id).size() % module.node_output_count(source_id) == 0);
    const size_t driver_count =
        module.node_inputs(source_id).size() / module.node_output_count(source_id);
    for (size_t driver = 0; driver < driver_count; ++driver) {
      std::span ff_refs(module.node_inputs(source_id).data() +
                            driver * module.node_output_count(source_id),
                        module.node_output_count(source_id));
      if (!collect_loop_terminals(module, ff_refs, terminals)) {
        return false;
      }
    }
    return true;
  }
  case NodeKind::kFf: {
    assert(input.size() == module.node_output_count(source_id));
    assert(module.node_inputs(source_id).size() >= module.node_output_count(source_id));
    std::span data_refs(module.node_inputs(source_id).data(), module.node_output_count(source_id));
    const auto edge_ref = std::ranges::find_if(
        data_refs, [](const auto &data_ref) { return data_ref.node_id != kInvalidNodeId; });
    if (edge_ref != data_refs.end() &&
        module.node_kind(edge_ref->node_id) == NodeKind::kEdgeMultiDriver) {
      return collect_loop_terminals(module, data_refs, terminals);
    }
    terminals.push_back(data_refs);
    return true;
  }
  case NodeKind::kEdgeMultiDriver: {
    assert(input.size() == module.node_output_count(source_id));
    assert(module.node_inputs(source_id).size() % module.node_output_count(source_id) == 0);
    const size_t driver_count =
        module.node_inputs(source_id).size() / module.node_output_count(source_id);
    for (size_t driver = 0; driver < driver_count; ++driver) {
      terminals.emplace_back(module.node_inputs(source_id).data() +
                                 driver * module.node_output_count(source_id),
                             module.node_output_count(source_id));
    }
    return true;
  }
  case NodeKind::kFlatten:
  case NodeKind::kFold:
  case NodeKind::kMemory:
  case NodeKind::kMemoryRead:
  case NodeKind::kMemoryWrite:
    return false;
  default:
    terminals.push_back(input);
    return true;
  }
}

void TigTransformer::remove_feedback(Tig::Module &module, NodeId node_id, size_t driver_count) {
  assert(module.node_output_count(node_id) != 0);
  assert(driver_count * module.node_output_count(node_id) <= module.node_inputs(node_id).size());
  std::vector<std::span<EdgeRef>> terminals;
  for (size_t driver = 0; driver < driver_count; ++driver) {
    std::span input(module.node_inputs(node_id).data() + driver * module.node_output_count(node_id),
                    module.node_output_count(node_id));
    if (!collect_loop_terminals(module, input, terminals)) {
      return;
    }
  }
  for (auto terminal : terminals) {
    assert(terminal.size() == module.node_output_count(node_id));
    for (PortIndex port = 0; port < module.node_output_count(node_id); ++port) {
      if (terminal[port].node_id == node_id && terminal[port].port_idx == port) {
        terminal[port].node_id = kInvalidNodeId;
      }
    }
  }
}

std::vector<EdgeRef> TigTransformer::select_driver_inputs(Tig::Module &module, NodeId node_id,
                                                          size_t driver_count) {
  assert(module.node_output_count(node_id) != 0);
  assert(driver_count * module.node_output_count(node_id) <= module.node_inputs(node_id).size());
  std::vector<std::span<EdgeRef>> inputs;
  std::vector<std::span<EdgeRef>> terminals;
  if (module.node_kind(node_id) == Tig::Module::NodeKind::kFf) {
    std::span data_refs(module.node_inputs(node_id).data(), module.node_output_count(node_id));
    if (!collect_loop_terminals(module, data_refs, terminals)) {
      return {};
    }
    inputs = terminals;
  } else {
    inputs.reserve(driver_count);
    terminals.reserve(driver_count);
    for (size_t driver = 0; driver < driver_count; ++driver) {
      std::span input(module.node_inputs(node_id).data() +
                          driver * module.node_output_count(node_id),
                      module.node_output_count(node_id));
      std::vector<std::span<EdgeRef>> collected;
      if (!collect_loop_terminals(module, input, collected)) {
        return {};
      }
      if (collected.empty()) {
        continue;
      }
      assert(collected.size() == 1);
      inputs.push_back(input);
      terminals.push_back(collected.front());
    }
  }
  assert(inputs.size() == terminals.size());
  std::vector<EdgeRef> selected(module.node_output_count(node_id));
  std::vector<size_t> selected_counts(module.node_output_count(node_id), 0);
  for (size_t driver = 0; driver < inputs.size(); ++driver) {
    const auto input = inputs[driver];
    const auto terminal = terminals[driver];
    for (PortIndex port = 0; port < module.node_output_count(node_id); ++port) {
      if (terminal[port].node_id == kInvalidNodeId) {
        input[port].node_id = kInvalidNodeId;
        continue;
      }
      if (selected_counts[port]++ == 0) {
        selected[port] = input[port];
      } else {
        terminal[port].node_id = kInvalidNodeId;
      }
      input[port].node_id = kInvalidNodeId;
    }
  }
  for (PortIndex port = 0; port < module.node_output_count(node_id); ++port) {
    if (selected_counts[port] > 1) {
      diagnostics_.warning(DiagnosticId::kTransformMultipleDriversResolved,
                           module.node_output_name(node_id, port));
    }
  }
  return selected;
}

void TigTransformer::resolve_multiple_drivers(Tig::Module &module) {
  using NodeKind = Tig::Module::NodeKind;
  const size_t node_count = module.size();

  const auto transfer_output_names = [&](const Replacements &replacements) {
    for (const auto &[source, replacement] : replacements) {
      auto &source_name = module.node_output_name(source.first, source.second);
      if (source_name.empty()) {
        continue;
      }
      if (replacement.node_id != kInvalidNodeId) {
        module.node_output_name(replacement.node_id, replacement.port_idx) = source_name;
      }
      source_name.clear();
    }
  };

  for (NodeId node_id = 0; node_id < node_count; ++node_id) {
    if (module.node_kind(node_id) != NodeKind::kMultiDriver) {
      continue;
    }
    assert(module.node_inputs(node_id).size() % module.node_output_count(node_id) == 0);
    remove_feedback(module, node_id,
                    module.node_inputs(node_id).size() / module.node_output_count(node_id));
  }

  for (NodeId node_id = 0; node_id < node_count; ++node_id) {
    if (module.node_kind(node_id) != NodeKind::kJoin) {
      continue;
    }
    assert(module.node_inputs(node_id).size() % module.node_output_count(node_id) == 0);
    remove_feedback(module, node_id,
                    module.node_inputs(node_id).size() / module.node_output_count(node_id));
  }

  for (NodeId ff_id = 0; ff_id < node_count; ++ff_id) {
    if (module.node_kind(ff_id) != NodeKind::kFf) {
      continue;
    }
    remove_feedback(module, ff_id, 1);
  }

  for (NodeId ff_id = 0; ff_id < node_count; ++ff_id) {
    if (module.node_kind(ff_id) != NodeKind::kFf) {
      continue;
    }
    const auto selected = select_driver_inputs(module, ff_id, 1);
    if (selected.empty()) {
      continue;
    }
    std::span data_refs(module.node_inputs(ff_id).data(), module.node_output_count(ff_id));
    assert(selected.size() == data_refs.size());
    std::ranges::copy(selected, data_refs.begin());
  }

  Replacements join_replacements;
  for (NodeId join_id = 0; join_id < node_count; ++join_id) {
    if (module.node_kind(join_id) != NodeKind::kJoin) {
      continue;
    }
    const auto selected = select_driver_inputs(
        module, join_id, module.node_inputs(join_id).size() / module.node_output_count(join_id));
    if (selected.empty()) {
      continue;
    }
    for (PortIndex port = 0; port < selected.size(); ++port) {
      join_replacements.emplace(std::pair{join_id, port}, selected[port]);
    }
  }
  transfer_output_names(join_replacements);
  apply_replacements(module, join_replacements);

  Replacements multi_driver_replacements;
  for (NodeId multi_driver_id = static_cast<NodeId>(node_count); multi_driver_id-- > 0;) {
    if (module.node_kind(multi_driver_id) != NodeKind::kMultiDriver) {
      continue;
    }
    auto selected = select_driver_inputs(module, multi_driver_id,
                                         module.node_inputs(multi_driver_id).size() /
                                             module.node_output_count(multi_driver_id));
    if (selected.empty()) {
      continue;
    }
    for (PortIndex port = 0; port < selected.size(); ++port) {
      EdgeRef &selected_ref = selected[port];
      if (selected_ref.node_id != kInvalidNodeId) {
        const auto replacement =
            multi_driver_replacements.find({selected_ref.node_id, selected_ref.port_idx});
        if (replacement != multi_driver_replacements.end()) {
          selected_ref = replacement->second;
        }
      }
      multi_driver_replacements.emplace(std::pair{multi_driver_id, port}, selected_ref);
    }
  }
  transfer_output_names(multi_driver_replacements);
  apply_replacements(module, multi_driver_replacements);
}

} // namespace abys::ir
