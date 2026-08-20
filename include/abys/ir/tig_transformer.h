#pragma once

#include "abys/infra/diagnostics.h"
#include "abys/infra/naming.h"
#include "abys/ir/tig.h"

namespace abys::ir {

class TigTransformer {
public:
  TigTransformer(Tig &design, Diagnostics &diagnostics, const NamingOptions &naming);

  void flatten_subroutine();
  void infer_memory();

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

  static Tig::Module::EdgeRef add_node_output_expr(Tig::Module &module, Tig::NodeId node_id,
                                                   ExprId expr_id);
  static void clean_op_node(Tig::Module::Node &node);
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
