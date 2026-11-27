#include "abys/ir/tig_transformer.h"

#include <cassert>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace abys::ir {

void TigTransformer::flatten_subroutine() {
  for (auto &module : design_.modules) {
    for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
      if (module.node_kind(node_id) != Tig::Module::NodeKind::kOp) {
        continue;
      }
      ExprGraph &expr_graph = module.node_expr_graph(node_id);
      bool rewrote_call = false;
      std::unordered_map<ExprId, std::vector<SubrId>> call_ancestries;
      auto replace_call_with_zero = [&](ExprId call_id, std::string detail) {
        diagnostics_.error(DiagnosticId::kLoweringUnsupportedExpressionReplacedWithZero,
                           std::move(detail));
        expr_graph.set_op(call_id, ExprGraph::Op::kConvert);
        expr_graph.set_operands(call_id, {ExprGraph::constant_zero});
      };
      for (ExprId call_id = 0; call_id < expr_graph.size(); ++call_id) {
        if (expr_graph.op(call_id) != ExprGraph::Op::kCall) {
          continue;
        }
        rewrote_call = true;
        const SubrId subr_id = expr_graph.call_target(call_id);
        bool recursive = false;
        for (SubrId ancestor : call_ancestries[call_id]) {
          if (ancestor == subr_id) {
            recursive = true;
            break;
          }
        }
        if (recursive) {
          diagnostics_.error(DiagnosticId::kTransformRecursiveSubroutineCallReplacedWithZero,
                             expr_graph.call_name(call_id));
          expr_graph.set_op(call_id, ExprGraph::Op::kConvert);
          expr_graph.set_operands(call_id, {ExprGraph::constant_zero});
          continue;
        }
        if (subr_id >= design_.subroutines.size() ||
            design_.subroutines[subr_id].output_expr_id == kInvalidExprId) {
          replace_call_with_zero(call_id, "unknown subroutine: " + expr_graph.call_name(call_id));
          continue;
        }
        const Tig::Subroutine &subr = design_.subroutines[subr_id];
        if (expr_graph.operands(call_id).size() != subr.inputs.size()) {
          replace_call_with_zero(call_id, "call arity mismatch: " + expr_graph.call_name(call_id));
          continue;
        }
        std::unordered_map<ExprId, ExprId> id_map;
        id_map.reserve(subr.expr_graph.size() + 1);
        id_map.emplace(kInvalidExprId, kInvalidExprId);
        bool call_valid = true;
        assert(subr.inputs.size() == subr.input_expr_ids.size());
        for (size_t j = 0; j < subr.inputs.size(); ++j) {
          id_map.emplace(subr.input_expr_ids[j], expr_graph.operands(call_id)[j]);
        }
        assert(subr.captures.size() == subr.capture_expr_ids.size());
        for (size_t j = 0; j < subr.captures.size(); ++j) {
          const auto capture = subr.captures[j];
          assert(capture.node_id != kInvalidNodeId);
          assert(&module == &design_.modules.at(subr.module_id));
          const ExprId capture_id = add_node_input_expr(module, node_id, capture);
          id_map.emplace(subr.capture_expr_ids[j], capture_id);
        }
        if (!call_valid) {
          continue;
        }
        id_map.emplace(ExprGraph::constant_zero, ExprGraph::constant_zero);
        id_map.emplace(ExprGraph::constant_one, ExprGraph::constant_one);
        for (ExprId src_id = 2; src_id < subr.expr_graph.size(); ++src_id) {
          if (subr.expr_graph.op(src_id) == ExprGraph::Op::kConst) {
            id_map.emplace(src_id, expr_graph.append_copy(subr.expr_graph, src_id, {}));
          }
        }
        for (ExprId src_id = 0; src_id < static_cast<ExprId>(subr.expr_graph.size()); ++src_id) {
          if (id_map.find(src_id) != id_map.end()) {
            continue;
          }
          std::vector<ExprId> new_ops;
          new_ops.reserve(subr.expr_graph.operands(src_id).size());
          for (ExprId sop : subr.expr_graph.operands(src_id)) {
            auto mit = id_map.find(sop);
            if (mit == id_map.end()) {
              replace_call_with_zero(call_id,
                                     "subroutine graph is not topologically ordered: " + subr.name);
              call_valid = false;
              break;
            }
            new_ops.push_back(mit->second);
          }
          if (!call_valid) {
            break;
          }
          const ExprId dst_id = expr_graph.append_copy(subr.expr_graph, src_id, std::move(new_ops));
          id_map.emplace(src_id, dst_id);
          if (subr.expr_graph.op(src_id) == ExprGraph::Op::kCall) {
            auto ancestry = call_ancestries[call_id];
            ancestry.push_back(subr_id);
            call_ancestries.emplace(dst_id, std::move(ancestry));
          }
        }
        if (!call_valid) {
          continue;
        }
        auto rit = id_map.find(subr.output_expr_id);
        if (rit == id_map.end()) {
          replace_call_with_zero(call_id, "failed to map subroutine root: " + subr.name);
          continue;
        }
        const ExprId inlined_root = rit->second;
        expr_graph.set_op(call_id, ExprGraph::Op::kConvert); // temporary buffer op
        expr_graph.set_operands(call_id, {inlined_root});
      }
      if (rewrote_call) {
        clean_op_node(module, node_id);
      }
    }
  }
  design_.subroutines.clear();
}

} // namespace abys::ir
