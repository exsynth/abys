#include "abys/ir/tig_transformer.h"

#include <algorithm>
#include <cassert>
#include <optional>
#include <utility>
#include <vector>

#include "abys/ir/expr_builder.h"
#include "boop/apps/fast.h"
#include "boop/network/and_network.h"

namespace abys::ir {
namespace {

class BoopBridge {
public:
  BoopBridge(Tig::Module::Node &node, Diagnostics &diagnostics, bool bus)
      : node_(node), diagnostics_(diagnostics), bus_(bus), literals_(node.expr_graph.nodes.size()),
        opaque_(node.expr_graph.nodes.size(), false),
        is_output_(node.expr_graph.nodes.size(), false) {}

  bool run() {
    create_network();
    if (outputs_.empty()) {
      return false;
    }
    boop::RunFast(&network_);
    rebuild_expr_graph();
    return true;
  }

private:
  struct Input {
    ExprId id;
    SignalWidth bit;
  };

  struct Output {
    ExprId id;
    SignalWidth width;
    bool sign;
    std::optional<ExprGraph::UnpackedProperties> properties;
  };

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
    for (ExprId root : node_.expr_roots) {
      if (root != kInvalidExprId) {
        create_output(root);
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

  void create_output(ExprId id) {
    assert(id < is_output_.size());
    if (opaque_[id] || is_output_[id]) {
      return;
    }
    is_output_[id] = true;
    for (const int literal : literals_[id]) {
      network_.AddPoEdge(literal);
    }
    outputs_.push_back({id, width(id), node_.expr_graph.nodes[id].sign, properties(id)});
  }

  ExprId get_expr_id(ExprBuilder &builder, const std::vector<ExprId> &expr_ids, int id,
                     bool complement) const {
    assert(id >= 0 && id < static_cast<int>(expr_ids.size()));
    ExprId expr_id = expr_ids[id];
    assert(expr_id != kInvalidExprId);
    if (!complement) {
      return expr_id;
    }
    if (expr_id == ExprGraph::constant_zero) {
      return ExprGraph::constant_one;
    }
    if (expr_id == ExprGraph::constant_one) {
      return ExprGraph::constant_zero;
    }
    return builder.create_bitwise_not(expr_id);
  }

  ExprId repeat(ExprBuilder &builder, ExprId id, SignalWidth count) const {
    assert(count > 0);
    if (count == 1) {
      return id;
    }
    return builder.create_concat(std::vector<ExprId>(count, id), false);
  }

  void rebuild_expr_graph() {
    const ExprId expression_count = static_cast<ExprId>(node_.expr_graph.nodes.size());
    ExprBuilder builder(node_.expr_graph, diagnostics_);
    std::vector<ExprId> expr_ids(network_.GetNumNodes(), kInvalidExprId);
    expr_ids[network_.GetConst0()] = ExprGraph::constant_zero;
    std::vector<SignalWidth> bus_widths;
    if (bus_) {
      bus_widths.assign(network_.GetNumNodes(), 0);
      bus_widths[network_.GetConst0()] = 1;
      for (int index = 0; index < network_.GetNumPis(); ++index) {
        const auto input = inputs_[index];
        ExprId source = input.id;
        if (properties(input.id)) {
          source = builder.create_unpacked_flatten(source);
        }
        expr_ids[network_.GetPi(index)] = source;
        bus_widths[network_.GetPi(index)] = width(input.id);
      }
      network_.ForEachInt([&](int id) {
        SignalWidth fanin_width = 1;
        network_.ForEachFanin(id, [&](int fanin, bool) {
          const SignalWidth input_width = bus_widths[fanin];
          assert(input_width == 1 || fanin_width == 1 || input_width == fanin_width);
          fanin_width = std::max(fanin_width, input_width);
        });
        bus_widths[id] = fanin_width;
        std::vector<ExprId> operands;
        network_.ForEachFanin(id, [&](int fanin, bool complement) {
          ExprId operand = get_expr_id(builder, expr_ids, fanin, complement);
          if (bus_widths[fanin] == 1 && fanin_width > 1) {
            operand = repeat(builder, operand, fanin_width);
          }
          operands.push_back(operand);
        });
        if (operands.empty()) {
          expr_ids[id] = ExprGraph::constant_one;
        } else if (operands.size() == 1) {
          expr_ids[id] = operands.front();
        } else {
          expr_ids[id] = builder.create_and(std::move(operands));
        }
      });
    } else {
      for (int index = 0; index < network_.GetNumPis(); ++index) {
        const auto input = inputs_[index];
        ExprId source = input.id;
        if (properties(input.id)) {
          source = builder.create_unpacked_flatten(source);
        }
        expr_ids[network_.GetPi(index)] =
            width(input.id) == 1 ? source
                                 : builder.create_static_range(source, input.bit, 1, false);
      }
      network_.ForEachInt([&](int id) {
        std::vector<ExprId> operands;
        network_.ForEachFanin(id, [&](int fanin, bool complement) {
          operands.push_back(get_expr_id(builder, expr_ids, fanin, complement));
        });
        if (operands.empty()) {
          expr_ids[id] = ExprGraph::constant_one;
        } else if (operands.size() == 1) {
          expr_ids[id] = operands.front();
        } else {
          expr_ids[id] = builder.create_and(std::move(operands));
        }
      });
    }
    std::vector<ExprId> replacements(expression_count, kInvalidExprId);
    int output_index = 0;
    for (const auto &output : outputs_) {
      std::vector<ExprId> bits;
      const SignalWidth output_count = bus_ ? 1 : output.width;
      bits.reserve(output_count);
      for (SignalWidth bit = 0; bit < output_count; ++bit) {
        const int po = network_.GetPo(output_index++);
        assert(network_.GetNumFanins(po) == 1);
        const int driver = network_.GetFanin(po, 0);
        ExprId result = get_expr_id(builder, expr_ids, driver, network_.GetCompl(po, 0));
        if (bus_) {
          assert(bus_widths[driver] == 1 || bus_widths[driver] == output.width);
          if (bus_widths[driver] == 1) {
            result = repeat(builder, result, output.width);
          }
        }
        bits.push_back(result);
      }
      ExprId result = bits.front();
      if (bits.size() > 1) {
        std::reverse(bits.begin(), bits.end());
        result = builder.create_concat(std::move(bits), output.sign);
      }
      if (output.properties) {
        result = builder.create_unpacked_fold(result, output.properties->unpacked_dims,
                                              output.properties->width, output.properties->sign);
      }
      replacements[output.id] = result;
    }
    assert(output_index == network_.GetNumPos());
    for (ExprId id = 0; id < expression_count; ++id) {
      for (ExprId &operand : node_.expr_graph.nodes[id].operands) {
        if (operand != kInvalidExprId && replacements[operand] != kInvalidExprId) {
          operand = replacements[operand];
        }
      }
    }
    for (ExprId &root : node_.expr_roots) {
      if (root != kInvalidExprId && replacements[root] != kInvalidExprId) {
        root = replacements[root];
      }
    }
  }

  Tig::Module::Node &node_;
  Diagnostics &diagnostics_;
  bool bus_;
  boop::AndNetwork network_;
  std::vector<std::vector<int>> literals_;
  std::vector<Input> inputs_;
  std::vector<bool> opaque_;
  std::vector<bool> is_output_;
  std::vector<Output> outputs_;
};

} // namespace

void TigTransformer::fast(bool bus) {
  for (auto &module : design_.modules) {
    for (auto &node : module.nodes) {
      if (node.kind == Tig::Module::NodeKind::kOp) {
        assert(is_expr_graph_topological(node));
        BoopBridge bridge(node, diagnostics_, bus);
        if (bridge.run()) {
          clean_op_node(node);
        }
      }
    }
  }
}

} // namespace abys::ir
