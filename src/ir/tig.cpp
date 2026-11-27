#include "abys/ir/tig.h"

#include <utility>

namespace abys::ir {

NodeId Tig::Module::size() const {
  return static_cast<NodeId>(node_kinds_.size());
}

Tig::Module::NodeKind Tig::Module::node_kind(NodeId id) const {
  assert(id < size());
  return node_kinds_[id];
}

const std::vector<EdgeRef> &Tig::Module::node_inputs(NodeId id) const {
  assert(id < size());
  return node_inputs_[id];
}

std::vector<EdgeRef> &Tig::Module::node_inputs(NodeId id) {
  return const_cast<std::vector<EdgeRef> &>(std::as_const(*this).node_inputs(id));
}

PortIndex Tig::Module::node_output_count(NodeId id) const {
  assert(id < size());
  return static_cast<PortIndex>(node_output_names_[id].size());
}

const std::string &Tig::Module::node_output_name(NodeId id, PortIndex port) const {
  assert(port < node_output_count(id));
  return node_output_names_[id][port];
}

std::string &Tig::Module::node_output_name(NodeId id, PortIndex port) {
  return const_cast<std::string &>(std::as_const(*this).node_output_name(id, port));
}

SignalWidth Tig::Module::node_output_outer_dim(NodeId id, PortIndex port) const {
  assert(port < node_output_count(id));
  return node_output_outer_dims_[id][port];
}

bool Tig::Module::node_output_outer_sign(NodeId id, PortIndex port) const {
  assert(port < node_output_count(id));
  return node_output_outer_signs_[id][port];
}

void Tig::Module::append_node_output(NodeId id, std::string name, SignalWidth outer_dim,
                                     bool outer_sign) {
  assert(node_output_count(id) < kInvalidPortIndex);
  node_output_names_[id].push_back(std::move(name));
  node_output_outer_dims_[id].push_back(outer_dim);
  node_output_outer_signs_[id].push_back(outer_sign);
}

void Tig::Module::clear_node_outputs(NodeId id) {
  set_node_outputs(id, {}, {}, {});
}

void Tig::Module::set_node_outputs(NodeId id, std::vector<std::string> names,
                                   std::vector<SignalWidth> outer_dims,
                                   std::vector<bool> outer_signs) {
  assert(id < size() && names.size() == outer_dims.size() && names.size() == outer_signs.size());
  node_output_names_[id] = std::move(names);
  node_output_outer_dims_[id] = std::move(outer_dims);
  node_output_outer_signs_[id] = std::move(outer_signs);
}

const std::vector<bool> &Tig::Module::node_output_combs(NodeId id) const {
  assert(id < size());
  return node_output_combs_[id];
}

std::vector<bool> &Tig::Module::node_output_combs(NodeId id) {
  return const_cast<std::vector<bool> &>(std::as_const(*this).node_output_combs(id));
}

const std::string &Tig::Module::node_constant_value(NodeId id) const {
  assert(node_kind(id) == NodeKind::kConstant);
  return constant_values_[node_kind_indices_[id]];
}

std::string &Tig::Module::node_constant_value(NodeId id) {
  return const_cast<std::string &>(std::as_const(*this).node_constant_value(id));
}

const std::string &Tig::Module::node_name(NodeId id) const {
  assert(node_kind(id) == NodeKind::kInstance || node_kind(id) == NodeKind::kBoundNetwork);
  return node_kind(id) == NodeKind::kInstance ? instance_names_[node_kind_indices_[id]]
                                              : bound_network_names_[node_kind_indices_[id]];
}

std::string &Tig::Module::node_name(NodeId id) {
  return const_cast<std::string &>(std::as_const(*this).node_name(id));
}

const ModuleId &Tig::Module::node_module_id(NodeId id) const {
  assert(node_kind(id) == NodeKind::kInstance);
  return instance_module_ids_[node_kind_indices_[id]];
}

ModuleId &Tig::Module::node_module_id(NodeId id) {
  return const_cast<ModuleId &>(std::as_const(*this).node_module_id(id));
}

const EdgeKind &Tig::Module::node_clock_kind(NodeId id) const {
  assert(node_kind(id) == NodeKind::kFf || node_kind(id) == NodeKind::kMemory);
  return node_kind(id) == NodeKind::kFf ? ff_clock_edges_[node_kind_indices_[id]]
                                        : memory_clock_edges_[node_kind_indices_[id]];
}

EdgeKind &Tig::Module::node_clock_kind(NodeId id) {
  return const_cast<EdgeKind &>(std::as_const(*this).node_clock_kind(id));
}

const EdgeKind &Tig::Module::node_reset_kind(NodeId id) const {
  assert(node_kind(id) == NodeKind::kFf || node_kind(id) == NodeKind::kMemory);
  return node_kind(id) == NodeKind::kFf ? ff_reset_edges_[node_kind_indices_[id]]
                                        : memory_reset_edges_[node_kind_indices_[id]];
}

EdgeKind &Tig::Module::node_reset_kind(NodeId id) {
  return const_cast<EdgeKind &>(std::as_const(*this).node_reset_kind(id));
}

const std::vector<ExprId> &Tig::Module::node_input_expr_ids(NodeId id) const {
  assert(node_kind(id) == NodeKind::kOp);
  return op_input_expr_ids_[node_kind_indices_[id]];
}

std::vector<ExprId> &Tig::Module::node_input_expr_ids(NodeId id) {
  return const_cast<std::vector<ExprId> &>(std::as_const(*this).node_input_expr_ids(id));
}

const ExprGraph &Tig::Module::node_expr_graph(NodeId id) const {
  assert(node_kind(id) == NodeKind::kOp);
  return op_graphs_[node_kind_indices_[id]];
}

ExprGraph &Tig::Module::node_expr_graph(NodeId id) {
  return const_cast<ExprGraph &>(std::as_const(*this).node_expr_graph(id));
}

const std::vector<ExprId> &Tig::Module::node_output_expr_ids(NodeId id) const {
  assert(node_kind(id) == NodeKind::kOp);
  return op_output_expr_ids_[node_kind_indices_[id]];
}

std::vector<ExprId> &Tig::Module::node_output_expr_ids(NodeId id) {
  return const_cast<std::vector<ExprId> &>(std::as_const(*this).node_output_expr_ids(id));
}

const std::vector<bool> &Tig::Module::node_memory_range_flags(NodeId id) const {
  assert(node_kind(id) == NodeKind::kMemoryRead || node_kind(id) == NodeKind::kMemoryWrite);
  return node_kind(id) == NodeKind::kMemoryRead ? memory_read_range_flags_[node_kind_indices_[id]]
                                                : memory_write_range_flags_[node_kind_indices_[id]];
}

std::vector<bool> &Tig::Module::node_memory_range_flags(NodeId id) {
  return const_cast<std::vector<bool> &>(std::as_const(*this).node_memory_range_flags(id));
}

const std::shared_ptr<boop::AndNetwork> &Tig::Module::node_and_network(NodeId id) const {
  assert(node_kind(id) == NodeKind::kAndNetwork);
  return and_networks_[node_kind_indices_[id]];
}

std::shared_ptr<boop::AndNetwork> &Tig::Module::node_and_network(NodeId id) {
  return const_cast<std::shared_ptr<boop::AndNetwork> &>(std::as_const(*this).node_and_network(id));
}

const std::shared_ptr<boop::BoundNetwork> &Tig::Module::node_bound_network(NodeId id) const {
  assert(node_kind(id) == NodeKind::kBoundNetwork);
  return bound_networks_[node_kind_indices_[id]];
}

std::shared_ptr<boop::BoundNetwork> &Tig::Module::node_bound_network(NodeId id) {
  return const_cast<std::shared_ptr<boop::BoundNetwork> &>(
      std::as_const(*this).node_bound_network(id));
}

const std::shared_ptr<const boop::CellLibrary> &Tig::Module::node_cell_library(NodeId id) const {
  assert(node_kind(id) == NodeKind::kBoundNetwork);
  return bound_libraries_[node_kind_indices_[id]];
}

std::shared_ptr<const boop::CellLibrary> &Tig::Module::node_cell_library(NodeId id) {
  return const_cast<std::shared_ptr<const boop::CellLibrary> &>(
      std::as_const(*this).node_cell_library(id));
}

NodeId Tig::Module::append_node(NodeKind kind) {
  assert(size() != kInvalidNodeId);
  const NodeId id = size();
  const KindIndex index = allocate_payload(kind);
  node_kinds_.push_back(kind);
  node_kind_indices_.push_back(index);
  node_inputs_.emplace_back();
  node_output_names_.emplace_back();
  node_output_outer_dims_.emplace_back();
  node_output_outer_signs_.emplace_back();
  node_output_combs_.emplace_back();
  return id;
}

void Tig::Module::set_node_kind(NodeId id, NodeKind kind) {
  assert(id < size());
  const NodeKind old_kind = node_kinds_[id];
  if (kind == old_kind) {
    return;
  }
  const auto index = allocate_payload(kind);
  const auto old_index = node_kind_indices_[id];
  switch (old_kind) {
  case NodeKind::kConstant:
    constant_values_[old_index].clear();
    break;
  case NodeKind::kOp:
    op_graphs_[old_index] = {};
    op_input_expr_ids_[old_index].clear();
    op_output_expr_ids_[old_index].clear();
    break;
  case NodeKind::kAndNetwork:
    and_networks_[old_index].reset();
    break;
  case NodeKind::kBoundNetwork:
    bound_networks_[old_index].reset();
    bound_libraries_[old_index].reset();
    bound_network_names_[old_index].clear();
    break;
  default:
    break;
  }
  node_kinds_[id] = kind;
  node_kind_indices_[id] = index;
}

Tig::Module::KindIndex Tig::Module::allocate_payload(NodeKind kind) {
  assert(kind != NodeKind::kCount);
  switch (kind) {
  case NodeKind::kInstance:
    instance_names_.emplace_back();
    instance_module_ids_.push_back(kInvalidModuleId);
    break;
  case NodeKind::kFf:
    ff_clock_edges_.push_back(EdgeKind::kNone);
    ff_reset_edges_.push_back(EdgeKind::kNone);
    break;
  case NodeKind::kMemory:
    memory_clock_edges_.push_back(EdgeKind::kNone);
    memory_reset_edges_.push_back(EdgeKind::kNone);
    break;
  case NodeKind::kOp:
    op_graphs_.emplace_back();
    op_input_expr_ids_.emplace_back();
    op_output_expr_ids_.emplace_back();
    break;
  case NodeKind::kConstant:
    constant_values_.emplace_back();
    break;
  case NodeKind::kMemoryRead:
    memory_read_range_flags_.emplace_back();
    break;
  case NodeKind::kMemoryWrite:
    memory_write_range_flags_.emplace_back();
    break;
  case NodeKind::kAndNetwork:
    and_networks_.emplace_back();
    break;
  case NodeKind::kBoundNetwork:
    bound_network_names_.emplace_back();
    bound_networks_.emplace_back();
    bound_libraries_.emplace_back();
    break;
  default:
    break;
  }
  return next_kind_indices_[static_cast<size_t>(kind)]++;
}

} // namespace abys::ir
