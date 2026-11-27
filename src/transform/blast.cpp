#include "abys/ir/tig_transformer.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "abys/ir/expr_builder.h"

namespace abys::ir {

namespace {

std::string escaped_indexed_name(const std::string &name, SignalWidth index) {
  return "\\" + name + "[" + std::to_string(index) + "] ";
}

} // namespace

bool TigTransformer::has_fixed_inputs(Tig::Module::NodeKind kind) {
  using NodeKind = Tig::Module::NodeKind;
  switch (kind) {
  case NodeKind::kPi:
  case NodeKind::kPo:
  case NodeKind::kInstance:
  case NodeKind::kMemory:
  case NodeKind::kMemoryRead:
  case NodeKind::kMemoryWrite:
  case NodeKind::kAndNetwork:
  case NodeKind::kBoundNetwork:
  case NodeKind::kMacro:
  case NodeKind::kUnknown:
  case NodeKind::kFlatten:
    return true;
  default:
    return false;
  }
}

bool TigTransformer::has_fixed_outputs(Tig::Module::NodeKind kind) {
  using NodeKind = Tig::Module::NodeKind;
  switch (kind) {
  case NodeKind::kPi:
  case NodeKind::kPo:
  case NodeKind::kInstance:
  case NodeKind::kMemory:
  case NodeKind::kMemoryRead:
  case NodeKind::kMemoryWrite:
  case NodeKind::kAndNetwork:
  case NodeKind::kBoundNetwork:
  case NodeKind::kMacro:
  case NodeKind::kUnknown:
  case NodeKind::kFold:
  case NodeKind::kConstant:
    return true;
  default:
    return false;
  }
}

std::optional<UnpackedShape> TigTransformer::get_output_shape(const Tig::Module &module,
                                                              NodeId node_id,
                                                              PortIndex port_idx) const {
  using NodeKind = Tig::Module::NodeKind;
  assert(node_id < module.size());
  assert(port_idx < module.node_output_count(node_id));
  if (module.node_kind(node_id) == NodeKind::kOp) {
    assert(port_idx < module.node_output_expr_ids(node_id).size());
    const ExprId root = module.node_output_expr_ids(node_id)[port_idx];
    return root == kInvalidExprId ? std::nullopt : get_unpacked_shape(module, node_id, root);
  }
  const UnpackedShape *shape = nullptr;
  if (module.node_kind(node_id) == NodeKind::kInstance) {
    assert(module.node_module_id(node_id) < design_.modules.size());
    const auto &ports = design_.modules[module.node_module_id(node_id)].output_ports;
    shape = ports.shape(port_idx);
  } else {
    const auto &name = module.node_output_name(node_id, port_idx);
    if (!name.empty()) {
      if (const auto index = module.signals.find_index(name)) {
        shape = module.signals.shape(*index);
      }
    }
  }
  return shape ? std::optional<UnpackedShape>(*shape) : std::nullopt;
}

EdgeRef TigTransformer::insert_flatten(Tig::Module &module, EdgeRef input,
                                       const std::optional<UnpackedShape> &shape) {
  using NodeKind = Tig::Module::NodeKind;
  assert(input.node_id != kInvalidNodeId);
  assert(input.node_id < module.size());
  assert(input.port_idx < module.node_output_count(input.node_id));
  auto &output_name = module.node_output_name(input.node_id, input.port_idx);
  const auto output_outer_dim = module.node_output_outer_dim(input.node_id, input.port_idx);
  const auto output_outer_sign = module.node_output_outer_sign(input.node_id, input.port_idx);
  if (output_name.empty()) {
    output_name = create_temporary_name(module);
    append_signal(module, output_name, output_outer_dim, output_outer_sign,
                  shape ? &*shape : nullptr);
  }
  const NodeId flatten_id = module.append_node(NodeKind::kFlatten);
  module.node_inputs(flatten_id).push_back(input);
  module.append_node_output(flatten_id, create_temporary_name(module),
                            shape ? shape->flattened_width() : output_outer_dim,
                            shape ? shape->inner_sign : output_outer_sign);
  module.node_output_combs(flatten_id).push_back(true);
  return {flatten_id, 0};
}

EdgeRef TigTransformer::insert_fold(Tig::Module &module, EdgeRef input,
                                    const std::optional<UnpackedShape> &shape) {
  using NodeKind = Tig::Module::NodeKind;
  assert(input.node_id != kInvalidNodeId);
  assert(input.node_id < module.size());
  assert(input.port_idx < module.node_output_count(input.node_id));
  auto &output_name = module.node_output_name(input.node_id, input.port_idx);
  const auto output_outer_dim = module.node_output_outer_dim(input.node_id, input.port_idx);
  const auto output_outer_sign = module.node_output_outer_sign(input.node_id, input.port_idx);
  const bool has_name = !output_name.empty();
  std::string name = has_name ? output_name : create_temporary_name(module);
  const auto source_shape = get_output_shape(module, input.node_id, input.port_idx);
  output_name = create_temporary_name(module);
  append_signal(module, output_name, output_outer_dim, output_outer_sign,
                source_shape ? &*source_shape : nullptr);
  std::optional<std::string> constant_value;
  const auto source_id = input.node_id;
  if (!shape && module.node_kind(source_id) == NodeKind::kOp) {
    assert(input.port_idx < module.node_output_expr_ids(source_id).size());
    const ExprId root = module.node_output_expr_ids(source_id)[input.port_idx];
    const auto &graph = module.node_expr_graph(source_id);
    if (root != kInvalidExprId && graph.op(root) == ExprGraph::Op::kConst) {
      constant_value = graph.constant_value(root);
    } else if (root != kInvalidExprId) {
      // TODO: Remove this evaluation once constant propagation runs before bit-blasting.
      ExprGraph source_expr_graph = graph;
      ExprBuilder source_builder(source_expr_graph, diagnostics_);
      if (const auto value = source_builder.try_evaluate(root)) {
        constant_value = format_integral_constant(*value, output_outer_dim, output_outer_sign);
      }
    }
  }
  const NodeId adapter_id =
      module.append_node(constant_value ? NodeKind::kConstant : NodeKind::kFold);
  const auto fold_outer_dim = shape ? shape->unpacked_dims.front() : output_outer_dim;
  const bool fold_outer_sign = shape ? false : output_outer_sign;
  module.append_node_output(adapter_id, name, fold_outer_dim, fold_outer_sign);
  module.node_output_combs(adapter_id).push_back(true);
  if (constant_value) {
    module.node_constant_value(adapter_id) = std::move(*constant_value);
  } else {
    module.node_inputs(adapter_id).push_back(input);
  }
  if (!has_name) {
    append_signal(module, name, fold_outer_dim, fold_outer_sign, shape ? &*shape : nullptr);
  }
  return {adapter_id, 0};
}

void TigTransformer::insert_fixed_interfaces(Tig::Module &module) {
  using NodeKind = Tig::Module::NodeKind;
  const size_t node_count = module.size();
  std::vector<size_t> input_counts;
  input_counts.reserve(node_count);
  for (NodeId node_id = 0; node_id < node_count; ++node_id) {
    input_counts.push_back(module.node_inputs(node_id).size());
  }
  for (NodeId consumer_id = 0; consumer_id < node_count; ++consumer_id) {
    const NodeKind consumer_kind = module.node_kind(consumer_id);
    for (size_t input_idx = 0; input_idx < input_counts[consumer_id]; ++input_idx) {
      EdgeRef input = module.node_inputs(consumer_id)[input_idx];
      assert(input.node_id != kInvalidNodeId);
      const NodeKind source_kind = module.node_kind(input.node_id);
      if (has_fixed_outputs(source_kind) && !has_fixed_inputs(consumer_kind)) {
        const auto shape = get_output_shape(module, input.node_id, input.port_idx);
        if (shape || module.node_output_outer_dim(input.node_id, input.port_idx) > 1) {
          input = insert_flatten(module, input, shape);
        }
      } else if (has_fixed_inputs(consumer_kind) && !has_fixed_outputs(source_kind)) {
        std::optional<UnpackedShape> shape;
        if (consumer_kind == NodeKind::kInstance) {
          const auto &ports = design_.modules[module.node_module_id(consumer_id)].input_ports;
          if (const auto *port_shape = ports.shape(input_idx)) {
            shape = *port_shape;
          }
        } else {
          shape = get_output_shape(module, input.node_id, input.port_idx);
        }
        if (shape || module.node_output_outer_dim(input.node_id, input.port_idx) > 1) {
          input = insert_fold(module, input, shape);
        }
      }
      module.node_inputs(consumer_id)[input_idx] = input;
    }
  }
}

void TigTransformer::split_outputs(Tig::Module &module, PortMaps &port_maps) {
  using NodeKind = Tig::Module::NodeKind;
  const size_t node_count = module.size();
  for (NodeId node_id = 0; node_id < node_count; ++node_id) {
    const bool is_op = module.node_kind(node_id) == NodeKind::kOp;
    if (has_fixed_outputs(module.node_kind(node_id))) {
      port_maps[node_id].resize(module.node_output_count(node_id));
      for (PortIndex port = 0; port < module.node_output_count(node_id); ++port) {
        if (module.node_output_outer_dim(node_id, port) == 1) {
          port_maps[node_id][port] = {port};
        }
      }
      continue;
    }

    const bool has_combs = !module.node_output_combs(node_id).empty();
    if (is_op) {
      assert(module.node_output_count(node_id) == module.node_output_expr_ids(node_id).size());
      assert(module.node_output_count(node_id) == module.node_output_combs(node_id).size());
    } else {
      assert(!has_combs ||
             module.node_output_count(node_id) == module.node_output_combs(node_id).size());
    }

    const PortIndex old_output_count = module.node_output_count(node_id);
    auto &node_port_maps = port_maps[node_id];
    assert(node_port_maps.empty());
    node_port_maps.resize(old_output_count);
    std::optional<ExprBuilder> builder;
    if (is_op) {
      builder.emplace(module.node_expr_graph(node_id), diagnostics_);
    }
    std::vector<std::string> names;
    std::vector<SignalWidth> outer_dims;
    std::vector<bool> outer_signs;
    std::vector<ExprId> roots;
    std::vector<bool> combs;
    for (PortIndex old_port = 0; old_port < old_output_count; ++old_port) {
      const auto &old_name = module.node_output_name(node_id, old_port);
      const auto old_outer_dim = module.node_output_outer_dim(node_id, old_port);
      const ExprId root = is_op ? module.node_output_expr_ids(node_id)[old_port] : kInvalidExprId;
      const auto shape =
          is_op
              ? (root == kInvalidExprId ? std::nullopt : get_unpacked_shape(module, node_id, root))
              : get_output_shape(module, node_id, old_port);
      auto &mapping = node_port_maps[old_port];
      mapping.resize(shape ? shape->flattened_width() : old_outer_dim);

      if (is_op && root == kInvalidExprId) {
        assert(names.size() < kInvalidPortIndex);
        const PortIndex new_port = static_cast<PortIndex>(names.size());
        names.push_back(old_name);
        outer_dims.push_back(old_outer_dim);
        outer_signs.push_back(module.node_output_outer_sign(node_id, old_port));
        roots.push_back(root);
        combs.push_back(module.node_output_combs(node_id)[old_port]);
        std::ranges::fill(mapping, new_port);
        continue;
      }

      std::vector<SignalWidth> indices(shape ? shape->unpacked_dims.size() : 0, 0);
      SignalWidth packed_bit = 0;
      const ExprId data = is_op && shape ? builder->create_unpacked_flatten(root) : root;
      for (SignalWidth bit = 0; bit < mapping.size(); ++bit) {
        std::string name;
        if (!old_name.empty()) {
          if (module.node_kind(node_id) == NodeKind::kFlatten && old_name.front() != '\\') {
            name = escaped_indexed_name(old_name, bit);
            append_signal(module, name, 1, false);
          } else {
            name = old_name;
          }
          if (module.node_kind(node_id) != NodeKind::kFlatten && shape) {
            for (SignalWidth index : indices) {
              name += "[" + std::to_string(index) + "]";
            }
            name += "[" + std::to_string(packed_bit) + "]";
          } else if (module.node_kind(node_id) != NodeKind::kFlatten && mapping.size() > 1) {
            name += "[" + std::to_string(bit) + "]";
          }
        }

        assert(names.size() < kInvalidPortIndex);
        mapping[bit] = static_cast<PortIndex>(names.size());
        names.push_back(std::move(name));
        outer_dims.push_back(1);
        outer_signs.push_back(false);
        if (is_op) {
          roots.push_back(builder->create_static_range(data, bit, 1, false));
        }
        if (has_combs) {
          combs.push_back(module.node_output_combs(node_id)[old_port]);
        }

        if (!shape || ++packed_bit < shape->inner_dim) {
          continue;
        }
        packed_bit = 0;
        for (size_t dimension = indices.size(); dimension-- > 0;) {
          if (++indices[dimension] < shape->unpacked_dims[dimension]) {
            break;
          }
          indices[dimension] = 0;
        }
      }
    }

    module.set_node_outputs(node_id, std::move(names), std::move(outer_dims),
                            std::move(outer_signs));
    if (is_op) {
      module.node_output_expr_ids(node_id) = std::move(roots);
    }
    if (has_combs) {
      module.node_output_combs(node_id) = std::move(combs);
    }
  }
}

void TigTransformer::blast_op_nodes(Tig::Module &module, const PortMaps &port_maps) {
  const auto mapped_bit = [&](EdgeRef old_input, SignalWidth bit) -> EdgeRef {
    assert(old_input.node_id != kInvalidNodeId);
    const auto &mapping = port_maps.at(old_input.node_id).at(old_input.port_idx);
    assert(bit < mapping.size());
    assert(mapping[bit] != kInvalidPortIndex);
    return {old_input.node_id, mapping[bit]};
  };

  for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
    if (module.node_kind(node_id) != Tig::Module::NodeKind::kOp) {
      continue;
    }
    assert(is_expr_graph_topological(module, node_id));
    std::vector<EdgeRef> old_inputs = std::move(module.node_inputs(node_id));
    assert(old_inputs.size() == module.node_input_expr_ids(node_id).size());
    const std::vector<ExprId> old_input_expr_ids = module.node_input_expr_ids(node_id);
    std::vector<std::vector<ExprId>> blasted_ids(module.node_expr_graph(node_id).size());
    std::vector<EdgeRef> inputs;
    std::vector<ExprId> input_expr_ids;
    for (size_t input = 0; input < old_inputs.size(); ++input) {
      const EdgeRef old_input = old_inputs[input];
      assert(old_input.node_id != kInvalidNodeId);
      const auto &mapping = port_maps.at(old_input.node_id).at(old_input.port_idx);
      const ExprId old_input_expr_id = old_input_expr_ids[input];
      assert(old_input_expr_id < blasted_ids.size());
      const bool split =
          get_unpacked_shape(module, node_id, old_input_expr_id) || mapping.size() > 1;
      if (!split) {
        assert(!mapping.empty());
        assert(mapping.front() != kInvalidPortIndex);
        inputs.push_back({old_input.node_id, mapping.front()});
        input_expr_ids.push_back(old_input_expr_id);
        blasted_ids[old_input_expr_id].push_back(old_input_expr_id);
        continue;
      }
      auto &bits = blasted_ids[old_input_expr_id];
      bits.reserve(mapping.size());
      for (SignalWidth bit = 0; bit < mapping.size(); ++bit) {
        const EdgeRef source = mapped_bit(old_input, bit);
        const ExprId input_id = module.node_expr_graph(node_id).append(ExprGraph::Op::kInput, 1);
        inputs.push_back(source);
        input_expr_ids.push_back(input_id);
        bits.push_back(input_id);
      }
    }
    module.node_inputs(node_id) = std::move(inputs);
    module.node_input_expr_ids(node_id) = std::move(input_expr_ids);
    blast_expr_graph(module, node_id, std::move(blasted_ids));
    clean_op_node(module, node_id);
  }
}

