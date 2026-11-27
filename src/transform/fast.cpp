#include "abys/ir/tig_transformer.h"

#include <cassert>

#include "boop/apps/fast.h"
#include "boop/network/and_network.h"

namespace abys::ir {

void TigTransformer::fast() {
  for (auto &module : design_.modules) {
    for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
      if (module.node_kind(node_id) != Tig::Module::NodeKind::kAndNetwork) {
        continue;
      }
      assert(module.node_and_network(node_id));
      boop::RunFast(module.node_and_network(node_id).get());
    }
  }
}

} // namespace abys::ir
