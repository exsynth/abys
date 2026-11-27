#include "abys/ir/tig_transformer.h"

#include <cassert>
#include <cstddef>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "abys/ir/expr_builder.h"

namespace abys::ir {

std::vector<EdgeRef> TigTransformer::create_memory_writes(Tig::Module &module, NodeId op_id,
                                                          ExprId sequence_id) {
  assert(module.node_kind(op_id) == Tig::Module::NodeKind::kOp);
  assert(module.node_expr_graph(op_id).op(sequence_id) == ExprGraph::Op::kSequence);
  ExprBuilder builder(module.node_expr_graph(op_id), diagnostics_);
  std::vector<SignalWidth> full_extents;
  if (const auto *shape = module.node_expr_graph(op_id).unpacked_shape(sequence_id)) {
    full_extents = shape->unpacked_dims;
  }
  assert(!full_extents.empty());
  struct LocalWrite {
    EdgeRef enable;
    EdgeRef data;
    std::vector<EdgeRef> region;
    std::vector<bool> region_ranges;
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
    const auto expr_operands = module.node_expr_graph(op_id).operands(id);
    if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kSequence) {
      assert(!expr_operands.empty());
      for (size_t operand = 1; operand < expr_operands.size(); ++operand) {
        self(self, expr_operands[operand], enable);
      }
      return;
    }
    if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kMux ||
        module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kUnpackedMux) {
      const ExprId condition = expr_operands.at(0);
      self(self, expr_operands.at(1), conjunction(enable, condition));
      self(self, expr_operands.at(2), conjunction(enable, builder.create_logical_not(condition)));
      return;
    }
    if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kCase ||
        module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kUnpackedCase) {
      const ExprId selector = expr_operands.at(0);
      ExprId unmatched = ExprGraph::constant_one;
      size_t operand = 1;
      while (operand + 1 < expr_operands.size()) {
        const ExprId match = builder.create_match(selector, expr_operands[operand]);
        self(self, expr_operands[operand + 1], conjunction(enable, conjunction(unmatched, match)));
        unmatched = conjunction(unmatched, builder.create_logical_not(match));
        operand += 2;
      }
      if (operand < expr_operands.size()) {
        self(self, expr_operands[operand], conjunction(enable, unmatched));
      }
      return;
    }
    std::vector<ExprId> region;
    std::vector<bool> region_ranges;
    ExprId data = id;
    while (module.node_expr_graph(op_id).op(data) == ExprGraph::Op::kUnpackedAssign ||
           module.node_expr_graph(op_id).op(data) == ExprGraph::Op::kUnpackedRangeAssign) {
      const auto assign_operands = module.node_expr_graph(op_id).operands(data);
      const bool range =
          module.node_expr_graph(op_id).op(data) == ExprGraph::Op::kUnpackedRangeAssign;
      region.push_back(range ? materialize_unpacked_range_assign_base(module.node_expr_graph(op_id),
                                                                      builder, data)
                             : assign_operands.at(1));
      const auto width = range ? module.node_expr_graph(op_id).slice_width(data) : 1;
      region.push_back(builder.find_or_create_const(
          static_cast<BitIndex>(width), ExprBuilder::minimum_unsigned_width(width), false));
      region_ranges.push_back(range);
      data = assign_operands.at(0);
    }
    if (region.empty()) {
      for (SignalWidth extent : full_extents) {
        assert(extent <= static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
        region.push_back(ExprGraph::constant_zero);
        region.push_back(builder.find_or_create_const(
            static_cast<BitIndex>(extent),
            ExprBuilder::minimum_unsigned_width(static_cast<BitIndex>(extent)), false));
        region_ranges.push_back(true);
      }
    }
    std::vector<EdgeRef> region_refs;
    for (size_t operand = 0; operand < region.size(); operand += 2) {
      region_refs.push_back(
          add_node_output_expr(module, op_id, region[operand], create_temporary_name(module)));
      region_refs.push_back(add_node_output_expr(module, op_id, region[operand + 1]));
    }
    local_writes.push_back(
        {add_node_output_expr(module, op_id, enable, create_temporary_name(module)),
         add_node_output_expr(module, op_id, data, create_temporary_name(module)),
         std::move(region_refs), std::move(region_ranges)});
  };
  lower_update(lower_update, sequence_id, ExprGraph::constant_one);

  const SignalWidth outer_dim = module.node_expr_graph(op_id).outer_dim(sequence_id);
  const bool sign = module.node_expr_graph(op_id).outer_sign(sequence_id);
  std::vector<EdgeRef> writes;
  for (auto &local : local_writes) {
    const NodeId write_id = module.append_node(Tig::Module::NodeKind::kMemoryWrite);
    module.node_inputs(write_id) = {local.enable, local.data};
    module.node_inputs(write_id).insert(module.node_inputs(write_id).end(), local.region.begin(),
                                        local.region.end());
    module.node_memory_range_flags(write_id) = std::move(local.region_ranges);
    module.append_node_output(write_id, "", outer_dim, sign);
    module.node_output_combs(write_id).push_back(false);
    writes.push_back({write_id, 0});
  }
  return writes;
}

