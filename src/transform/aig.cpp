#include "abys/ir/tig_transformer.h"

#include <algorithm>
#include <cassert>
#include <optional>
#include <utility>
#include <vector>

#include "abys/ir/expr_builder.h"
#include "boop/network/and_network.h"

namespace abys::ir {
namespace {

class BoopBridge {
public:
  struct Input {
    ExprId id;
    SignalWidth bit;
  };

  struct Output {
    ExprId id;
    SignalWidth width;
    bool sign;
    std::optional<ExprGraph::UnpackedProperties> properties;
    std::optional<PortIndex> root_port;
  };

  BoopBridge(Tig::Module::Node &node, bool bus)
      : node_(node), bus_(bus), literals_(node.expr_graph.nodes.size()),
        opaque_(node.expr_graph.nodes.size(), false),
        is_output_(node.expr_graph.nodes.size(), false) {}

  bool build() {
    create_network();
    return !outputs_.empty();
  }

  const boop::AndNetwork &network() const { return network_; }
  const std::vector<Input> &inputs() const { return inputs_; }
  const std::vector<Output> &outputs() const { return outputs_; }

private:
  std::optional<ExprGraph::UnpackedProperties> properties(ExprId id) const {
    for (const auto &properties : node_.expr_graph.unpacked_properties) {
      if (properties.id == id) {
        return properties;
      }
    }
    return std::nullopt;
  }

  SignalWidth width(ExprId id) const {
    const auto unpacked = properties(id);
    return unpacked ? unpacked->flattened_width() : node_.expr_graph.nodes[id].width;
  }

  void create_network() {
    for (ExprId id = 0; id < literals_.size(); ++id) {
      const auto &expr = node_.expr_graph.nodes[id];
      const SignalWidth result_width = bus_ ? 1 : width(id);
      if (id == ExprGraph::constant_zero) {
        literals_[id] = {network_.Node2Edge(network_.GetConst0(), false)};
        continue;
      }
      if (id == ExprGraph::constant_one) {
        literals_[id] = {network_.Node2Edge(network_.GetConst0(), true)};
        continue;
      }

      std::vector<int> result;
      bool supported = true;
      switch (expr.op) {
      case ExprGraph::Op::kBitwiseNot: {
        assert(expr.operands.size() == 1);
        result = literals_[expr.operands.front()];
        assert(result.size() == result_width);
        for (int &literal : result) {
          literal = network_.ComplEdge(literal);
        }
        break;
      }
      case ExprGraph::Op::kAnd:
      case ExprGraph::Op::kOr:
      case ExprGraph::Op::kXor: {
        // TODO: balanced tree?
        assert(!expr.operands.empty());
        result = literals_[expr.operands.front()];
        assert(result.size() == result_width);
        for (size_t index = 1; index < expr.operands.size(); ++index) {
          const auto &next = literals_[expr.operands[index]];
          assert(next.size() == result_width);
          for (SignalWidth bit = 0; bit < result_width; ++bit) {
            if (expr.op == ExprGraph::Op::kAnd) {
              result[bit] = network_.AddAndEdge(result[bit], next[bit]);
            } else if (expr.op == ExprGraph::Op::kOr) {
              result[bit] = network_.AddOrEdge(result[bit], next[bit]);
            } else {
              result[bit] = network_.AddXorEdge(result[bit], next[bit]);
            }
          }
        }
        break;
      }
      case ExprGraph::Op::kMux: {
        assert(expr.operands.size() == 3);
        const auto &condition = literals_[expr.operands[0]];
        const auto &then_literals = literals_[expr.operands[1]];
        const auto &else_literals = literals_[expr.operands[2]];
        assert(condition.size() == 1);
        assert(then_literals.size() == result_width);
        assert(else_literals.size() == result_width);
        result.reserve(result_width);
        for (SignalWidth bit = 0; bit < result_width; ++bit) {
          result.push_back(
              network_.AddMuxEdge(condition.front(), then_literals[bit], else_literals[bit]));
        }
        break;
      }
      case ExprGraph::Op::kPmux: {
        // TODO: balanced tree? (if exclusive, then two-level logic)
        assert(expr.operands.size() >= 3);
        assert(expr.operands.size() % 2 == 1);
        result = literals_[expr.operands.back()];
        assert(result.size() == result_width);
        for (size_t index = expr.operands.size() - 1; index > 0; index -= 2) {
          const auto &condition = literals_[expr.operands[index - 2]];
          const auto &data = literals_[expr.operands[index - 1]];
          assert(condition.size() == 1);
          assert(data.size() == result_width);
          for (SignalWidth bit = 0; bit < result_width; ++bit) {
            result[bit] = network_.AddMuxEdge(condition.front(), data[bit], result[bit]);
          }
        }
        break;
      }
      default:
        supported = false;
        break;
      }
      if (!supported) {
        opaque_[id] = true;
        result = create_inputs(id);
      }
      assert(result.size() == result_width);
      literals_[id] = std::move(result);
    }
    for (ExprId id = 0; id < literals_.size(); ++id) {
      if (!opaque_[id]) {
        continue;
      }
      for (ExprId operand : node_.expr_graph.nodes[id].operands) {
        if (operand != kInvalidExprId) {
          create_output(operand);
        }
      }
    }
    for (PortIndex port = 0; port < node_.expr_roots.size(); ++port) {
      const ExprId root = node_.expr_roots[port];
      if (root != kInvalidExprId) {
        create_output(root, port);
      }
    }
  }

