#include "abys/ir/tig_transformer.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

#include "abys/ir/expr_builder.h"

namespace abys::ir {

void TigTransformer::blast_expr_graph(Tig::Module &module, NodeId node_id,
                                      std::vector<std::vector<ExprId>> blasted_ids) {
  assert(module.node_kind(node_id) == Tig::Module::NodeKind::kOp);
  auto &graph = module.node_expr_graph(node_id);
  const size_t expr_count = blasted_ids.size();
  assert(expr_count <= nodes.size());
  ExprBuilder builder(module.node_expr_graph(node_id), diagnostics_);
  std::vector<ExprId> word_outputs(expr_count, kInvalidExprId);
  std::vector<bool> propagate_unpacked_assign(expr_count, false);
  const auto find_or_create_word_output = [&](ExprId id) {
    if (id == kInvalidExprId) {
      return id;
    }
    assert(id < expr_count);
    if (word_outputs[id] != kInvalidExprId) {
      return word_outputs[id];
    }
    assert(!blasted_ids[id].empty());
    if (blasted_ids[id].size() == 1 &&
        (blasted_ids[id].front() == id || !get_unpacked_shape(module, node_id, id))) {
      word_outputs[id] = blasted_ids[id].front();
      return word_outputs[id];
    }
    ExprId result = blasted_ids[id].front();
    if (blasted_ids[id].size() > 1) {
      std::vector<ExprId> bits = blasted_ids[id];
      std::reverse(bits.begin(), bits.end());
      result = builder.create_concat(std::move(bits), graph.outer_sign(id));
    }
    if (const auto shape = get_unpacked_shape(module, node_id, id)) {
      result = builder.create_unpacked_fold(result, shape->unpacked_dims, shape->inner_dim,
                                            shape->inner_sign);
    }
    word_outputs[id] = result;
    return result;
  };
  const auto update_word_operands = [&](ExprId id, const std::vector<ExprId> &expr_operands) {
    std::vector<ExprId> operands;
    operands.reserve(expr_operands.size());
    for (ExprId operand : expr_operands) {
      operands.push_back(find_or_create_word_output(operand));
    }
    graph.set_operands(id, std::move(operands));
  };
  const auto blast_word_output = [&](ExprId id, const std::vector<ExprId> &expr_operands,
                                     SignalWidth width, std::vector<ExprId> &bits) {
    update_word_operands(id, expr_operands);
    word_outputs[id] = id;
    const ExprId data =
        get_unpacked_shape(module, node_id, id) ? builder.create_unpacked_flatten(id) : id;
    for (SignalWidth bit = 0; bit < width; ++bit) {
      bits.push_back(builder.create_static_range(data, bit, 1, false));
    }
  };
  for (ExprId id = 0; id < expr_count; ++id) {
    if (!blasted_ids[id].empty()) {
      continue;
    }
    const auto expr_operands = graph.operands(id);
    const auto shape = get_unpacked_shape(module, node_id, id);
    const SignalWidth width = shape ? shape->flattened_width() : graph.outer_dim(id);
    auto &bits = blasted_ids[id];
    bits.reserve(width);
    switch (graph.op(id)) {
    case ExprGraph::Op::kConcat:
      for (auto operand = expr_operands.rbegin(); operand != expr_operands.rend(); ++operand) {
        bits.insert(bits.end(), blasted_ids[*operand].begin(), blasted_ids[*operand].end());
      }
      break;
    case ExprGraph::Op::kConvert: {
      assert(expr_operands.size() == 1);
      const auto &operand = blasted_ids[expr_operands.front()];
      assert(!operand.empty());
      bits = operand;
      bits.resize(width, graph.outer_sign(expr_operands.front()) ? operand.back()
                                                                 : ExprGraph::constant_zero);
      break;
    }
    case ExprGraph::Op::kReverse:
      assert(expr_operands.size() == 1);
      bits.assign(blasted_ids[expr_operands.front()].rbegin(),
                  blasted_ids[expr_operands.front()].rend());
      break;
    case ExprGraph::Op::kMux:
    case ExprGraph::Op::kUnpackedMux: {
      assert(expr_operands.size() == 3);
      if ((expr_operands[1] != kInvalidExprId && propagate_unpacked_assign[expr_operands[1]]) ||
          (expr_operands[2] != kInvalidExprId && propagate_unpacked_assign[expr_operands[2]])) {
        blast_word_output(id, expr_operands, width, bits);
        propagate_unpacked_assign[id] = true;
        break;
      }
      assert(blasted_ids[expr_operands[0]].size() == 1);
      const ExprId condition = blasted_ids[expr_operands[0]].front();
      assert(expr_operands[1] == kInvalidExprId || blasted_ids[expr_operands[1]].size() == width);
      assert(expr_operands[2] == kInvalidExprId || blasted_ids[expr_operands[2]].size() == width);
      for (SignalWidth bit = 0; bit < width; ++bit) {
        const ExprId then_id = expr_operands[1] == kInvalidExprId
                                   ? kInvalidExprId
                                   : blasted_ids[expr_operands[1]][bit];
        const ExprId else_id = expr_operands[2] == kInvalidExprId
                                   ? kInvalidExprId
                                   : blasted_ids[expr_operands[2]][bit];
        bits.push_back(then_id == kInvalidExprId && else_id == kInvalidExprId
                           ? kInvalidExprId
                           : builder.create_mux(condition, then_id, else_id));
      }
      break;
    }
    case ExprGraph::Op::kCase:
    case ExprGraph::Op::kUnpackedCase: {
      bool retain = false;
      for (size_t operand = 2; operand < expr_operands.size(); operand += 2) {
        retain |= expr_operands[operand] != kInvalidExprId &&
                  propagate_unpacked_assign[expr_operands[operand]];
      }
      if (expr_operands.size() % 2 == 0) {
        retain |= expr_operands.back() != kInvalidExprId &&
                  propagate_unpacked_assign[expr_operands.back()];
      }
      if (retain) {
        blast_word_output(id, expr_operands, width, bits);
        propagate_unpacked_assign[id] = true;
        break;
      }
      const ExprId selector = find_or_create_word_output(expr_operands.front());
      std::vector<ExprId> conditions;
      for (size_t operand = 1; operand + 1 < expr_operands.size(); operand += 2) {
        conditions.push_back(
            builder.create_match(selector, find_or_create_word_output(expr_operands[operand])));
      }
      for (SignalWidth bit = 0; bit < width; ++bit) {
        std::vector<ExprId> data;
        size_t operand = 1;
        while (operand + 1 < expr_operands.size()) {
          const ExprId data_id = expr_operands[operand + 1];
          data.push_back(data_id == kInvalidExprId ? kInvalidExprId : blasted_ids[data_id][bit]);
          operand += 2;
        }
        if (operand < expr_operands.size()) {
          const ExprId data_id = expr_operands[operand];
          data.push_back(data_id == kInvalidExprId ? kInvalidExprId : blasted_ids[data_id][bit]);
        }
        bits.push_back(
            std::ranges::all_of(data, [](ExprId data_id) { return data_id == kInvalidExprId; })
                ? kInvalidExprId
                : builder.create_pmux(conditions, std::move(data)));
      }
      break;
    }
    case ExprGraph::Op::kRange: {
      const auto base = builder.decode_affine_index(id);
      const auto &data = blasted_ids[expr_operands.front()];
      assert(data.size() >= width);
      if (base.terms.empty()) {
        assert(base.offset >= 0);
        assert(static_cast<SignalWidth>(base.offset) <= data.size() - width);
        bits.insert(bits.end(), data.begin() + base.offset, data.begin() + base.offset + width);
        break;
      }
      blast_word_output(id, expr_operands, width, bits);
      break;
    }
    case ExprGraph::Op::kMaskedAssign: {
      const SignalWidth assigned_width = graph.slice_width(id);
      const auto base = builder.decode_affine_index(id);
      if (base.terms.empty()) {
        assert(blasted_ids[expr_operands[0]].size() >= width);
        assert(blasted_ids[expr_operands[1]].size() >= assigned_width);
        for (SignalWidth bit = 0; bit < width; ++bit) {
          assert(bit <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
          const BitIndex next_bit = static_cast<BitIndex>(bit) - base.offset;
          if (next_bit >= 0 && static_cast<SignalWidth>(next_bit) < assigned_width) {
            bits.push_back(blasted_ids[expr_operands[1]][next_bit]);
          } else {
            bits.push_back(blasted_ids[expr_operands[0]][bit]);
          }
        }
        break;
      }
      blast_word_output(id, expr_operands, width, bits);
      break;
    }
    case ExprGraph::Op::kGather: {
      assert(shape);
      const SignalWidth element_width = width / graph.outer_dim(id);
      for (ExprId operand : expr_operands) {
        bits.insert(bits.end(), blasted_ids[operand].begin(), blasted_ids[operand].end());
      }
      assert(bits.size() == expr_operands.size() * element_width);
      break;
    }
    case ExprGraph::Op::kUnpackedConcat:
      for (ExprId operand : expr_operands) {
        bits.insert(bits.end(), blasted_ids[operand].begin(), blasted_ids[operand].end());
      }
      break;
    case ExprGraph::Op::kUnpackedFlatten:
    case ExprGraph::Op::kUnpackedFold:
      assert(expr_operands.size() == 1);
      bits = blasted_ids[expr_operands.front()];
      break;
    case ExprGraph::Op::kUnpackedAssign: {
      assert(shape);
      propagate_unpacked_assign[id] = true;
      assert(expr_operands.size() == 2);
      const ExprId next = expr_operands[0];
      const ExprId index = expr_operands[1];
      const SignalWidth extent = shape->unpacked_dims.front();
      assert(extent > 0);
      assert(width % extent == 0);
      const SignalWidth element_width = width / extent;
      assert(next != kInvalidExprId);
      assert(blasted_ids[next].size() == element_width);
      const auto static_index = builder.try_evaluate(index);
      if (!static_index) {
        blast_word_output(id, expr_operands, width, bits);
        break;
      }
      for (SignalWidth element = 0; element < extent; ++element) {
        const bool selected =
            *static_index >= 0 && static_cast<SignalWidth>(*static_index) == element;
        for (SignalWidth bit = 0; bit < element_width; ++bit) {
          const ExprId next_bit = blasted_ids[next][bit];
          bits.push_back(selected ? next_bit : kInvalidExprId);
        }
      }
      break;
    }
    case ExprGraph::Op::kUnpackedRangeAssign: {
      assert(shape);
      propagate_unpacked_assign[id] = true;
      const ExprId next = expr_operands[0];
      const SignalWidth slice_width = graph.slice_width(id);
      const SignalWidth extent = shape->unpacked_dims.front();
      assert(extent > 0);
      assert(width % extent == 0);
      const SignalWidth element_width = width / extent;
      assert(blasted_ids[next].size() == slice_width * element_width);
      auto static_base = graph.unpacked_range_assign_base(id);
      if (!static_base) {
        static_base = builder.try_evaluate(expr_operands[1]);
      }
      if (!static_base) {
        blast_word_output(id, expr_operands, width, bits);
        break;
      }
      for (SignalWidth destination = 0; destination < extent; ++destination) {
        for (SignalWidth bit = 0; bit < element_width; ++bit) {
          ExprId selected = kInvalidExprId;
          for (SignalWidth source = 0; source < slice_width; ++source) {
            const ExprId next_bit = blasted_ids[next][source * element_width + bit];
            if (next_bit == kInvalidExprId) {
              continue;
            }
            const BitIndex candidate_base =
                static_cast<BitIndex>(destination) - static_cast<BitIndex>(source);
            if (*static_base == candidate_base) {
              selected = next_bit;
            }
          }
          bits.push_back(selected);
        }
      }
      break;
    }
    case ExprGraph::Op::kSequence: {
      assert(!expr_operands.empty());
      blast_word_output(id, expr_operands, width, bits);
      if (expr_operands.front() == kInvalidExprId) {
        propagate_unpacked_assign[id] = std::ranges::any_of(
            expr_operands.begin() + 1, expr_operands.end(), [&](ExprId operand) {
              return operand != kInvalidExprId && propagate_unpacked_assign[operand];
            });
      }
      break;
    }
    case ExprGraph::Op::kUnpackedSelect: {
      assert(expr_operands.size() == 2);
      const auto index = builder.try_evaluate(expr_operands[1]);
      if (index && *index >= 0) {
        const SignalWidth offset = static_cast<SignalWidth>(*index) * width;
        const auto &data = blasted_ids[expr_operands.front()];
        assert(offset <= data.size() - width);
        bits.insert(bits.end(), data.begin() + offset, data.begin() + offset + width);
        break;
      }
      blast_word_output(id, expr_operands, width, bits);
      break;
    }
    case ExprGraph::Op::kUnpackedRange: {
      assert(shape);
      assert(expr_operands.size() == 2);
      const auto base = builder.try_evaluate(expr_operands[1]);
      if (base && *base >= 0) {
        const SignalWidth element_width = width / graph.outer_dim(id);
        const SignalWidth offset = static_cast<SignalWidth>(*base) * element_width;
        const auto &data = blasted_ids[expr_operands.front()];
        assert(offset <= data.size() - width);
        bits.insert(bits.end(), data.begin() + offset, data.begin() + offset + width);
        break;
      }
      blast_word_output(id, expr_operands, width, bits);
      break;
    }
    default: {
      blast_word_output(id, expr_operands, width, bits);
      break;
    }
    }
    assert(bits.size() == width);
  }
  for (ExprId &root : module.node_output_expr_ids(node_id)) {
    if (root == kInvalidExprId) {
      continue;
    }
    assert(root < expr_count);
    assert(blasted_ids[root].size() == 1);
    root = blasted_ids[root].front();
  }
}

} // namespace abys::ir
