#include <cassert>
#include <utility>

#include "abys/ir/stmt_builder.h"

namespace abys::ir {

StmtBuilder::StmtBuilder(ExprGraph &expr_graph, Diagnostics &diagnostics)
    : expr_graph_(expr_graph), diagnostics_(diagnostics) {
  contexts_.emplace_back(Context({ExprBuilder(expr_graph, diagnostics), {}, {}, {}, {}, {}}));
}

ExprBuilder &StmtBuilder::get_expr_builder() {
  return contexts_.back().expr_builder;
}
void StmtBuilder::add_output(std::string name, bool nonblocking, ExprId id) {
  Context &context = contexts_.back();
  context.output_names.push_back(std::move(name));
  context.output_nonblocking.push_back(nonblocking);
  context.output_ids.push_back(id);
}
std::unordered_map<std::string, ExprId> &StmtBuilder::scheduled_assignments() {
  return contexts_.back().scheduled_assignments;
}
void StmtBuilder::add_local_variable(std::string name) {
  contexts_.back().local_names.insert(std::move(name));
}
size_t StmtBuilder::get_context_stack_index() const {
  return context_stack_.size();
}

const ExprBuilder &StmtBuilder::get_const_expr_builder() const {
  return contexts_.back().expr_builder;
}
const std::vector<std::string> &StmtBuilder::const_output_names() const {
  return contexts_.back().output_names;
}
const std::vector<bool> &StmtBuilder::const_output_nonblocking() const {
  return contexts_.back().output_nonblocking;
}
const std::vector<ExprId> &StmtBuilder::const_output_ids() const {
  return contexts_.back().output_ids;
}

void StmtBuilder::set_comb() {
  policy_ = Policy::Comb;
}

void StmtBuilder::set_latch() {
  policy_ = Policy::Latch;
}

void StmtBuilder::set_comb_or_latch() {
  policy_ = Policy::CombOrLatch;
}

void StmtBuilder::set_ff() {
  policy_ = Policy::Ff;
}

bool StmtBuilder::is_root_context() const {
  return contexts_.size() == 1;
}

bool StmtBuilder::is_comb() const {
  return policy_ == Policy::Comb;
}
bool StmtBuilder::is_comb_or_latch() const {
  return policy_ == Policy::CombOrLatch;
}
bool StmtBuilder::is_ff() const {
  return policy_ == Policy::Ff;
}
bool StmtBuilder::is_undecided() const {
  return policy_ == Policy::Undecided;
}

bool StmtBuilder::has_timing() const {
  return !timing_events_.empty();
}

void StmtBuilder::add_timing(ExprId expr_id, ExprId iff_id, bool posedge, bool negedge) {
  EdgeKind edge_kind = EdgeKind::kNone;
  if (posedge && negedge) {
    edge_kind = EdgeKind::kBothEdges;
  } else if (posedge) {
    edge_kind = EdgeKind::kPosedge;
  } else if (negedge) {
    edge_kind = EdgeKind::kNegedge;
  }
  if (edge_kind == EdgeKind::kNone) {
    if (!is_comb_or_latch() && !is_undecided()) {
      diagnostics_.warning(DiagnosticId::kLoweringLevelSensitiveEventIgnored);
      return;
    }
    set_comb_or_latch();
  } else {
    if (!is_undecided() && !is_ff()) {
      diagnostics_.error(DiagnosticId::kLoweringUnsupportedTimingControlIgnored,
                         "edge event in combinational or latch block");
      return;
    }
    set_ff();
  }
  timing_events_.push_back({edge_kind, expr_id, iff_id});
}

void StmtBuilder::create_context() {
  contexts_.emplace_back(
      Context({ExprBuilder(get_expr_builder()), {}, {}, {}, {}, scheduled_assignments()}));
}

void StmtBuilder::stack_context() {
  assert(contexts_.size() > 1);
  context_stack_.push_back(std::move(contexts_.back()));
  contexts_.pop_back();
}

ExprId StmtBuilder::fallback_value(const std::string &name) const {
  const auto &ctx = contexts_.back();
  auto it = ctx.scheduled_assignments.find(name);
  if (it != ctx.scheduled_assignments.end()) {
    return it->second;
  }
  return ctx.expr_builder.get_current_value(name);
}

void StmtBuilder::transfer_output(const Context &from, size_t i, ExprId expr_id) {
  if (!from.output_nonblocking[i]) {
    get_expr_builder().update_value(from.output_names[i], expr_id);
  }
  scheduled_assignments()[from.output_names[i]] = expr_id;
  add_output(from.output_names[i], from.output_nonblocking[i], expr_id);
}

std::vector<size_t> StmtBuilder::collect_last_output_indices(Context &ctx) const {
  std::unordered_map<std::string, size_t> last_index;
  for (size_t i = 0; i < ctx.output_names.size(); ++i) {
    const std::string &name = ctx.output_names[i];
    auto it = last_index.find(name);
    if (it != last_index.end()) {
      if (ctx.output_nonblocking[it->second] != ctx.output_nonblocking[i]) {
        if (!is_ff()) {
          diagnostics_.error(DiagnosticId::kLoweringMixedAssignmentTreatedAsNonblocking, name);
        } else {
          diagnostics_.warning(DiagnosticId::kLoweringMixedAssignmentTreatedAsNonblocking, name);
        }
        ctx.output_nonblocking[it->second] = true;
        ctx.output_nonblocking[i] = true;
      }
      it->second = i;
    } else {
      last_index[name] = i;
    }
  }
  std::vector<size_t> indices;
  indices.reserve(last_index.size());
  for (size_t i = 0; i < ctx.output_names.size(); ++i) {
    if (last_index[ctx.output_names[i]] == i) {
      indices.push_back(i);
    }
  }
  return indices;
}

void StmtBuilder::merge_context() {
  assert(contexts_.size() > 1);
  Context child = std::move(contexts_.back());
  contexts_.pop_back();
  for (const auto &name : child.local_names) {
    child.expr_builder.remove_input(name);
  }
  for (size_t i : collect_last_output_indices(child)) {
    if (child.local_names.contains(child.output_names[i])) {
      continue;
    }
    transfer_output(child, i, child.output_ids[i]);
  }
}

void StmtBuilder::merge_conditional(ExprId cond_id) {
  assert(context_stack_.size() > 1);
  Context else_ctx = std::move(context_stack_.back());
  context_stack_.pop_back();
  Context then_ctx = std::move(context_stack_.back());
  context_stack_.pop_back();
  const auto then_indices = collect_last_output_indices(then_ctx);
  const auto else_indices = collect_last_output_indices(else_ctx);
  // compute shared outputs
  std::unordered_set<std::string> then_outputs;
  for (size_t i : then_indices) {
    then_outputs.insert(then_ctx.output_names[i]);
  }
  std::unordered_map<std::string, size_t> shared_output_to_else_index;
  for (size_t i : else_indices) {
    if (then_outputs.contains(else_ctx.output_names[i])) {
      shared_output_to_else_index[else_ctx.output_names[i]] = i;
    }
  }
  // append then/shared outputs if not local & update their current values
  ExprBuilder &expr_builder = get_expr_builder();
  for (size_t i : then_indices) {
    assert(then_ctx.local_names.empty()); // assuming locals can only be declared in block, which
                                          // removes local on merge
    ExprId then_id = then_ctx.output_ids[i];
    const bool is_sequence = expr_builder.is_sequence(then_id);
    const std::string &name = then_ctx.output_names[i];
    auto it = shared_output_to_else_index.find(name);
    ExprId new_id = kInvalidExprId;
    if (it != shared_output_to_else_index.end()) {
      // shared
      ExprId else_id = else_ctx.output_ids[it->second];
      assert(expr_builder.is_sequence(else_id) == is_sequence);
      new_id = expr_builder.create_mux(cond_id, then_id, else_id);
      if (then_ctx.output_nonblocking[i] != else_ctx.output_nonblocking[it->second]) {
        if (!is_ff()) {
          diagnostics_.error(DiagnosticId::kLoweringMixedAssignmentTreatedAsNonblocking, name);
        } else {
          diagnostics_.warning(DiagnosticId::kLoweringMixedAssignmentTreatedAsNonblocking, name);
        }
        then_ctx.output_nonblocking[i] = true;
        else_ctx.output_nonblocking[it->second] = true;
      }
    } else if (!is_sequence) {
      // not shared
      ExprId else_id = fallback_value(name);
      if (else_id == kInvalidExprId) {
        else_id = expr_builder.find_or_create_input(name, expr_builder.get_width(then_id),
                                                    expr_builder.get_sign(then_id));
      }
      new_id = expr_builder.create_mux(cond_id, then_id, else_id);
    } else {
      new_id = expr_builder.create_mux(cond_id, then_id, kInvalidExprId);
    }
    assert(new_id != kInvalidExprId);
    if (is_sequence) {
      ExprId current_id = fallback_value(name);
      new_id = expr_builder.create_sequence(current_id, new_id, then_id);
    }
    transfer_output(then_ctx, i, new_id);
  }
  // append else/non-shared outputs if not local & update their current values
  for (size_t i : else_indices) {
    assert(else_ctx.local_names.empty());
    const std::string &name = else_ctx.output_names[i];
    if (shared_output_to_else_index.contains(name)) {
      continue;
    }
    // not shared
    ExprId then_id = fallback_value(name);
    ExprId else_id = else_ctx.output_ids[i];
    const bool is_sequence = expr_builder.is_sequence(else_id);
    ExprId new_id = kInvalidExprId;
    if (is_sequence) {
      new_id = expr_builder.create_mux(cond_id, kInvalidExprId, else_id);
      new_id = expr_builder.create_sequence(then_id, new_id, else_id);
    } else {
      if (then_id == kInvalidExprId) {
        then_id = expr_builder.find_or_create_input(name, expr_builder.get_width(else_id),
                                                    expr_builder.get_sign(else_id));
      }
      new_id = expr_builder.create_mux(cond_id, then_id, else_id);
    }
    transfer_output(else_ctx, i, new_id);
  }
}

void StmtBuilder::merge_case(ExprId selector_id, const std::vector<ExprId> &case_values,
                             size_t stack_index, bool full_case) {
  size_t output_count = 0;
  std::unordered_map<std::string, size_t> output_map;
  std::vector<std::vector<ExprId>> case_output_ids;
  std::vector<std::vector<bool>> case_output_nonblocking;
  std::vector<std::vector<bool>> case_output_sequence;
  ExprBuilder &expr_builder = get_expr_builder();
  for (size_t j = 0; j < context_stack_.size() - stack_index; ++j) {
    const Context &ctx = context_stack_[j + stack_index];
    for (size_t i = 0; i < ctx.output_names.size(); ++i) {
      const std::string &name = ctx.output_names[i];
      auto it = output_map.find(name);
      size_t output_index;
      if (it == output_map.end()) {
        output_index = output_count;
        output_map[name] = output_count;
        ++output_count;
        case_output_ids.resize(output_count);
        case_output_nonblocking.resize(output_count);
        case_output_sequence.resize(output_count);
      } else {
        output_index = it->second;
      }
      case_output_ids[output_index].resize(j + 1, kInvalidExprId);
      case_output_ids[output_index][j] = ctx.output_ids[i];
      case_output_nonblocking[output_index].resize(j + 1, false);
      case_output_nonblocking[output_index][j] = ctx.output_nonblocking[i];
      case_output_sequence[output_index].resize(j + 1, false);
      case_output_sequence[output_index][j] = expr_builder.is_sequence(ctx.output_ids[i]);
    }
  }
  for (const auto &entry : output_map) {
    ExprId current_id = fallback_value(entry.first);
    case_output_ids[entry.second].resize(context_stack_.size() - stack_index, kInvalidExprId);
    case_output_nonblocking[entry.second].resize(context_stack_.size() - stack_index, false);
    const size_t branch_count = case_output_ids[entry.second].size();
    bool is_first = true;
    bool is_nonblocking = false;
    bool is_sequence = false;
    ExprId sequence_source = kInvalidExprId;
    for (size_t j = 0; j < branch_count; ++j) {
      if (case_output_ids[entry.second][j] != kInvalidExprId) {
        if (is_first) {
          is_nonblocking = case_output_nonblocking[entry.second][j];
          is_sequence = case_output_sequence[entry.second][j];
          if (is_sequence) {
            sequence_source = case_output_ids[entry.second][j];
          }
          is_first = false;
        } else {
          assert(is_nonblocking == case_output_nonblocking[entry.second][j]);
          assert(is_sequence == case_output_sequence[entry.second][j]);
        }
      }
    }
    assert(!is_first);
    if (!is_sequence) {
      if (current_id == kInvalidExprId) {
        for (ExprId case_output_id : case_output_ids[entry.second]) {
          if (case_output_id != kInvalidExprId) {
            current_id = expr_builder.find_or_create_input(entry.first,
                                                           expr_builder.get_width(case_output_id),
                                                           expr_builder.get_sign(case_output_id));
            break;
          }
        }
      }
      for (size_t j = 0; j < branch_count; ++j) {
        if (case_output_ids[entry.second][j] == kInvalidExprId) {
          if (!full_case || j + 1 != branch_count) {
            case_output_ids[entry.second][j] = current_id;
          }
        }
      }
    }
    if (full_case) {
      assert(case_output_ids[entry.second].size() == case_values.size() + 1);
      case_output_ids[entry.second].pop_back();
      case_output_nonblocking[entry.second].pop_back();
    }
    ExprId new_id = expr_builder.create_case(selector_id, case_values,
                                             std::move(case_output_ids[entry.second]));
    if (is_sequence) {
      new_id = expr_builder.create_sequence(current_id, new_id, sequence_source);
    }
    if (!is_nonblocking) {
      expr_builder.update_value(entry.first, new_id);
    }
    scheduled_assignments()[entry.first] = new_id;
    add_output(entry.first, is_nonblocking, new_id);
  }
  while (context_stack_.size() > stack_index) {
    context_stack_.pop_back();
  }
}

bool StmtBuilder::get_timing_spec(const std::vector<std::pair<std::string, ExprId>> &outputs,
                                  std::string &clk_name, SignalWidth &clk_width, bool &clk_sign,
                                  EdgeKind &clk_edge, std::string &rst_name, SignalWidth &rst_width,
                                  bool &rst_sign, EdgeKind &rst_edge) const {
  if (timing_events_.size() != 1 && timing_events_.size() != 2) {
    diagnostics_.error(DiagnosticId::kLoweringInvalidFfTreatedAsCombinational,
                       "expected one or two timing events");
    return false;
  }
  const ExprBuilder &expr_builder = contexts_.back().expr_builder;
  auto get_input_spec = [&](int index, ExprId &input_id, std::string &name, SignalWidth &width,
                            bool &sign, EdgeKind &edge) -> bool {
    const auto &ev = timing_events_[index];
    if (ev.edge == EdgeKind::kNone) {
      diagnostics_.error(DiagnosticId::kLoweringInvalidFfTreatedAsCombinational,
                         "timing event is not edge-triggered");
      return false;
    }
    edge = ev.edge;
    if (!expr_builder.get_input_spec(ev.expr_id, input_id, name, width, sign)) {
      diagnostics_.error(DiagnosticId::kLoweringInvalidFfTreatedAsCombinational,
                         "timing event is not a named input");
      return false;
    }
    return true;
  };
  if (timing_events_.size() == 1) {
    ExprId input_id;
    if (!get_input_spec(0, input_id, clk_name, clk_width, clk_sign, clk_edge)) {
      return false;
    }
    return true;
  }
  for (int i = 0; i < 2; ++i) {
    // TODO: require the inferred reset signal to be used only as the reset condition.
    ExprId input_id;
    std::string name;
    SignalWidth width;
    bool sign;
    EdgeKind edge;
    if (!get_input_spec(i, input_id, name, width, sign, edge)) {
      return false;
    }
    bool is_used = false;
    for (const auto &kv : outputs) {
      if (expr_builder.check_dependency(kv.second, input_id)) {
        is_used = true;
        break;
      }
    }
    if (is_used) {
      if (!rst_name.empty()) {
        diagnostics_.error(DiagnosticId::kLoweringInvalidFfTreatedAsCombinational,
                           "ambiguous reset inference");
        return false;
      }
      rst_name = name;
      rst_width = width;
      rst_sign = sign;
      rst_edge = edge;
    } else {
      if (!clk_name.empty()) {
        diagnostics_.error(DiagnosticId::kLoweringInvalidFfTreatedAsCombinational,
                           "ambiguous clock inference");
        return false;
      }
      clk_name = name;
      clk_width = width;
      clk_sign = sign;
      clk_edge = edge;
    }
  }
  // TODO: handle iff (enable) too
  return true;
}

} // namespace abys::ir
