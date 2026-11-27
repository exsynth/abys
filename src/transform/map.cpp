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
    for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
      if (module.node_kind(node_id) != Tig::Module::NodeKind::kAndNetwork) {
        continue;
      }
      assert(module.node_and_network(node_id));
      auto mapped = std::make_shared<boop::BoundNetwork>(library.get());
      if (!boop::MockturtleMap(module.node_and_network(node_id).get(), mapped.get())) {
        std::string detail = "module " + module.name + ", node_id " + std::to_string(node_id);
        bool first_output = true;
        for (PortIndex port = 0; port < module.node_output_count(node_id); ++port) {
          const auto &name = module.node_output_name(node_id, port);
          if (!name.empty()) {
            detail += first_output ? ", outputs {" : ", ";
            detail += name;
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
      module.set_node_kind(node_id, Tig::Module::NodeKind::kBoundNetwork);
      module.node_name(node_id) = create_temporary_name(module);
      module.node_bound_network(node_id) = std::move(mapped);
      module.node_cell_library(node_id) = library;
    }
  }
  return success;
}

} // namespace abys::ir