bool TigTransformer::create_memory_reads(Tig::Module &module, NodeId op_id, ExprId id,
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
  auto expr_operands = module.node_expr_graph(op_id).operands(id);
  if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kUnpackedSelect ||
      module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kUnpackedRange) {
    for (size_t operand = 1; operand < expr_operands.size(); ++operand) {
      create_memory_reads(module, op_id, expr_operands[operand], kInvalidExprId, nullptr, visited,
                          pending_reads);
    }
    std::vector<MemoryReadDimension> new_region;
    if (region == nullptr) {
      read_id = id;
      region = &new_region;
    }
    const SignalWidth extent =
        module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kUnpackedSelect
            ? 1
            : module.node_expr_graph(op_id).outer_dim(id);
    region->push_back({expr_operands.at(1), extent,
                       module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kUnpackedRange});
    return create_memory_reads(module, op_id, expr_operands.at(0), read_id, region, visited,
                               pending_reads);
  }
  if (read_id != kInvalidExprId) {
    if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kInput) {
      assert(module.node_inputs(op_id).size() == module.node_input_expr_ids(op_id).size());
      for (size_t port = 0; port < module.node_inputs(op_id).size(); ++port) {
        if (module.node_input_expr_ids(op_id)[port] != id) {
          continue;
        }
        const auto input = module.node_inputs(op_id)[port];
        const auto source_id = input.node_id;
        const bool extends_read = module.node_kind(source_id) == Tig::Module::NodeKind::kMemoryRead;
        if (!extends_read && module.node_kind(source_id) != Tig::Module::NodeKind::kMemory &&
            (module.node_kind(source_id) != Tig::Module::NodeKind::kJoin ||
             module.node_kind(module.node_inputs(source_id).front().node_id) !=
                 Tig::Module::NodeKind::kMemory)) {
          continue;
        }
        std::vector<EdgeRef> region_refs;
        ExprBuilder expr_builder(module.node_expr_graph(op_id), diagnostics_);
        for (auto dimension = region->rbegin(); dimension != region->rend(); ++dimension) {
          assert(dimension->extent <=
                 static_cast<SignalWidth>(std::numeric_limits<BitIndex>::max()));
          const BitIndex extent = static_cast<BitIndex>(dimension->extent);
          const ExprId extent_id =
              extent == 1 ? ExprGraph::constant_one
                          : expr_builder.find_or_create_const(
                                extent, ExprBuilder::minimum_unsigned_width(extent), false);
          region_refs.push_back(
              add_node_output_expr(module, op_id, dimension->index, create_temporary_name(module)));
          region_refs.push_back(add_node_output_expr(module, op_id, extent_id));
        }
        std::vector<EdgeRef> read_inputs;
        std::vector<bool> region_ranges;
        std::vector<bool> new_region_ranges;
        for (auto dimension = region->rbegin(); dimension != region->rend(); ++dimension) {
          new_region_ranges.push_back(dimension->range);
        }
        if (extends_read) {
          read_inputs = module.node_inputs(source_id);
          region_ranges = module.node_memory_range_flags(source_id);
        } else {
          const auto ren = add_node_output_expr(module, op_id, ExprGraph::constant_one,
                                                create_temporary_name(module));
          read_inputs = {ren, input};
        }
        if (extends_read && region_ranges.back()) {
          const auto old_index_ref = read_inputs.at(read_inputs.size() - 2);
          assert(old_index_ref.node_id == op_id);
          const ExprId old_index = module.node_output_expr_ids(op_id).at(old_index_ref.port_idx);
          const auto new_index_ref = region_refs.at(0);
          const ExprId new_index = module.node_output_expr_ids(op_id).at(new_index_ref.port_idx);
          const ExprId combined_index = expr_builder.create_add(old_index, new_index);
          read_inputs[read_inputs.size() - 2] =
              add_node_output_expr(module, op_id, combined_index, create_temporary_name(module));
          read_inputs.back() = region_refs.at(1);
          region_ranges.back() = new_region_ranges.front();
          region_refs.erase(region_refs.begin(), region_refs.begin() + 2);
          new_region_ranges.erase(new_region_ranges.begin());
        }
        read_inputs.insert(read_inputs.end(), region_refs.begin(), region_refs.end());
        region_ranges.insert(region_ranges.end(), new_region_ranges.begin(),
                             new_region_ranges.end());
        const NodeId memory_read_id = module.append_node(Tig::Module::NodeKind::kMemoryRead);
        module.node_inputs(memory_read_id) = std::move(read_inputs);
        module.node_memory_range_flags(memory_read_id) = std::move(region_ranges);
        module.append_node_output(memory_read_id, "",
                                  module.node_expr_graph(op_id).outer_dim(read_id),
                                  module.node_expr_graph(op_id).outer_sign(read_id));
        module.node_output_combs(memory_read_id).push_back(true);
        pending_reads.push_back({read_id, memory_read_id});
        return true;
      }
      return false;
    }

    const size_t first_operand =
        module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kSequence ? 1 : 0;
    for (size_t operand = first_operand; operand < expr_operands.size(); ++operand) {
      create_memory_reads(module, op_id, expr_operands[operand], kInvalidExprId, nullptr, visited,
                          pending_reads);
    }
    if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kSequence) {
      assert(!expr_operands.empty());
      const bool resolved = create_memory_reads(module, op_id, expr_operands.front(), read_id,
                                                region, visited, pending_reads);
      if (resolved) {
        diagnostics_.error(DiagnosticId::kTransformUnsupportedMemoryRead,
                           "memory read after write ignores previous writes");
      }
      return resolved;
    }
    if (module.node_expr_graph(op_id).op(id) == ExprGraph::Op::kCall) {
      diagnostics_.warning(DiagnosticId::kTransformUnsupportedMemoryRead,
                           "memory read through subroutine call was not extracted");
      return false;
    }
    diagnostics_.error(DiagnosticId::kTransformUnsupportedMemoryRead,
                       "memory source is not a memory input or sequence");
    return false;
  }
  for (ExprId operand : expr_operands) {
    create_memory_reads(module, op_id, operand, kInvalidExprId, nullptr, visited, pending_reads);
  }
  return false;
}

