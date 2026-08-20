#include "abys/ir/tig_transformer.h"

#include <cassert>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "abys/ir/expr_builder.h"

namespace abys::ir {

void TigTransformer::clean_op_node(Tig::Module::Node &node) {
  assert(node.kind == Tig::Module::NodeKind::kOp);
  auto &source = node.expr_graph;
  ExprGraph compact;
  std::vector<ExprId> remap(source.nodes.size(), kInvalidExprId);
  remap[ExprGraph::constant_zero] = ExprGraph::constant_zero;
  remap[ExprGraph::constant_one] = ExprGraph::constant_one;

  const auto clone = [&](const auto &self, ExprId old_id) -> ExprId {
    if (old_id == kInvalidExprId) {
      return kInvalidExprId;
    }
    if (remap[old_id] != kInvalidExprId) {
      return remap[old_id];
    }
    auto expression = source.nodes[old_id];
    for (ExprId &operand : expression.operands) {
      operand = self(self, operand);
    }
    for (const auto &properties : source.unpacked_properties) {
      if (properties.id == old_id) {
        self(self, properties.base);
        break;
      }
    }
    const ExprId new_id = static_cast<ExprId>(compact.nodes.size());
    remap[old_id] = new_id;
    compact.nodes.push_back(std::move(expression));
    return new_id;
  };

  for (ExprId &root : node.expr_roots) {
    root = clone(clone, root);
  }
  assert(node.inputs.size() == node.input_expr_ids.size());
  std::vector<Tig::Module::EdgeRef> compact_inputs;
  std::vector<ExprId> compact_input_expr_ids;
  for (size_t port = 0; port < node.inputs.size(); ++port) {
    const ExprId id = node.input_expr_ids[port];
    if (remap[id] != kInvalidExprId) {
      compact_inputs.push_back(node.inputs[port]);
      compact_input_expr_ids.push_back(remap[id]);
    }
  }
  node.inputs = std::move(compact_inputs);
  node.input_expr_ids = std::move(compact_input_expr_ids);

  for (auto &[name, id] : source.inputs) {
    if (remap[id] != kInvalidExprId) {
      compact.inputs.emplace(std::move(name), remap[id]);
    }
  }
  compact.constants.clear();
  for (auto &constant : source.constants) {
    if (remap[constant.id] != kInvalidExprId) {
      constant.id = remap[constant.id];
      compact.constants.push_back(std::move(constant));
    }
  }
  for (auto &call : source.calls) {
    if (remap[call.id] != kInvalidExprId) {
      call.id = remap[call.id];
      compact.calls.push_back(std::move(call));
    }
  }
  for (auto &properties : source.unpacked_properties) {
    if (remap[properties.id] != kInvalidExprId) {
      properties.id = remap[properties.id];
      if (properties.base != kInvalidExprId) {
        properties.base = remap[properties.base];
      }
      compact.unpacked_properties.push_back(std::move(properties));
    }
  }
  source = std::move(compact);
}

Tig::Module::EdgeRef TigTransformer::add_node_output_expr(Tig::Module &module, Tig::NodeId node_id,
                                                          ExprId expr_id) {
  auto &node = module.nodes.at(node_id);
  const auto expr = node.expr_graph.nodes.at(expr_id);
  const PortIndex port_idx = static_cast<PortIndex>(node.outputs.size());
  node.outputs.push_back({"", expr.width, expr.sign});
  node.expr_roots.push_back(expr_id);
  node.combs.push_back(true);
  return {node_id, port_idx};
}

TigTransformer::TigTransformer(Tig &design, Diagnostics &diagnostics, const NamingOptions &naming)
    : design_(design), diagnostics_(diagnostics), naming_(naming) {}

void TigTransformer::flatten_subroutine() {
  for (auto &module : design_.modules) {
    for (auto &node : module.nodes) {
      ExprGraph &expr_graph = node.expr_graph;
      if (expr_graph.calls.empty()) {
        continue;
      }
      std::vector<std::vector<SubrId>> call_ancestries(expr_graph.calls.size());
      auto replace_call_with_zero = [&](ExprId call_id, std::string detail) {
        diagnostics_.error(DiagnosticId::kLoweringUnsupportedExpressionReplacedWithZero,
                           std::move(detail));
        auto &call_node = expr_graph.nodes[call_id];
        call_node.op = ExprGraph::Op::kConvert;
        call_node.operands = {ExprGraph::constant_zero};
      };
      for (size_t i = 0; i < expr_graph.calls.size(); ++i) {
        const SubrId subr_id = expr_graph.calls[i].subr_id;
        const ExprId call_id = expr_graph.calls[i].id;
        bool recursive = false;
        for (SubrId ancestor : call_ancestries[i]) {
          if (ancestor == subr_id) {
            recursive = true;
            break;
          }
        }
        if (recursive) {
          diagnostics_.error(DiagnosticId::kTransformRecursiveSubroutineCallReplacedWithZero,
                             expr_graph.calls[i].name);
          auto &call_node = expr_graph.nodes[call_id];
          call_node.op = ExprGraph::Op::kConvert;
          call_node.operands = {ExprGraph::constant_zero};
          continue;
        }
        if (subr_id >= design_.subroutines.size() ||
            design_.subroutines[subr_id].expr_root == kInvalidExprId) {
          replace_call_with_zero(call_id, "unknown subroutine: " + expr_graph.calls[i].name);
          continue;
        }
        const Tig::Subroutine &subr = design_.subroutines[subr_id];
        if (expr_graph.nodes[call_id].operands.size() != subr.inputs.size()) {
          replace_call_with_zero(call_id, "call arity mismatch: " + expr_graph.calls[i].name);
          continue;
        }
        std::unordered_map<ExprId, ExprId> id_map;
        id_map.reserve(subr.expr_graph.nodes.size() + 1);
        id_map.emplace(kInvalidExprId, kInvalidExprId);
        bool call_valid = true;
        for (size_t j = 0; j < subr.inputs.size(); ++j) {
          const auto input_it = subr.expr_graph.inputs.find(subr.inputs[j].name);
          if (input_it == subr.expr_graph.inputs.end()) {
            replace_call_with_zero(call_id, "subroutine input not found: " + subr.inputs[j].name);
            call_valid = false;
            break;
          }
          id_map.emplace(input_it->second, expr_graph.nodes[call_id].operands[j]);
        }
        if (!call_valid) {
          continue;
        }
        for (const auto &constant : subr.expr_graph.constants) {
          if (constant.id == ExprGraph::constant_zero) {
            id_map.emplace(constant.id, ExprGraph::constant_zero);
          } else if (constant.id == ExprGraph::constant_one) {
            id_map.emplace(constant.id, ExprGraph::constant_one);
          } else {
            const auto &src = subr.expr_graph.nodes[constant.id];
            const ExprId dst_id = static_cast<ExprId>(expr_graph.nodes.size());
            expr_graph.nodes.push_back(src);
            expr_graph.constants.push_back({dst_id, constant.value});
            id_map.emplace(constant.id, dst_id);
          }
        }
        const auto return_input_it = subr.expr_graph.inputs.find(subr.name);
        if (return_input_it != subr.expr_graph.inputs.end()) {
          id_map.emplace(return_input_it->second, kInvalidExprId);
        }
        for (ExprId src_id = 0; src_id < static_cast<ExprId>(subr.expr_graph.nodes.size());
             ++src_id) {
          if (id_map.find(src_id) != id_map.end()) {
            continue;
          }
          const auto &src = subr.expr_graph.nodes[src_id];
          std::vector<ExprId> new_ops;
          new_ops.reserve(src.operands.size());
          for (ExprId sop : src.operands) {
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
          const ExprId dst_id = static_cast<ExprId>(expr_graph.nodes.size());
          expr_graph.nodes.push_back(src);
          expr_graph.nodes.back().operands = std::move(new_ops);
          id_map.emplace(src_id, dst_id);
          if (src.op == ExprGraph::Op::kCall) {
            for (const auto &src_call : subr.expr_graph.calls) {
              if (src_call.id == src_id) {
                expr_graph.calls.push_back({dst_id, src_call.subr_id, src_call.name});
                auto ancestry = call_ancestries[i];
                ancestry.push_back(subr_id);
                call_ancestries.push_back(std::move(ancestry));
                break;
              }
            }
          }
          for (const auto &src_unpacked_properties : subr.expr_graph.unpacked_properties) {
            if (src_unpacked_properties.id == src_id) {
              const ExprId base = src_unpacked_properties.base == kInvalidExprId
                                      ? kInvalidExprId
                                      : id_map.at(src_unpacked_properties.base);
              expr_graph.unpacked_properties.push_back(
                  {dst_id, base, src_unpacked_properties.unpacked_dims,
                   src_unpacked_properties.width, src_unpacked_properties.sign});
              break;
            }
          }
        }
        if (!call_valid) {
          continue;
        }
        auto rit = id_map.find(subr.expr_root);
        if (rit == id_map.end()) {
          replace_call_with_zero(call_id, "failed to map subroutine root: " + subr.name);
          continue;
        }
        const ExprId inlined_root = rit->second;
        auto &call_node = expr_graph.nodes[call_id];
        call_node.op = ExprGraph::Op::kConvert; // temporary buffer op
        call_node.operands.clear();
        call_node.operands.push_back(inlined_root);
      }
      expr_graph.calls.clear();
      clean_op_node(node);
    }
  }
  design_.subroutines.clear();
}

std::vector<Tig::Module::EdgeRef>
TigTransformer::create_memory_writes(Tig::Module &module, Tig::NodeId op_id, ExprId sequence_id) {
  auto &op = module.nodes.at(op_id);
  assert(op.kind == Tig::Module::NodeKind::kOp);
  assert(op.expr_graph.nodes.at(sequence_id).op == ExprGraph::Op::kSequence);
  ExprBuilder builder(op.expr_graph, diagnostics_);
  std::vector<SignalWidth> full_extents;
  for (const auto &properties : op.expr_graph.unpacked_properties) {
    if (properties.id == sequence_id) {
      full_extents = properties.unpacked_dims;
      break;
    }
  }
  assert(!full_extents.empty());
  struct LocalWrite {
    Tig::Module::EdgeRef enable;
    Tig::Module::EdgeRef data;
    std::vector<Tig::Module::EdgeRef> region;
  };
  std::vector<LocalWrite> local_writes;
  const auto conjunction = [&](ExprId a, ExprId b) {
    if (a == ExprGraph::constant_one) {
      return b;
    }
    if (b == ExprGraph::constant_one) {
      return a;
    }
    return builder.create_logical_and(a, b);
  };
  const auto lower_update = [&](const auto &self, ExprId id, ExprId enable) -> void {
    if (id == kInvalidExprId) {
      return;
    }
    const auto expr = op.expr_graph.nodes.at(id);
    if (expr.op == ExprGraph::Op::kSequence) {
      for (ExprId operand : expr.operands) {
        self(self, operand, enable);
      }
      return;
    }
    if (expr.op == ExprGraph::Op::kMux) {
      const ExprId condition = expr.operands.at(0);
      self(self, expr.operands.at(1), conjunction(enable, condition));
      self(self, expr.operands.at(2), conjunction(enable, builder.create_logical_not(condition)));
      return;
    }
    if (expr.op == ExprGraph::Op::kCase) {
      const ExprId selector = expr.operands.at(0);
      ExprId unmatched = ExprGraph::constant_one;
      size_t operand = 1;
      while (operand + 1 < expr.operands.size()) {
        const ExprId match = builder.create_match(selector, expr.operands[operand]);
        self(self, expr.operands[operand + 1], conjunction(enable, conjunction(unmatched, match)));
        unmatched = conjunction(unmatched, builder.create_logical_not(match));
        operand += 2;
      }
      if (operand < expr.operands.size()) {
        self(self, expr.operands[operand], conjunction(enable, unmatched));
      }
      return;
    }
    std::vector<ExprId> region;
    ExprId data = id;
    while (op.expr_graph.nodes.at(data).op == ExprGraph::Op::kUnpackedAssign) {
      const auto assign = op.expr_graph.nodes.at(data);
      region.push_back(assign.operands.at(1));
      region.push_back(assign.operands.at(2));
      data = assign.operands.at(0);
    }
    if (region.empty()) {
      for (SignalWidth extent : full_extents) {
        assert(extent <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
        region.push_back(ExprGraph::constant_zero);
        region.push_back(builder.find_or_create_const(
            static_cast<BitIndex>(extent),
            ExprBuilder::minimum_unsigned_width(static_cast<BitIndex>(extent)), false));
      }
    }
    std::vector<Tig::Module::EdgeRef> region_refs;
    for (ExprId operand : region) {
      region_refs.push_back(add_node_output_expr(module, op_id, operand));
    }
    local_writes.push_back({add_node_output_expr(module, op_id, enable),
                            add_node_output_expr(module, op_id, data), std::move(region_refs)});
  };
  lower_update(lower_update, sequence_id, ExprGraph::constant_one);

  std::vector<Tig::Module::EdgeRef> writes;
  const auto sequence_expr = op.expr_graph.nodes.at(sequence_id);
  for (auto &local : local_writes) {
    const Tig::NodeId write_id = static_cast<Tig::NodeId>(module.nodes.size());
    module.nodes.emplace_back();
    auto &write = module.nodes.back();
    write.kind = Tig::Module::NodeKind::kMemoryWrite;
    write.inputs = {local.enable, local.data};
    write.inputs.insert(write.inputs.end(), local.region.begin(), local.region.end());
    write.outputs.push_back({"", sequence_expr.width, sequence_expr.sign});
    write.expr_roots.push_back(kInvalidExprId);
    write.combs.push_back(false);
    writes.push_back({write_id, 0});
  }
  return writes;
}

bool TigTransformer::create_memory_reads(Tig::Module &module, Tig::NodeId op_id, ExprId id,
                                         ExprId read_id, std::vector<MemoryReadDimension> *region,
                                         std::vector<bool> &visited,
                                         std::vector<PendingMemoryRead> &pending_reads) {
  if (id == kInvalidExprId) {
    return false;
  }
  if (read_id == kInvalidExprId) {
    if (visited[id]) {
      return false;
    }
    visited[id] = true;
  }
  const auto expr = module.nodes[op_id].expr_graph.nodes[id];
  if (expr.op == ExprGraph::Op::kUnpackedSelect || expr.op == ExprGraph::Op::kUnpackedRange) {
    for (size_t operand = 1; operand < expr.operands.size(); ++operand) {
      create_memory_reads(module, op_id, expr.operands[operand], kInvalidExprId, nullptr, visited,
                          pending_reads);
    }
    std::vector<MemoryReadDimension> new_region;
    if (region == nullptr) {
      read_id = id;
      region = &new_region;
    }
    const SignalWidth extent = expr.op == ExprGraph::Op::kUnpackedSelect ? 1 : expr.width;
    region->push_back({expr.operands.at(1), extent, expr.op == ExprGraph::Op::kUnpackedRange});
    return create_memory_reads(module, op_id, expr.operands.at(0), read_id, region, visited,
                               pending_reads);
  }
  if (read_id != kInvalidExprId) {
    if (expr.op == ExprGraph::Op::kInput) {
      const auto &op = module.nodes[op_id];
      assert(op.inputs.size() == op.input_expr_ids.size());
      for (size_t port = 0; port < op.inputs.size(); ++port) {
        if (op.input_expr_ids[port] != id) {
          continue;
        }
        const auto input = op.inputs[port];
        const auto &source = module.nodes.at(input.node_id);
        const bool extends_read = source.kind == Tig::Module::NodeKind::kMemoryRead;
        if (!extends_read && source.kind != Tig::Module::NodeKind::kMemory &&
            (source.kind != Tig::Module::NodeKind::kJoin ||
             module.nodes.at(source.inputs.front().node_id).kind !=
                 Tig::Module::NodeKind::kMemory)) {
          continue;
        }
        std::vector<Tig::Module::EdgeRef> region_refs;
        ExprBuilder expr_builder(module.nodes[op_id].expr_graph, diagnostics_);
        for (auto dimension = region->rbegin(); dimension != region->rend(); ++dimension) {
          assert(dimension->extent <=
                 static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
          const BitIndex extent = static_cast<BitIndex>(dimension->extent);
          const ExprId extent_id =
              extent == 1 ? ExprGraph::constant_one
                          : expr_builder.find_or_create_const(
                                extent, ExprBuilder::minimum_unsigned_width(extent), false);
          region_refs.push_back(add_node_output_expr(module, op_id, dimension->index));
          region_refs.push_back(add_node_output_expr(module, op_id, extent_id));
        }
        std::vector<Tig::Module::EdgeRef> read_inputs;
        std::vector<bool> region_ranges;
        std::vector<bool> new_region_ranges;
        for (auto dimension = region->rbegin(); dimension != region->rend(); ++dimension) {
          new_region_ranges.push_back(dimension->range);
        }
        if (extends_read) {
          read_inputs = source.inputs;
          region_ranges = source.memory_region_ranges;
        } else {
          const auto ren = add_node_output_expr(module, op_id, ExprGraph::constant_one);
          read_inputs = {ren, input};
        }
        if (extends_read && region_ranges.back()) {
          const auto old_index_ref = read_inputs.at(read_inputs.size() - 2);
          assert(old_index_ref.node_id == op_id);
          const ExprId old_index = module.nodes[op_id].expr_roots.at(old_index_ref.port_idx);
          const auto new_index_ref = region_refs.at(0);
          const ExprId new_index = module.nodes[op_id].expr_roots.at(new_index_ref.port_idx);
          const ExprId combined_index = expr_builder.create_add(old_index, new_index);
          read_inputs[read_inputs.size() - 2] = add_node_output_expr(module, op_id, combined_index);
          read_inputs.back() = region_refs.at(1);
          region_ranges.back() = new_region_ranges.front();
          region_refs.erase(region_refs.begin(), region_refs.begin() + 2);
          new_region_ranges.erase(new_region_ranges.begin());
        }
        read_inputs.insert(read_inputs.end(), region_refs.begin(), region_refs.end());
        region_ranges.insert(region_ranges.end(), new_region_ranges.begin(),
                             new_region_ranges.end());
        const auto read_expr = module.nodes[op_id].expr_graph.nodes[read_id];
        const Tig::NodeId memory_read_id = static_cast<Tig::NodeId>(module.nodes.size());
        module.nodes.emplace_back();
        auto &memory_read = module.nodes.back();
        memory_read.kind = Tig::Module::NodeKind::kMemoryRead;
        memory_read.inputs = std::move(read_inputs);
        memory_read.memory_region_ranges = std::move(region_ranges);
        memory_read.outputs.push_back({"", read_expr.width, read_expr.sign});
        memory_read.expr_roots.push_back(kInvalidExprId);
        memory_read.combs.push_back(true);
        pending_reads.push_back({read_id, memory_read_id});
        return true;
      }
      return false;
    }

    for (ExprId operand : expr.operands) {
      create_memory_reads(module, op_id, operand, kInvalidExprId, nullptr, visited, pending_reads);
    }
    if (expr.op == ExprGraph::Op::kSequence) {
      for (const auto &properties : module.nodes[op_id].expr_graph.unpacked_properties) {
        if (properties.id == id) {
          const bool resolved = create_memory_reads(module, op_id, properties.base, read_id, region,
                                                    visited, pending_reads);
          if (resolved) {
            diagnostics_.error(DiagnosticId::kTransformUnsupportedMemoryRead,
                               "memory read after write ignores previous writes");
          }
          return resolved;
        }
      }
      return false;
    }
    if (expr.op == ExprGraph::Op::kCall) {
      diagnostics_.warning(DiagnosticId::kTransformUnsupportedMemoryRead,
                           "memory read through subroutine call was not extracted");
      return false;
    }
    diagnostics_.error(DiagnosticId::kTransformUnsupportedMemoryRead,
                       "memory source is not a memory input or sequence");
    return false;
  }
  for (ExprId operand : expr.operands) {
    create_memory_reads(module, op_id, operand, kInvalidExprId, nullptr, visited, pending_reads);
  }
  return false;
}

void TigTransformer::infer_memory() {
  for (auto &module : design_.modules) {
    const size_t original_node_count = module.nodes.size();
    std::vector<bool> modified_ops(original_node_count, false);

    for (Tig::NodeId op_id = 0; op_id < original_node_count; ++op_id) {
      if (module.nodes[op_id].kind != Tig::Module::NodeKind::kOp) {
        continue;
      }
      std::vector<PendingMemoryRead> pending_reads;
      std::vector<bool> visited(module.nodes[op_id].expr_graph.nodes.size(), false);
      const std::vector<ExprId> roots = module.nodes[op_id].expr_roots;
      for (ExprId root : roots) {
        create_memory_reads(module, op_id, root, kInvalidExprId, nullptr, visited, pending_reads);
      }

      for (const auto &pending : pending_reads) {
        auto &op = module.nodes[op_id];
        auto &replacement = op.expr_graph.nodes[pending.id];
        replacement.op = ExprGraph::Op::kInput;
        replacement.operands.clear();
        const std::string name =
            naming_.transformer_memory_read_prefix + std::to_string(module.transform_name_count++);
        module.nodes[pending.node_id].outputs.at(0).name = name;
        std::vector<SignalWidth> unpacked_dims;
        SignalWidth width = replacement.width;
        bool sign = replacement.sign;
        for (const auto &properties : op.expr_graph.unpacked_properties) {
          if (properties.id == pending.id) {
            unpacked_dims = properties.unpacked_dims;
            width = properties.width;
            sign = properties.sign;
            break;
          }
        }
        module.signals.push_back({name, std::move(unpacked_dims), width, sign});
        const bool inserted = op.expr_graph.inputs.emplace(name, pending.id).second;
        assert(inserted);
        op.inputs.push_back({pending.node_id, 0});
        op.input_expr_ids.push_back(pending.id);
      }
      modified_ops[op_id] = !pending_reads.empty();
    }

    for (Tig::NodeId memory_id = 0; memory_id < original_node_count; ++memory_id) {
      if (module.nodes[memory_id].kind != Tig::Module::NodeKind::kMemory) {
        continue;
      }
      const auto update = module.nodes[memory_id].inputs.at(0);
      const auto update_kind = module.nodes.at(update.node_id).kind;
      if (update_kind == Tig::Module::NodeKind::kMemoryWrite) {
        continue;
      }
      const bool has_multi_driver = update_kind == Tig::Module::NodeKind::kMultiDriver;
      if (has_multi_driver) {
        bool already_inferred = !module.nodes.at(update.node_id).inputs.empty();
        for (const auto &source : module.nodes.at(update.node_id).inputs) {
          if (module.nodes.at(source.node_id).kind != Tig::Module::NodeKind::kMemoryWrite) {
            already_inferred = false;
            break;
          }
        }
        if (already_inferred) {
          continue;
        }
      }
      const auto output = module.nodes.at(update.node_id).outputs.at(update.port_idx);
      const std::vector<Tig::Module::EdgeRef> sources =
          has_multi_driver ? module.nodes.at(update.node_id).inputs
                           : std::vector<Tig::Module::EdgeRef>{update};
      std::vector<Tig::Module::EdgeRef> writes;
      for (auto source : sources) {
        const auto &op = module.nodes.at(source.node_id);
        assert(op.kind == Tig::Module::NodeKind::kOp);
        const ExprId root = op.expr_roots.at(source.port_idx);
        const auto root_writes = create_memory_writes(module, source.node_id, root);
        writes.insert(writes.end(), root_writes.begin(), root_writes.end());
        module.nodes[source.node_id].expr_roots[source.port_idx] = kInvalidExprId;
        modified_ops[source.node_id] = true;
      }
      assert(!writes.empty());
      if (writes.size() == 1) {
        module.nodes[memory_id].inputs[0] = writes.front();
      } else if (has_multi_driver) {
        module.nodes.at(update.node_id).inputs = std::move(writes);
      } else {
        const Tig::NodeId driver_id = static_cast<Tig::NodeId>(module.nodes.size());
        module.nodes.emplace_back();
        auto &driver = module.nodes.back();
        driver.kind = Tig::Module::NodeKind::kMultiDriver;
        driver.inputs = std::move(writes);
        driver.outputs.push_back(output);
        driver.expr_roots.push_back(kInvalidExprId);
        driver.combs.push_back(false);
        module.nodes[memory_id].inputs[0] = {driver_id, 0};
      }
    }

    for (Tig::NodeId op_id = 0; op_id < original_node_count; ++op_id) {
      if (modified_ops[op_id]) {
        clean_op_node(module.nodes[op_id]);
      }
    }
  }
}

} // namespace abys::ir
