#include "abys/ir/tig_transformer.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "abys/ir/expr_builder.h"
#include "boop/interface/mockturtle_interface.h"
#include "boop/network/and_network.h"

namespace abys::ir {
namespace {

std::vector<int> create_ripple_carry_adder(boop::AndNetwork &network, const std::vector<int> &lhs,
                                           const std::vector<int> &rhs) {
  assert(lhs.size() == rhs.size());
  std::vector<int> result;
  result.reserve(lhs.size());
  int carry = network.Node2Edge(network.GetConst0(), false);
  for (size_t bit = 0; bit < lhs.size(); ++bit) {
    const int operands_xor = network.AddXorEdge(lhs[bit], rhs[bit]);
    result.push_back(network.AddXorEdge(operands_xor, carry));
    const int generated = network.AddAndEdge(lhs[bit], rhs[bit]);
    const int propagated = network.AddAndEdge(operands_xor, carry);
    carry = network.AddOrEdge(generated, propagated);
  }
  return result;
}

std::vector<int> create_array_multiplier(boop::AndNetwork &network, const std::vector<int> &lhs,
                                         const std::vector<int> &rhs) {
  assert(lhs.size() == rhs.size());
  const int zero = network.Node2Edge(network.GetConst0(), false);
  std::vector<int> result(lhs.size(), zero);
  for (size_t rhs_bit = 0; rhs_bit < rhs.size(); ++rhs_bit) {
    std::vector<int> partial(lhs.size(), zero);
    for (size_t output_bit = rhs_bit; output_bit < lhs.size(); ++output_bit) {
      partial[output_bit] = network.AddAndEdge(lhs[output_bit - rhs_bit], rhs[rhs_bit]);
    }
    result = create_ripple_carry_adder(network, result, partial);
  }
  return result;
}

} // namespace

std::vector<NodeId> TigTransformer::extract_arithmetic(Tig::Module &module) {
  std::vector<NodeId> arithmetic_ids;
  const NodeId node_count = static_cast<NodeId>(module.size());
  for (NodeId op_id = 0; op_id < node_count; ++op_id) {
    if (module.node_kind(op_id) != Tig::Module::NodeKind::kOp) {
      continue;
    }
    assert(is_expr_graph_topological(module, op_id));
    const ExprId expr_count = static_cast<ExprId>(module.node_expr_graph(op_id).size());
    std::vector<ExprId> replacements(expr_count, kInvalidExprId);
    bool modified = false;
    for (ExprId id = 0; id < expr_count; ++id) {
      for (ExprId &operand : module.node_expr_graph(op_id).mutable_operands(id)) {
        if (operand != kInvalidExprId && replacements[operand] != kInvalidExprId) {
          operand = replacements[operand];
        }
      }
      if (module.node_expr_graph(op_id).op(id) != ExprGraph::Op::kAdd &&
          module.node_expr_graph(op_id).op(id) != ExprGraph::Op::kMul) {
        continue;
      }
      assert(module.node_expr_graph(op_id).operands(id).size() == 2);
      assert(module.node_expr_graph(op_id).width(id) > 0);
      auto graph = module.node_expr_graph(op_id);
      const auto op = graph.op(id);
      const auto width = graph.width(id);
      const auto sign = graph.sign(id);
      const auto operands = graph.operands(id);

      const NodeId arithmetic_id = module.append_node(Tig::Module::NodeKind::kOp);
      for (ExprId operand : operands) {
        const auto source =
            add_node_output_expr(module, op_id, operand, create_temporary_name(module));
        add_node_input_expr(module, arithmetic_id, source);
      }
      const ExprId root = module.node_expr_graph(arithmetic_id)
                              .append(op, width, sign, module.node_input_expr_ids(arithmetic_id));
      const std::string output_name = create_temporary_name(module);
      module.append_node_output(arithmetic_id, output_name, width, sign);
      module.node_output_expr_ids(arithmetic_id).push_back(root);
      module.node_output_combs(arithmetic_id).push_back(true);
      append_signal(module, output_name, {{}, width, sign});

      arithmetic_ids.push_back(arithmetic_id);
      replacements[id] = add_node_input_expr(module, op_id, {arithmetic_id, 0});
      modified = true;
    }
    if (!modified) {
      continue;
    }
    for (ExprId &root : module.node_output_expr_ids(op_id)) {
      if (root < expr_count && replacements[root] != kInvalidExprId) {
        root = replacements[root];
      }
    }
    clean_op_node(module, op_id);
  }
  return arithmetic_ids;
}

bool TigTransformer::map_arithmetic(Tig::Module &module, NodeId node_id,
                                    const std::shared_ptr<const boop::CellLibrary> &library) {
  assert(module.node_kind(node_id) == Tig::Module::NodeKind::kOp);
  assert(module.node_output_expr_ids(node_id).size() == 1);
  const ExprId root = module.node_output_expr_ids(node_id).front();
  assert(root != kInvalidExprId);
  const auto op = module.node_expr_graph(node_id).op(root);
  const auto width = module.node_expr_graph(node_id).width(root);
  const auto sign = module.node_expr_graph(node_id).sign(root);
  const auto &input_exprs = module.node_expr_graph(node_id).operands(root);
  assert(op == ExprGraph::Op::kAdd || op == ExprGraph::Op::kMul);
  assert(input_exprs.size() == 2);
  assert(module.node_inputs(node_id).size() == module.node_input_expr_ids(node_id).size());

  boop::AndNetwork network;
  std::vector<std::pair<EdgeRef, SignalWidth>> input_bits;
  for (ExprId operand : input_exprs) {
    const auto position = std::ranges::find(module.node_input_expr_ids(node_id), operand);
    assert(position != module.node_input_expr_ids(node_id).end());
    const auto source = module.node_inputs(node_id)[position - module.node_input_expr_ids(node_id).begin()];
    assert(module.node_expr_graph(node_id).width(operand) > 0);
    for (SignalWidth bit = 0; bit < width; ++bit) {
      if (bit >= module.node_expr_graph(node_id).width(operand) && !sign) {
        continue;
      }
      const SignalWidth source_bit = std::min(bit, module.node_expr_graph(node_id).width(operand) - 1);
      input_bits.emplace_back(source, source_bit);
    }
  }

  size_t input = 0;
  std::vector<std::vector<int>> operands;
  operands.reserve(2);
  for (ExprId operand : input_exprs) {
    std::vector<int> bits;
    bits.reserve(width);
    for (SignalWidth bit = 0; bit < width; ++bit) {
      if (bit >= module.node_expr_graph(node_id).width(operand) && !sign) {
        bits.push_back(network.Node2Edge(network.GetConst0(), false));
      } else {
        assert(input < input_bits.size());
        bits.push_back(network.Node2Edge(network.AddPi(), false));
        ++input;
      }
    }
    operands.push_back(std::move(bits));
  }
  assert(input == input_bits.size());

  std::vector<int> outputs;
  switch (op) {
  case ExprGraph::Op::kAdd:
    outputs = create_ripple_carry_adder(network, operands[0], operands[1]);
    break;
  case ExprGraph::Op::kMul:
    outputs = create_array_multiplier(network, operands[0], operands[1]);
    break;
  default:
    assert(false);
  }
  for (int output : outputs) {
    network.AddPoEdge(output);
  }

  auto mapped = std::make_shared<boop::BoundNetwork>(library.get());
  if (!boop::MockturtleMap(&network, mapped.get())) {
    diagnostics_.error(DiagnosticId::kMappingFailed,
                       "module " + module.name + ", node_id " + std::to_string(node_id));
    return false;
  }

  std::vector<EdgeRef> mapped_inputs;
  mapped_inputs.reserve(input_bits.size());
  for (const auto &[source, bit] : input_bits) {
    ExprBuilder builder(module.node_expr_graph(source.node_id), diagnostics_);
    const ExprId selected = builder.create_static_range(
        module.node_output_expr_ids(source.node_id)[source.port_idx], bit, 1, false);
    mapped_inputs.push_back(
        add_node_output_expr(module, source.node_id, selected, create_temporary_name(module)));
  }

  assert(module.node_output_count(node_id) == 1);
  const auto word_name = module.node_output_name(node_id, 0);
  const auto word_width = module.node_output_width(node_id, 0);
  const auto word_sign = module.node_output_sign(node_id, 0);
  module.set_node_kind(node_id, Tig::Module::NodeKind::kBoundNetwork);
  module.node_name(node_id) = create_temporary_name(module);
  module.node_inputs(node_id) = std::move(mapped_inputs);
  module.clear_node_outputs(node_id);
  module.node_output_combs(node_id).clear();
  module.node_bound_network(node_id) = std::move(mapped);
  module.node_cell_library(node_id) = library;
  assert(width <= std::numeric_limits<PortIndex>::max());
  for (SignalWidth bit = 0; bit < width; ++bit) {
    const std::string name = create_temporary_name(module);
    module.append_node_output(node_id, name, 1, false);
    append_signal(module, name, {{}, 1, false});
  }

  const NodeId fold_id = module.append_node(Tig::Module::NodeKind::kFold);
  apply_replacements(module, {{{node_id, 0}, {fold_id, 0}}});
  module.node_inputs(fold_id).reserve(width);
  for (SignalWidth bit = 0; bit < width; ++bit) {
    module.node_inputs(fold_id).push_back({node_id, static_cast<PortIndex>(bit)});
  }
  module.append_node_output(fold_id, word_name, word_width, word_sign);
  module.node_output_combs(fold_id).push_back(true);
  return true;
}

bool TigTransformer::arith(std::shared_ptr<const boop::CellLibrary> library) {
  assert(library);
  bool success = true;
  for (auto &module : design_.modules) {
    for (NodeId node_id : extract_arithmetic(module)) {
      success &= map_arithmetic(module, node_id, library);
    }
  }
  return success;
}

} // namespace abys::ir
