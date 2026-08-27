#pragma once

#include <map>
#include <optional>
#include <span>
#include <vector>

#include "abys/infra/diagnostics.h"
#include "abys/infra/naming.h"
#include "abys/ir/expr_builder.h"
#include "abys/ir/tig.h"

namespace abys::ir {

class TigTransformer {
public:
  TigTransformer(Tig &design, Diagnostics &diagnostics, const NamingOptions &naming);

  void flatten_subroutine();
  void infer_memory();
  void decompose_dynamic_access();
  void blast();

private:
  using PortMaps = std::vector<std::vector<std::vector<PortIndex>>>;
  using Replacements = std::map<std::pair<Tig::NodeId, PortIndex>, Tig::Module::EdgeRef>;

  struct PendingMemoryRead {
    ExprId id;
    Tig::NodeId node_id;
  };
  struct MemoryReadDimension {
    ExprId index;
    SignalWidth extent;
    bool range = false;
  };
  struct StridedIndex {
    ExprId index;
    SignalWidth stride;
    SignalWidth offset;
  };

  static ExprId add_node_input_expr(Tig::Module &module, Tig::Module::Node &node,
                                    Tig::Module::EdgeRef input);
  Tig::Module::EdgeRef add_node_output_expr(Tig::Module &module, Tig::NodeId node_id,
                                            ExprId expr_id, std::string name = {});
  std::string create_temporary_name(Tig::Module &module) const;
  static void clean_op_node(Tig::Module::Node &node);
  static bool has_fixed_inputs(Tig::Module::NodeKind kind);
  static bool has_fixed_outputs(Tig::Module::NodeKind kind);
  std::optional<ExprGraph::UnpackedProperties>
  get_output_properties(const Tig::Module &module, Tig::NodeId node_id, PortIndex port_idx) const;
  Tig::Module::EdgeRef
  insert_flatten(Tig::Module &module, Tig::Module::EdgeRef input,
                 const std::optional<ExprGraph::UnpackedProperties> &properties);
  Tig::Module::EdgeRef insert_fold(Tig::Module &module, Tig::Module::EdgeRef input,
                                   const std::optional<ExprGraph::UnpackedProperties> &properties);
  void insert_fixed_interfaces(Tig::Module &module);
  void split_outputs(Tig::Module &module, PortMaps &port_maps);
  void blast_op_nodes(Tig::Module &module, const PortMaps &port_maps);
  void blast_non_op_nodes(Tig::Module &module, const PortMaps &port_maps);
  void remove_buffers(Tig::Module &module);
  static void apply_replacements(Tig::Module &module, const Replacements &replacements);
  static bool collect_loop_terminals(Tig::Module &module, std::span<Tig::Module::EdgeRef> input,
                                     std::vector<std::span<Tig::Module::EdgeRef>> &terminals);
  static void remove_feedback(Tig::Module &module, Tig::NodeId node_id, size_t driver_count);
  std::vector<Tig::Module::EdgeRef>
  select_driver_inputs(Tig::Module &module, Tig::Module::Node &node, size_t driver_count);
  void resolve_multiple_drivers(Tig::Module &module);
  void blast_expr_graph(Tig::Module::Node &tig_node, std::vector<std::vector<ExprId>> blasted_ids);
  static std::optional<ExprGraph::UnpackedProperties>
  get_unpacked_properties(const Tig::Module::Node &node, ExprId id);
  static std::vector<ExprId>
  create_unpacked_element_selects(ExprBuilder &builder, ExprId data,
                                  const ExprGraph::UnpackedProperties &properties);
  static ExprId materialize_affine_index(ExprBuilder &builder,
                                         const ExprBuilder::AffineIndex &index);
  static std::optional<StridedIndex> extract_strided_index(ExprBuilder &builder, ExprId base);
  static std::vector<ExprId> create_barrel_shift(ExprBuilder &builder, std::vector<ExprId> lanes,
                                                 ExprId amount, ExprId fill);
  static void decompose_unpacked_range(Tig::Module::Node &node, ExprBuilder &builder,
                                       ExprId range_id);
  static void decompose_unpacked_sequence(Tig::Module::Node &node, ExprBuilder &builder,
                                          ExprId sequence_id);
  std::vector<Tig::Module::EdgeRef> create_memory_writes(Tig::Module &module, Tig::NodeId op_id,
                                                         ExprId sequence_id);
  bool create_memory_reads(Tig::Module &module, Tig::NodeId op_id, ExprId id, ExprId read_id,
                           std::vector<MemoryReadDimension> *region, std::vector<bool> &visited,
                           std::vector<PendingMemoryRead> &pending_reads);

  Tig &design_;
  Diagnostics &diagnostics_;
  const NamingOptions &naming_;
};

} // namespace abys::ir