void TigTransformer::blast_non_op_nodes(Tig::Module &module, const PortMaps &port_maps) {
  using NodeKind = Tig::Module::NodeKind;
  const auto append_bits = [&](std::vector<EdgeRef> &inputs, EdgeRef input) {
    assert(input.node_id != kInvalidNodeId);
    const auto &mapping = port_maps.at(input.node_id).at(input.port_idx);
    for (PortIndex port : mapping) {
      assert(port != kInvalidPortIndex);
      inputs.push_back({input.node_id, port});
    }
  };

  for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
    size_t data_input_count = 0;
    switch (module.node_kind(node_id)) {
    case NodeKind::kMultiDriver:
    case NodeKind::kEdgeMultiDriver:
    case NodeKind::kJoin:
      data_input_count = module.node_inputs(node_id).size();
      break;
    case NodeKind::kFf: {
      assert(module.node_clock_kind(node_id) != EdgeKind::kNone);
      const size_t control_input_count = module.node_reset_kind(node_id) == EdgeKind::kNone ? 1 : 2;
      assert(module.node_inputs(node_id).size() > control_input_count);
      data_input_count = module.node_inputs(node_id).size() - control_input_count;
      break;
    }
    case NodeKind::kFold:
      assert(module.node_output_count(node_id) == 1);
      assert(!module.node_inputs(node_id).empty());
      data_input_count = module.node_inputs(node_id).size();
      break;
    default:
      continue;
    }

    std::vector<EdgeRef> old_inputs = std::move(module.node_inputs(node_id));
    module.node_inputs(node_id).clear();
    for (size_t input = 0; input < data_input_count; ++input) {
      append_bits(module.node_inputs(node_id), old_inputs[input]);
    }
    module.node_inputs(node_id).insert(module.node_inputs(node_id).end(),
                                       old_inputs.begin() + data_input_count, old_inputs.end());
  }
}