void TigTransformer::infer_memory() {
  for (auto &module : design_.modules) {
    const size_t original_node_count = module.size();
    std::vector<bool> modified_ops(original_node_count, false);

    for (NodeId op_id = 0; op_id < original_node_count; ++op_id) {
      if (module.node_kind(op_id) != Tig::Module::NodeKind::kOp) {
        continue;
      }
      std::vector<PendingMemoryRead> pending_reads;
      std::vector<bool> visited(module.node_expr_graph(op_id).size(), false);
      const std::vector<ExprId> roots = module.node_output_expr_ids(op_id);
      for (ExprId root : roots) {
        create_memory_reads(module, op_id, root, kInvalidExprId, nullptr, visited, pending_reads);
      }

      for (const auto &pending : pending_reads) {
        module.node_expr_graph(op_id).set_op(pending.id, ExprGraph::Op::kInput);
        module.node_expr_graph(op_id).set_operands(pending.id, {});
        const std::string name = create_temporary_name(module);
        module.node_output_name(pending.node_id, 0) = name;
        const auto &graph = module.node_expr_graph(op_id);
        append_signal(module, name, graph.outer_dim(pending.id), graph.outer_sign(pending.id),
                      graph.unpacked_shape(pending.id));
        module.node_inputs(op_id).push_back({pending.node_id, 0});
        module.node_input_expr_ids(op_id).push_back(pending.id);
      }
      modified_ops[op_id] = !pending_reads.empty();
    }

    for (NodeId memory_id = 0; memory_id < original_node_count; ++memory_id) {
      if (module.node_kind(memory_id) != Tig::Module::NodeKind::kMemory) {
        continue;
      }
      auto update = module.node_inputs(memory_id).at(0);
      auto update_kind = module.node_kind(update.node_id);
      if (update_kind == Tig::Module::NodeKind::kMemoryWrite) {
        continue;
      }
      const bool has_multi_driver = update_kind == Tig::Module::NodeKind::kMultiDriver;
      if (has_multi_driver) {
        bool already_inferred = !module.node_inputs(update.node_id).empty();
        for (const auto &source : module.node_inputs(update.node_id)) {
          if (module.node_kind(source.node_id) != Tig::Module::NodeKind::kMemoryWrite) {
            already_inferred = false;
            break;
          }
        }
        if (already_inferred) {
          continue;
        }
      }
      const std::vector<EdgeRef> sources =
          has_multi_driver ? module.node_inputs(update.node_id) : std::vector<EdgeRef>{update};
      std::vector<EdgeRef> writes;
      for (auto source : sources) {
        const auto op_id = source.node_id;
        assert(module.node_kind(op_id) == Tig::Module::NodeKind::kOp);
        const ExprId root = module.node_output_expr_ids(op_id).at(source.port_idx);
        const auto root_writes = create_memory_writes(module, source.node_id, root);
        writes.insert(writes.end(), root_writes.begin(), root_writes.end());
        module.node_output_expr_ids(source.node_id)[source.port_idx] = kInvalidExprId;
        modified_ops[source.node_id] = true;
      }
      assert(!writes.empty());
      if (writes.size() == 1) {
        module.node_inputs(memory_id)[0] = writes.front();
      } else if (has_multi_driver) {
        module.node_inputs(update.node_id) = std::move(writes);
      } else {
        const NodeId driver_id = module.append_node(Tig::Module::NodeKind::kMultiDriver);
        module.node_inputs(driver_id) = std::move(writes);
        module.append_node_output(driver_id,
                                  module.node_output_name(update.node_id, update.port_idx),
                                  module.node_output_outer_dim(update.node_id, update.port_idx),
                                  module.node_output_outer_sign(update.node_id, update.port_idx));
        module.node_output_combs(driver_id).push_back(false);
        module.node_inputs(memory_id)[0] = {driver_id, 0};
      }
    }

    for (NodeId op_id = 0; op_id < original_node_count; ++op_id) {
      if (modified_ops[op_id]) {
        clean_op_node(module, op_id);
      }
    }
  }
}

} // namespace abys::ir
