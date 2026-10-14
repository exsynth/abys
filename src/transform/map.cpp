#include "abys/ir/tig_transformer.h"

#include <cassert>
#include <memory>
#include <string>
#include <utility>

#include "boop/interface/mockturtle_interface.h"

namespace abys::ir {

bool TigTransformer::map(std::shared_ptr<const boop::CellLibrary> library) {
  assert(library);
  bool success = true;
  for (auto &module : design_.modules) {
    for (Tig::NodeId node_id = 0; node_id < module.nodes.size(); ++node_id) {
      auto &node = module.nodes[node_id];
      if (node.kind != Tig::Module::NodeKind::kAndNetwork) {
        continue;
      }
      assert(node.and_network);
      auto mapped = std::make_shared<boop::BoundNetwork>(library.get());
      if (!boop::MockturtleMap(node.and_network.get(), mapped.get())) {
        std::string detail = "module " + module.name + ", node " + std::to_string(node_id);
        bool first_output = true;
        for (const auto &output : node.outputs) {
          if (!output.name.empty()) {
            detail += first_output ? ", outputs {" : ", ";
            detail += output.name;
            first_output = false;
          }
        }
        if (!first_output) {
          detail += "}";
        }
        diagnostics_.error(DiagnosticId::kMappingFailed, std::move(detail));
        success = false;
        continue;
      }
      node.kind = Tig::Module::NodeKind::kBoundNetwork;
      node.name = create_temporary_name(module);
      node.bound_network = std::move(mapped);
      node.cell_library = library;
      node.and_network.reset();
    }
  }
  return success;
}

} // namespace abys::ir
