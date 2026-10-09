#include "abys/ir/tig_transformer.h"

#include <cassert>

#include "boop/apps/fast.h"
#include "boop/network/and_network.h"

namespace abys::ir {

void TigTransformer::fast() {
  for (auto &module : design_.modules) {
    for (auto &node : module.nodes) {
      if (node.kind != Tig::Module::NodeKind::kAndNetwork) {
        continue;
      }
      assert(node.and_network);
      boop::RunFast(node.and_network.get());
    }
  }
}

} // namespace abys::ir
