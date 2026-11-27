#pragma once

#include <map>
#include <memory>
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
  void aig(bool bus = false);
  void fast();
  bool map(std::shared_ptr<const boop::CellLibrary> library);
  bool arith(std::shared_ptr<const boop::CellLibrary> library);

private:
  using PortMaps = std::vector<std::vector<std::vector<PortIndex>>>;
  using Replacements = std::map<std::pair<NodeId, PortIndex>, EdgeRef>;

  struct PendingMemoryRead {
    ExprId id;
    NodeId node_id;
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

  static ExprId add_node_input_expr(Tig::Module &module, NodeId node_id, EdgeRef input);
  EdgeRef add_node_output_expr(Tig::Module &module, NodeId node_id, ExprId expr_id,
                               std::string name = {});
  std::string create_temporary_name(Tig::Module &module) const;
  void append_signal(Tig::Module &module, const std::string &name, SignalWidth outer_dim,
                     bool outer_sign, const UnpackedShape *shape = nullptr);
  static bool is_expr_graph_topological(const Tig::Module &module, NodeId node_id);
  static void clean_op_node(Tig::Module &module, NodeId node_id);
  static bool has_fixed_inputs(Tig::Module::NodeKind kind);
  static bool has_fixed_outputs(Tig::Module::NodeKind kind);
  std::optional<UnpackedShape> get_output_shape(const Tig::Module &module, NodeId node_id,
                                                PortIndex port_idx) const;
  EdgeRef insert_flatten(Tig::Module &module, EdgeRef input,
                         const std::optional<UnpackedShape> &shape);
  EdgeRef insert_fold(Tig::Module &module, EdgeRef input,
                      const std::optional<UnpackedShape> &shape);
  void insert_fixed_interfaces(Tig::Module &module);
  void split_outputs(Tig::Module &module, PortMaps &port_maps);
  void blast_op_nodes(Tig::Module &module, const PortMaps &port_maps);
  void create_and_networks(Tig::Module &module, bool bus);
  std::vector<NodeId> extract_arithmetic(Tig::Module &module);
  bool map_arithmetic(Tig::Module &module, NodeId node_id,
                      const std::shared_ptr<const boop::CellLibrary> &library);
  void blast_non_op_nodes(Tig::Module &module, const PortMaps &port_maps);
  void remove_buffers(Tig::Module &module);
  static void apply_replacements(Tig::Module &module, const Replacements &replacements);
  static bool collect_loop_terminals(Tig::Module &module, std::span<EdgeRef> input,
                                     std::vector<std::span<EdgeRef>> &terminals);
  static void remove_feedback(Tig::Module &module, NodeId node_id, size_t driver_count);
  std::vector<EdgeRef> select_driver_inputs(Tig::Module &module, NodeId node_id,
                                            size_t driver_count);
  void resolve_multiple_drivers(Tig::Module &module);
  void blast_expr_graph(Tig::Module &module, NodeId node_id,
                        std::vector<std::vector<ExprId>> blasted_ids);
  static std::optional<UnpackedShape> get_unpacked_shape(const Tig::Module &module, NodeId node_id,
                                                         ExprId id);
  static std::vector<ExprId> create_unpacked_element_selects(ExprBuilder &builder, ExprId data,
                                                             const UnpackedShape &shape);
  static ExprId materialize_affine_index(ExprBuilder &builder, const AffineIndex &index);
  static bool can_decompose_affine_index(ExprBuilder &builder, const AffineIndex &index);
  static std::optional<StridedIndex> extract_strided_index(ExprBuilder &builder, ExprId base);
  static std::vector<ExprId> create_barrel_shift(ExprBuilder &builder, std::vector<ExprId> lanes,
                                                 ExprId amount, ExprId fill);
  static std::vector<ExprId> create_strided_barrel_shift(ExprBuilder &builder,
                                                         std::vector<ExprId> lanes, ExprId amount,
                                                         BitIndex stride, ExprId fill);
  static void decompose_unpacked_range(Tig::Module &module, NodeId node_id, ExprBuilder &builder,
                                       ExprId range_id);
  static void decompose_unpacked_sequence(Tig::Module &module, NodeId node_id, ExprBuilder &builder,
                                          ExprId sequence_id);
  std::vector<EdgeRef> create_memory_writes(Tig::Module &module, NodeId op_id, ExprId sequence_id);
  bool create_memory_reads(Tig::Module &module, NodeId op_id, ExprId id, ExprId read_id,
                           std::vector<MemoryReadDimension> *region, std::vector<bool> &visited,
                           std::vector<PendingMemoryRead> &pending_reads);

  static ExprId materialize_unpacked_range_assign_base(const ExprGraph &graph, ExprBuilder &builder,
                                                       ExprId id);

  Tig &design_;
  Diagnostics &diagnostics_;
  const NamingOptions &naming_;
};

} // namespace abys::ir
