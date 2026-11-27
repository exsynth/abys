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
    std::optional<UnpackedShape> shape;
    std::optional<PortIndex> root_port;
  };

  BoopBridge(const Tig::Module &module, NodeId node_id, bool bus)
      : module_(module), node_id_(node_id), bus_(bus),
        literals_(module.node_expr_graph(node_id).size()),
        opaque_(module.node_expr_graph(node_id).size(), false),
        is_output_(module.node_expr_graph(node_id).size(), false) {}

  bool build() {
    create_network();
    return !outputs_.empty();
  }

  const boop::AndNetwork &network() const { return network_; }
  const std::vector<Input> &inputs() const { return inputs_; }
  const std::vector<Output> &outputs() const { return outputs_; }

private:
  std::optional<UnpackedShape> shape(ExprId id) const {
    if (const auto *shape = module_.node_expr_graph(node_id_).unpacked_shape(id)) {
      return *shape;
    }
    return std::nullopt;
  }

  SignalWidth width(ExprId id) const {
    const auto unpacked = shape(id);
    return unpacked ? unpacked->flattened_width() : module_.node_expr_graph(node_id_).outer_dim(id);
  }

  void create_network() {
    for (ExprId id = 0; id < literals_.size(); ++id) {
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
      switch (module_.node_expr_graph(node_id_).op(id)) {
      case ExprGraph::Op::kBitwiseNot: {
        assert(module_.node_expr_graph(node_id_).operands(id).size() == 1);
        result = literals_[module_.node_expr_graph(node_id_).operands(id).front()];
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
        assert(!module_.node_expr_graph(node_id_).operands(id).empty());
        result = literals_[module_.node_expr_graph(node_id_).operands(id).front()];
        assert(result.size() == result_width);
        for (size_t index = 1; index < module_.node_expr_graph(node_id_).operands(id).size();
             ++index) {
          const auto &next = literals_[module_.node_expr_graph(node_id_).operands(id)[index]];
          assert(next.size() == result_width);
          for (SignalWidth bit = 0; bit < result_width; ++bit) {
            if (module_.node_expr_graph(node_id_).op(id) == ExprGraph::Op::kAnd) {
              result[bit] = network_.AddAndEdge(result[bit], next[bit]);
            } else if (module_.node_expr_graph(node_id_).op(id) == ExprGraph::Op::kOr) {
              result[bit] = network_.AddOrEdge(result[bit], next[bit]);
            } else {
              result[bit] = network_.AddXorEdge(result[bit], next[bit]);
            }
          }
        }
        break;
      }
      case ExprGraph::Op::kMux: {
        assert(module_.node_expr_graph(node_id_).operands(id).size() == 3);
        const auto &condition = literals_[module_.node_expr_graph(node_id_).operands(id)[0]];
        const auto &then_literals = literals_[module_.node_expr_graph(node_id_).operands(id)[1]];
        const auto &else_literals = literals_[module_.node_expr_graph(node_id_).operands(id)[2]];
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
        assert(module_.node_expr_graph(node_id_).operands(id).size() >= 3);
        assert(module_.node_expr_graph(node_id_).operands(id).size() % 2 == 1);
        result = literals_[module_.node_expr_graph(node_id_).operands(id).back()];
        assert(result.size() == result_width);
        for (size_t index = module_.node_expr_graph(node_id_).operands(id).size() - 1; index > 0;
             index -= 2) {
          const auto &condition =
              literals_[module_.node_expr_graph(node_id_).operands(id)[index - 2]];
          const auto &data = literals_[module_.node_expr_graph(node_id_).operands(id)[index - 1]];
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
      for (ExprId operand : module_.node_expr_graph(node_id_).operands(id)) {
        if (operand != kInvalidExprId) {
          create_output(operand);
        }
      }
    }
    for (PortIndex port = 0; port < module_.node_output_expr_ids(node_id_).size(); ++port) {
      const ExprId root = module_.node_output_expr_ids(node_id_)[port];
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
    outputs_.push_back(
        {id, width(id), module_.node_expr_graph(node_id_).outer_sign(id), shape(id), root_port});
  }

  const Tig::Module &module_;
  NodeId node_id_;
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
  using NodeKind = Tig::Module::NodeKind;

  const size_t node_count = module.size();
  Replacements edge_replacements;
  for (NodeId op_id = 0; op_id < node_count; ++op_id) {
    if (module.node_kind(op_id) != NodeKind::kOp) {
      continue;
    }
    assert(is_expr_graph_topological(module, op_id));
    BoopBridge bridge(module, op_id, bus);
    if (!bridge.build()) {
      continue;
    }
    const NodeId and_id = module.append_node(NodeKind::kAndNetwork);
    module.node_and_network(and_id) = std::make_shared<boop::AndNetwork>(bridge.network());

    ExprBuilder builder(module.node_expr_graph(op_id), diagnostics_);
    for (const auto &input : bridge.inputs()) {
      EdgeRef input_ref;
      if ((bus || module.node_expr_graph(op_id).outer_dim(input.id) == 1) &&
          module.node_expr_graph(op_id).op(input.id) == ExprGraph::Op::kInput &&
          !get_unpacked_shape(module, op_id, input.id)) {
        const auto position = std::ranges::find(module.node_input_expr_ids(op_id), input.id);
        if (position != module.node_input_expr_ids(op_id).end()) {
          const size_t index = position - module.node_input_expr_ids(op_id).begin();
          input_ref = module.node_inputs(op_id)[index];
        }
      }
      if (input_ref.node_id == kInvalidNodeId) {
        ExprId source = input.id;
        if (get_unpacked_shape(module, op_id, source)) {
          source = builder.create_unpacked_flatten(source);
        }
        if (!bus && module.node_expr_graph(op_id).outer_dim(source) > 1) {
          source = builder.create_static_range(source, input.bit, 1, false);
        }
        input_ref = add_node_output_expr(module, op_id, source, create_temporary_name(module));
      }
      module.node_inputs(and_id).push_back(input_ref);
    }

    std::vector<ExprId> expr_replacements(module.node_expr_graph(op_id).size(), kInvalidExprId);
    PortIndex and_port = 0;
    for (const auto &output : bridge.outputs()) {
      const SignalWidth port_count = bus ? 1 : output.width;
      for (SignalWidth bit = 0; bit < port_count; ++bit) {
        std::string name;
        if (output.root_port && (bus || output.width == 1)) {
          name = module.node_output_name(op_id, *output.root_port);
        }
        if (name.empty()) {
          name = create_temporary_name(module);
          const auto *shape = bus && output.shape ? &*output.shape : nullptr;
          const auto outer_dim = bus ? module.node_expr_graph(op_id).outer_dim(output.id) : 1;
          append_signal(module, name, outer_dim, bus && output.sign, shape);
        }
        module.append_node_output(and_id, std::move(name), bus ? output.width : 1,
                                  bus && output.sign);
      }

      if (output.root_port) {
        if (bus || output.width == 1) {
          edge_replacements.emplace(std::pair{op_id, *output.root_port}, EdgeRef{and_id, and_port});
        }
        and_port += port_count;
        continue;
      }
      and_port += port_count;
    }
    assert(and_port == module.node_output_count(and_id));
    assert(module.node_and_network(and_id)->GetNumPis() ==
           static_cast<int>(module.node_inputs(and_id).size()));
    assert(module.node_and_network(and_id)->GetNumPos() ==
           static_cast<int>(module.node_output_count(and_id)));

    and_port = 0;
    for (const auto &output : bridge.outputs()) {
      if (output.root_port) {
        const SignalWidth port_count = bus ? 1 : output.width;
        if (!bus && output.width > 1) {
          const NodeId fold_id = module.append_node(NodeKind::kFold);
          module.node_inputs(fold_id).reserve(output.width);
          for (SignalWidth bit = 0; bit < output.width; ++bit) {
            module.node_inputs(fold_id).push_back({and_id, static_cast<PortIndex>(and_port + bit)});
          }
          std::string name = module.node_output_name(op_id, *output.root_port);
          if (name.empty()) {
            name = create_temporary_name(module);
            append_signal(module, name, module.node_expr_graph(op_id).outer_dim(output.id),
                          output.sign, output.shape ? &*output.shape : nullptr);
          }
          module.append_node_output(fold_id, name,
                                    module.node_output_outer_dim(op_id, *output.root_port),
                                    module.node_output_outer_sign(op_id, *output.root_port));
          module.node_output_combs(fold_id).push_back(true);
          edge_replacements.emplace(std::pair{op_id, *output.root_port}, EdgeRef{fold_id, 0});
        }
        module.node_output_name(op_id, *output.root_port).clear();
        module.node_output_expr_ids(op_id)[*output.root_port] = kInvalidExprId;
        and_port += port_count;
        continue;
      }
      const SignalWidth port_count = bus ? 1 : output.width;
      std::vector<ExprId> bits;
      bits.reserve(port_count);
      for (SignalWidth bit = 0; bit < port_count; ++bit) {
        bits.push_back(add_node_input_expr(module, op_id, {and_id, and_port++}));
      }
      ExprId result = bits.front();
      if (!bus && bits.size() > 1) {
        std::reverse(bits.begin(), bits.end());
        result = builder.create_concat(std::move(bits), output.sign);
      }
      if (output.shape) {
        result = builder.create_unpacked_fold(result, output.shape->unpacked_dims,
                                              output.shape->inner_dim, output.shape->inner_sign);
      }
      expr_replacements[output.id] = result;
    }

    const ExprId expr_count = static_cast<ExprId>(expr_replacements.size());
    for (ExprId id = 0; id < expr_count; ++id) {
      for (ExprId &operand : module.node_expr_graph(op_id).mutable_operands(id)) {
        if (operand != kInvalidExprId && operand < expr_count &&
            expr_replacements[operand] != kInvalidExprId) {
          operand = expr_replacements[operand];
        }
      }
    }
    clean_op_node(module, op_id);
  }
  apply_replacements(module, edge_replacements);
}

void TigTransformer::aig(bool bus) {
  for (auto &module : design_.modules) {
    create_and_networks(module, bus);
  }
}

} // namespace abys::ir