  std::vector<int> create_inputs(ExprId id) {
    std::vector<int> result;
    if (bus_) {
      inputs_.push_back({id, 0});
      result.push_back(network_.Node2Edge(network_.AddPi(), false));
      return result;
    }
    result.reserve(width(id));
    for (SignalWidth bit = 0; bit < width(id); ++bit) {
      inputs_.push_back({id, bit});
      result.push_back(network_.Node2Edge(network_.AddPi(), false));
    }
    return result;
  }

  void create_output(ExprId id, std::optional<PortIndex> root_port = std::nullopt) {
    assert(id < is_output_.size());
    if (opaque_[id] || id == ExprGraph::constant_zero || id == ExprGraph::constant_one ||
        (!root_port && is_output_[id])) {
      return;
    }
    is_output_[id] = true;
    for (const int literal : literals_[id]) {
      network_.AddPoEdge(literal);
    }
    outputs_.push_back({id, width(id), node_.expr_graph.nodes[id].sign, properties(id), root_port});
  }

  Tig::Module::Node &node_;
  bool bus_;
  boop::AndNetwork network_;
  std::vector<std::vector<int>> literals_;
  std::vector<Input> inputs_;
  std::vector<bool> opaque_;
  std::vector<bool> is_output_;
  std::vector<Output> outputs_;
};

} // namespace

void TigTransformer::create_and_networks(Tig::Module &module, bool bus) {
  using EdgeRef = Tig::Module::EdgeRef;
  using NodeKind = Tig::Module::NodeKind;

  const size_t node_count = module.nodes.size();
  size_t additional_nodes = 0;
  for (const auto &node : module.nodes) {
    if (node.kind == NodeKind::kOp) {
      additional_nodes += 1 + node.outputs.size();
    }
  }
  module.nodes.reserve(node_count + additional_nodes);
  Replacements edge_replacements;
  for (Tig::NodeId op_id = 0; op_id < node_count; ++op_id) {
    auto &op_node = module.nodes[op_id];
    if (op_node.kind != NodeKind::kOp) {
      continue;
    }
    assert(is_expr_graph_topological(op_node));
    BoopBridge bridge(op_node, bus);
    if (!bridge.build()) {
      continue;
    }
    Tig::Module::Node and_node;
    and_node.kind = NodeKind::kAndNetwork;
    and_node.and_network = std::make_shared<boop::AndNetwork>(bridge.network());

    ExprBuilder builder(op_node.expr_graph, diagnostics_);
    for (const auto &input : bridge.inputs()) {
      EdgeRef input_ref;
      if ((bus || op_node.expr_graph.nodes[input.id].width == 1) &&
          op_node.expr_graph.nodes[input.id].op == ExprGraph::Op::kInput &&
          !get_unpacked_properties(op_node, input.id)) {
        const auto position = std::ranges::find(op_node.input_expr_ids, input.id);
        if (position != op_node.input_expr_ids.end()) {
          const size_t index = position - op_node.input_expr_ids.begin();
          input_ref = op_node.inputs[index];
        }
      }
      if (input_ref.node_id == Tig::kInvalidNodeId) {
        ExprId source = input.id;
        if (get_unpacked_properties(op_node, source)) {
          source = builder.create_unpacked_flatten(source);
        }
        if (!bus && op_node.expr_graph.nodes[source].width > 1) {
          source = builder.create_static_range(source, input.bit, 1, false);
        }
        input_ref = add_node_output_expr(module, op_id, source, create_temporary_name(module));
      }
      and_node.inputs.push_back(input_ref);
    }

    std::vector<ExprId> expr_replacements(op_node.expr_graph.nodes.size(), kInvalidExprId);
    PortIndex and_port = 0;
    for (const auto &output : bridge.outputs()) {
      const SignalWidth port_count = bus ? 1 : output.width;
      for (SignalWidth bit = 0; bit < port_count; ++bit) {
        std::string name;
        if (output.root_port && (bus || output.width == 1)) {
          name = op_node.outputs[*output.root_port].name;
        }
        if (name.empty()) {
          name = create_temporary_name(module);
          module.signals.push_back(
              {name,
               bus && output.properties ? output.properties->unpacked_dims
                                        : std::vector<SignalWidth>{},
               bus ? (output.properties ? output.properties->width : output.width) : 1,
               bus && output.sign});
        }
        and_node.outputs.push_back({std::move(name), bus ? output.width : 1, bus && output.sign});
      }

      if (output.root_port) {
        if (bus || output.width == 1) {
          edge_replacements.emplace(
              std::pair{op_id, *output.root_port},
              EdgeRef{static_cast<Tig::NodeId>(module.nodes.size()), and_port});
        }
        and_port += port_count;
        continue;
      }
      and_port += port_count;
    }
    assert(and_port == and_node.outputs.size());
    assert(and_node.and_network->GetNumPis() == static_cast<int>(and_node.inputs.size()));
    assert(and_node.and_network->GetNumPos() == static_cast<int>(and_node.outputs.size()));

    const Tig::NodeId and_id = static_cast<Tig::NodeId>(module.nodes.size());
    module.nodes.push_back(std::move(and_node));

    and_port = 0;
    for (const auto &output : bridge.outputs()) {
      if (output.root_port) {
        const SignalWidth port_count = bus ? 1 : output.width;
        if (!bus && output.width > 1) {
          Tig::Module::Node fold;
          fold.kind = NodeKind::kFold;
          fold.inputs.reserve(output.width);
          for (SignalWidth bit = 0; bit < output.width; ++bit) {
            fold.inputs.push_back({and_id, static_cast<PortIndex>(and_port + bit)});
          }
          std::string name = op_node.outputs[*output.root_port].name;
          if (name.empty()) {
            name = create_temporary_name(module);
            module.signals.push_back(
                {name,
                 output.properties ? output.properties->unpacked_dims : std::vector<SignalWidth>{},
                 output.properties ? output.properties->width : output.width,
                 output.properties ? output.properties->sign : output.sign});
          }
          fold.outputs.push_back({name, op_node.outputs[*output.root_port].width,
                                  op_node.outputs[*output.root_port].sign});
          fold.expr_roots.push_back(kInvalidExprId);
          fold.combs.push_back(true);
          const Tig::NodeId fold_id = static_cast<Tig::NodeId>(module.nodes.size());
          module.nodes.push_back(std::move(fold));
          edge_replacements.emplace(std::pair{op_id, *output.root_port}, EdgeRef{fold_id, 0});
        }
        op_node.outputs[*output.root_port].name.clear();
        op_node.expr_roots[*output.root_port] = kInvalidExprId;
        and_port += port_count;
        continue;
      }
      const SignalWidth port_count = bus ? 1 : output.width;
      std::vector<ExprId> bits;
      bits.reserve(port_count);
      for (SignalWidth bit = 0; bit < port_count; ++bit) {
        bits.push_back(add_node_input_expr(module, module.nodes[op_id], {and_id, and_port++}));
      }
      ExprId result = bits.front();
      if (!bus && bits.size() > 1) {
        std::reverse(bits.begin(), bits.end());
        result = builder.create_concat(std::move(bits), output.sign);
      }
      if (output.properties) {
        result = builder.create_unpacked_fold(result, output.properties->unpacked_dims,
                                              output.properties->width, output.properties->sign);
      }
      expr_replacements[output.id] = result;
    }

    const ExprId expr_count = static_cast<ExprId>(expr_replacements.size());
    for (ExprId id = 0; id < expr_count; ++id) {
      for (ExprId &operand : module.nodes[op_id].expr_graph.nodes[id].operands) {
        if (operand != kInvalidExprId && operand < expr_count &&
            expr_replacements[operand] != kInvalidExprId) {
          operand = expr_replacements[operand];
        }
      }
    }
    clean_op_node(module.nodes[op_id]);
  }
  apply_replacements(module, edge_replacements);
}

void TigTransformer::aig(bool bus) {
  for (auto &module : design_.modules) {
    create_and_networks(module, bus);
  }
}

} // namespace abys::ir
