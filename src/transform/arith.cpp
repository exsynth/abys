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

std::vector<Tig::NodeId> TigTransformer::extract_arithmetic(Tig::Module &module) {
  std::vector<Tig::NodeId> arithmetic_ids;
  const Tig::NodeId node_count = static_cast<Tig::NodeId>(module.nodes.size());
  for (Tig::NodeId op_id = 0; op_id < node_count; ++op_id) {
    if (module.nodes[op_id].kind != Tig::Module::NodeKind::kOp) {
      continue;
    }
    assert(is_expr_graph_topological(module.nodes[op_id]));
    const ExprId expr_count = static_cast<ExprId>(module.nodes[op_id].expr_graph.nodes.size());
    std::vector<ExprId> replacements(expr_count, kInvalidExprId);
    bool modified = false;
    for (ExprId id = 0; id < expr_count; ++id) {
      auto &expr = module.nodes[op_id].expr_graph.nodes[id];
      for (ExprId &operand : expr.operands) {
        if (operand != kInvalidExprId && replacements[operand] != kInvalidExprId) {
          operand = replacements[operand];
        }
      }
      if (expr.op != ExprGraph::Op::kAdd && expr.op != ExprGraph::Op::kMul) {
        continue;
      }
      assert(expr.operands.size() == 2);
      assert(expr.width > 0);
      const ExprGraph::Node arithmetic_expr = expr;

      Tig::Module::Node arithmetic_node;
      arithmetic_node.kind = Tig::Module::NodeKind::kOp;
      for (ExprId operand : arithmetic_expr.operands) {
        const auto source =
            add_node_output_expr(module, op_id, operand, create_temporary_name(module));
        add_node_input_expr(module, arithmetic_node, source);
      }
      arithmetic_node.expr_graph.nodes.push_back({arithmetic_expr.op, arithmetic_expr.width,
                                                  arithmetic_expr.sign,
                                                  arithmetic_node.input_expr_ids});
      const ExprId root = static_cast<ExprId>(arithmetic_node.expr_graph.nodes.size() - 1);
      const std::string output_name = create_temporary_name(module);
      arithmetic_node.outputs.push_back({output_name, arithmetic_expr.width, arithmetic_expr.sign});
      arithmetic_node.expr_roots.push_back(root);
      arithmetic_node.combs.push_back(true);
      module.signals.push_back({output_name, {}, arithmetic_expr.width, arithmetic_expr.sign});

      const Tig::NodeId arithmetic_id = static_cast<Tig::NodeId>(module.nodes.size());
      module.nodes.push_back(std::move(arithmetic_node));
      arithmetic_ids.push_back(arithmetic_id);
      replacements[id] = add_node_input_expr(module, module.nodes[op_id], {arithmetic_id, 0});
      modified = true;
    }
    if (!modified) {
      continue;
    }
    for (ExprId &root : module.nodes[op_id].expr_roots) {
      if (root < expr_count && replacements[root] != kInvalidExprId) {
        root = replacements[root];
      }
    }
    clean_op_node(module.nodes[op_id]);
  }
  return arithmetic_ids;
}

bool TigTransformer::map_arithmetic(Tig::Module &module, Tig::NodeId node_id,
                                    const std::shared_ptr<const boop::CellLibrary> &library) {
  auto &node = module.nodes[node_id];
  assert(node.kind == Tig::Module::NodeKind::kOp);
  assert(node.expr_roots.size() == 1);
  assert(node.expr_roots.front() != kInvalidExprId);
  const auto expr = node.expr_graph.nodes[node.expr_roots.front()];
  assert(expr.op == ExprGraph::Op::kAdd || expr.op == ExprGraph::Op::kMul);
  assert(expr.operands.size() == 2);
  assert(node.inputs.size() == node.input_expr_ids.size());

  boop::AndNetwork network;
  std::vector<std::pair<Tig::Module::EdgeRef, SignalWidth>> input_bits;
  for (ExprId operand : expr.operands) {
    const auto position = std::ranges::find(node.input_expr_ids, operand);
    assert(position != node.input_expr_ids.end());
    const auto source = node.inputs[position - node.input_expr_ids.begin()];
    const auto &operand_expr = node.expr_graph.nodes[operand];
    assert(operand_expr.width > 0);
    for (SignalWidth bit = 0; bit < expr.width; ++bit) {
      if (bit >= operand_expr.width && !expr.sign) {
        continue;
      }
      const SignalWidth source_bit = std::min(bit, operand_expr.width - 1);
      input_bits.emplace_back(source, source_bit);
    }
  }

  size_t input = 0;
  std::vector<std::vector<int>> operands;
  operands.reserve(2);
  for (ExprId operand : expr.operands) {
    const auto &operand_expr = node.expr_graph.nodes[operand];
    std::vector<int> bits;
    bits.reserve(expr.width);
    for (SignalWidth bit = 0; bit < expr.width; ++bit) {
      if (bit >= operand_expr.width && !expr.sign) {
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
  switch (expr.op) {
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
                       "module " + module.name + ", node " + std::to_string(node_id));
    return false;
  }

  std::vector<Tig::Module::EdgeRef> mapped_inputs;
  mapped_inputs.reserve(input_bits.size());
  for (const auto &[source, bit] : input_bits) {
    ExprBuilder builder(module.nodes[source.node_id].expr_graph, diagnostics_);
    const ExprId selected = builder.create_static_range(
        module.nodes[source.node_id].expr_roots[source.port_idx], bit, 1, false);
    mapped_inputs.push_back(
        add_node_output_expr(module, source.node_id, selected, create_temporary_name(module)));
  }

  assert(node.outputs.size() == 1);
  const auto word_output = node.outputs.front();
  node.kind = Tig::Module::NodeKind::kBoundNetwork;
  node.name = create_temporary_name(module);
  node.inputs = std::move(mapped_inputs);
  node.input_expr_ids.clear();
  node.outputs.clear();
  node.expr_roots.clear();
  node.combs.clear();
  node.expr_graph = {};
  node.bound_network = std::move(mapped);
  node.cell_library = library;
  assert(expr.width <= std::numeric_limits<PortIndex>::max());
  for (SignalWidth bit = 0; bit < expr.width; ++bit) {
    const std::string name = create_temporary_name(module);
    node.outputs.push_back({name, 1, false});
    module.signals.push_back({name, {}, 1, false});
  }

  Tig::Module::Node fold;
  fold.kind = Tig::Module::NodeKind::kFold;
  fold.inputs.reserve(expr.width);
  for (SignalWidth bit = 0; bit < expr.width; ++bit) {
    fold.inputs.push_back({node_id, static_cast<PortIndex>(bit)});
  }
  fold.outputs.push_back(word_output);
  fold.expr_roots.push_back(kInvalidExprId);
  fold.combs.push_back(true);
  const Tig::NodeId fold_id = static_cast<Tig::NodeId>(module.nodes.size());
  apply_replacements(module, {{{node_id, 0}, {fold_id, 0}}});
  module.nodes.push_back(std::move(fold));
  return true;
}

bool TigTransformer::arith(std::shared_ptr<const boop::CellLibrary> library) {
  assert(library);
  bool success = true;
  for (auto &module : design_.modules) {
    for (Tig::NodeId node_id : extract_arithmetic(module)) {
      success &= map_arithmetic(module, node_id, library);
    }
  }
  return success;
}

} // namespace abys::ir
