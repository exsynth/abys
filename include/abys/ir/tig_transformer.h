#pragma once

#include <optional>
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

private:
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
