#include "abys/ir/tig_transformer.h"

#include <cassert>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "abys/ir/expr_builder.h"

namespace abys::ir {

TigTransformer::TigTransformer(Tig &design, Diagnostics &diagnostics, const NamingOptions &naming)
    : design_(design), diagnostics_(diagnostics), naming_(naming) {}

std::optional<UnpackedShape> TigTransformer::get_unpacked_shape(const Tig::Module &module,
                                                                NodeId node_id, ExprId id) {
  if (const auto *shape = module.node_expr_graph(node_id).unpacked_shape(id)) {
    return *shape;
  }
  return std::nullopt;
}

ExprId TigTransformer::add_node_input_expr(Tig::Module &module, NodeId node_id, EdgeRef input) {
  assert(module.node_kind(node_id) == Tig::Module::NodeKind::kOp);
  assert(module.node_inputs(node_id).size() == module.node_input_expr_ids(node_id).size());
  for (size_t port = 0; port < module.node_inputs(node_id).size(); ++port) {
    if (module.node_inputs(node_id)[port].node_id == input.node_id &&
        module.node_inputs(node_id)[port].port_idx == input.port_idx) {
      return module.node_input_expr_ids(node_id)[port];
    }
  }
  const auto &name = module.node_output_name(input.node_id, input.port_idx);
  const auto index = module.signals.find_index(name);
  const auto *shape = index ? module.signals.shape(*index) : nullptr;
  const ExprId id = module.node_expr_graph(node_id).append(
      ExprGraph::Op::kInput,
      shape ? shape->unpacked_dims.front()
            : module.node_output_outer_dim(input.node_id, input.port_idx),
      !shape && module.node_output_outer_sign(input.node_id, input.port_idx));
  if (shape) {
    module.node_expr_graph(node_id).set_unpacked_shape(id, *shape);
  }
  module.node_inputs(node_id).push_back(input);
  module.node_input_expr_ids(node_id).push_back(id);
  return id;
}

void TigTransformer::clean_op_node(Tig::Module &module, NodeId node_id) {
  assert(module.node_kind(node_id) == Tig::Module::NodeKind::kOp);
  auto &source = module.node_expr_graph(node_id);
  ExprGraph compact;
  std::vector<ExprId> remap(source.size(), kInvalidExprId);
  remap[ExprGraph::constant_zero] = ExprGraph::constant_zero;
  remap[ExprGraph::constant_one] = ExprGraph::constant_one;

  const auto clone = [&](const auto &self, ExprId old_id) -> ExprId {
    if (old_id == kInvalidExprId) {
      return kInvalidExprId;
    }
    if (remap[old_id] != kInvalidExprId) {
      return remap[old_id];
    }
    auto expression_operands = source.operands(old_id);
    for (ExprId &operand : expression_operands) {
      operand = self(self, operand);
    }
    const ExprId new_id = compact.append_copy(source, old_id, std::move(expression_operands));
    remap[old_id] = new_id;
    return new_id;
  };

  for (ExprId &root : module.node_output_expr_ids(node_id)) {
    root = clone(clone, root);
  }
  assert(module.node_inputs(node_id).size() == module.node_input_expr_ids(node_id).size());
  std::vector<EdgeRef> compact_inputs;
  std::vector<ExprId> compact_input_expr_ids;
  for (size_t port = 0; port < module.node_inputs(node_id).size(); ++port) {
    const ExprId id = module.node_input_expr_ids(node_id)[port];
    assert(module.node_inputs(node_id)[port].node_id != kInvalidNodeId ||
           remap[id] == kInvalidExprId);
    if (remap[id] != kInvalidExprId) {
      compact_inputs.push_back(module.node_inputs(node_id)[port]);
      compact_input_expr_ids.push_back(remap[id]);
    }
  }
  module.node_inputs(node_id) = std::move(compact_inputs);
  module.node_input_expr_ids(node_id) = std::move(compact_input_expr_ids);

  source = std::move(compact);
}

bool TigTransformer::is_expr_graph_topological(const Tig::Module &module, NodeId node_id) {
  assert(module.node_kind(node_id) == Tig::Module::NodeKind::kOp);
  for (ExprId id = 0; id < module.node_expr_graph(node_id).size(); ++id) {
    for (ExprId operand : module.node_expr_graph(node_id).operands(id)) {
      if (operand != kInvalidExprId && operand >= id) {
        return false;
      }
    }
  }
  return true;
}

EdgeRef TigTransformer::add_node_output_expr(Tig::Module &module, NodeId node_id, ExprId expr_id,
                                             std::string name) {
  const PortIndex port_idx = static_cast<PortIndex>(module.node_output_count(node_id));
  module.append_node_output(node_id, name, module.node_expr_graph(node_id).outer_dim(expr_id),
                            module.node_expr_graph(node_id).outer_sign(expr_id));
  module.node_output_expr_ids(node_id).push_back(expr_id);
  module.node_output_combs(node_id).push_back(true);
  if (!name.empty()) {
    const auto &graph = module.node_expr_graph(node_id);
    append_signal(module, name, graph.outer_dim(expr_id), graph.outer_sign(expr_id),
                  graph.unpacked_shape(expr_id));
  }
  return {node_id, port_idx};
}

std::string TigTransformer::create_temporary_name(Tig::Module &module) const {
  return naming_.transformer_temporary_signal_prefix +
         std::to_string(module.transform_name_count++);
}

void TigTransformer::append_signal(Tig::Module &module, const std::string &name,
                                   SignalWidth outer_dim, bool outer_sign,
                                   const UnpackedShape *shape) {
  if (!module.signals.append(name, outer_dim, outer_sign, shape)) {
    diagnostics_.error(DiagnosticId::kDuplicateSignalName, name);
  }
}

void TigTransformer::apply_replacements(Tig::Module &module, const Replacements &replacements) {
  for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
    for (EdgeRef &input : module.node_inputs(node_id)) {
      if (input.node_id == kInvalidNodeId) {
        continue;
      }
      const auto replacement = replacements.find({input.node_id, input.port_idx});
      if (replacement != replacements.end()) {
        input = replacement->second;
      }
    }
  }
}

ExprId TigTransformer::materialize_unpacked_range_assign_base(const ExprGraph &graph,
                                                              ExprBuilder &builder, ExprId id) {
  if (const auto base = graph.unpacked_range_assign_base(id)) {
    return builder.find_or_create_const(*base, ExprBuilder::minimum_unsigned_width(*base), false);
  }
  return graph.operands(id).at(1);
}

} // namespace abys::ir