void TigTransformer::remove_buffers(Tig::Module &module) {
  using NodeKind = Tig::Module::NodeKind;
  const size_t node_count = module.size();
  Replacements replacements;
  for (NodeId node_id = 0; node_id < node_count; ++node_id) {
    if (module.node_kind(node_id) != NodeKind::kOp) {
      continue;
    }
    for (PortIndex port = 0; port < module.node_output_count(node_id); ++port) {
      const ExprId root = module.node_output_expr_ids(node_id)[port];
      if (root == kInvalidExprId ||
          module.node_expr_graph(node_id).op(root) != ExprGraph::Op::kInput) {
        continue;
      }
      const auto input = std::ranges::find(module.node_input_expr_ids(node_id), root);
      assert(input != module.node_input_expr_ids(node_id).end());
      EdgeRef source = module.node_inputs(
          node_id)[static_cast<size_t>(input - module.node_input_expr_ids(node_id).begin())];
      assert(source.node_id != kInvalidNodeId);
      while (true) {
        const auto replacement = replacements.find({source.node_id, source.port_idx});
        if (replacement == replacements.end()) {
          break;
        }
        source = replacement->second;
      }
      replacements.emplace(std::pair{node_id, port}, source);
    }
  }
  for (const auto &replacement : replacements) {
    const auto [node_id, port] = replacement.first;
    module.node_output_name(node_id, port).clear();
    module.node_output_expr_ids(node_id)[port] = kInvalidExprId;
    module.node_output_combs(node_id)[port] = false;
  }
  apply_replacements(module, replacements);
}

void TigTransformer::blast() {
  for (auto &module : design_.modules) {
    insert_fixed_interfaces(module);

    PortMaps port_maps(module.size());
    split_outputs(module, port_maps);
    blast_op_nodes(module, port_maps);
    blast_non_op_nodes(module, port_maps);
    remove_buffers(module);
    resolve_multiple_drivers(module);
  }
}

} // namespace abys::ir
