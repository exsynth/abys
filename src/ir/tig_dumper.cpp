#include <algorithm>
#include <cassert>
#include <map>
#include <sstream>
#include <unordered_set>
#include <vector>

#include "abys/ir/expr_builder.h"
#include "abys/ir/tig_dumper.h"
#include "boop/network/and_network.h"
#include "boop/network/bound_network.h"

namespace abys::ir {
namespace {

std::string format_and_expr(const boop::AndNetwork &network, int id,
                            const std::vector<std::string> &inputs,
                            const std::vector<SignalWidth> &input_widths, SignalWidth width) {
  if (network.IsConst0(id)) {
    return std::to_string(width) + "'b0";
  }
  if (network.IsPi(id)) {
    const int index = network.GetPiIndex(id);
    if (input_widths.at(index) == 1 && width > 1) {
      return "{" + std::to_string(width) + "{" + inputs.at(index) + "}}";
    }
    assert(input_widths.at(index) == width);
    return inputs.at(index);
  }
  if (network.GetNumFanins(id) == 0) {
    return std::to_string(width) + "'b1";
  }
  std::string result = "(";
  for (int index = 0; index < network.GetNumFanins(id); ++index) {
    if (index != 0) {
      result += " & ";
    }
    const int fanin = network.GetFanin(id, index);
    const std::string expression = format_and_expr(network, fanin, inputs, input_widths, width);
    if (network.GetCompl(id, index)) {
      result += "~(" + expression + ")";
    } else {
      result += expression;
    }
  }
  return result + ")";
}

std::string format_mapped_connection(const std::string &name, SignalWidth source_width,
                                     SignalWidth target_width) {
  assert(source_width == target_width || source_width == 1);
  if (source_width == target_width) {
    return name;
  }
  return "{" + std::to_string(target_width) + "{" + name + "}}";
}

void emit_bound_network(const Tig::Module &module, NodeId node_id, std::ostream &os) {
  assert(module.node_bound_network(node_id));
  const auto &network = *module.node_bound_network(node_id);
  assert(module.node_cell_library(node_id) &&
         module.node_cell_library(node_id).get() == network.GetLibrary());
  const auto &library = *network.GetLibrary();
  assert(network.GetNumPis() == static_cast<int>(module.node_inputs(node_id).size()));
  assert(network.GetNumPos() == static_cast<int>(module.node_output_count(node_id)));

  std::vector<std::string> names(network.GetNumNodes());
  std::vector<SignalWidth> widths(network.GetNumNodes(), 0);
  names[network.GetConst0()] = "1'b0";
  names[network.GetConst1()] = "1'b1";
  widths[network.GetConst0()] = 1;
  widths[network.GetConst1()] = 1;

  size_t input = 0;
  network.ForEachPi([&](int id) {
    const auto input_ref = module.node_inputs(node_id)[input++];
    assert(input_ref.node_id != kInvalidNodeId);
    const auto &name = module.node_output_name(input_ref.node_id, input_ref.port_idx);
    assert(!name.empty());
    names[id] = name;
    widths[id] = module.node_output_outer_dim(input_ref.node_id, input_ref.port_idx);
  });

  size_t output = 0;
  network.ForEachPo([&](int id) {
    assert(network.GetNumFanins(id) == 1);
    const int driver = network.GetFanin(id, 0);
    widths[driver] = std::max(widths[driver], module.node_output_outer_dim(node_id, output++));
  });
  network.ForEachInstance<true>([&](int id) {
    SignalWidth width = 0;
    network.ForEachOutput(id, [&](int output_id) { width = std::max(width, widths[output_id]); });
    assert(width > 0);
    network.ForEachOutput(id, [&](int output_id) { widths[output_id] = width; });
    network.ForEachFanin(id, [&](int fanin) {
      if (network.IsInt(fanin)) {
        widths[fanin] = std::max(widths[fanin], width);
      }
    });
  });

  network.ForEachInstance([&](int id) {
    const int cell = network.GetCell(id);
    const SignalWidth width = widths[id];
    const std::string instance_name = module.node_name(node_id) + "_" + std::to_string(id);
    network.ForEachOutput(id, [&](int output_id) {
      names[output_id] = instance_name + "_" + std::to_string(network.GetOutputIndex(output_id));
      os << "  logic ";
      if (width > 1) {
        os << "[" << (width - 1) << ":0] ";
      }
      os << names[output_id] << ";\n";
    });
    os << "  " << library.GetCellName(cell) << " " << instance_name;
    if (width > 1) {
      os << " [" << (width - 1) << ":0]";
    }
    os << " (\n";
    for (int pin = 0; pin < library.GetNumInputs(cell); ++pin) {
      const int fanin = network.GetFanin(id, pin);
      os << "    ." << library.GetInputName(cell, pin) << "("
         << format_mapped_connection(names[fanin], widths[fanin], width) << "),\n";
    }
    for (int pin = 0; pin < library.GetNumOutputs(cell); ++pin) {
      const int output_id = network.GetOutput(id, pin);
      os << "    ." << library.GetOutputName(cell, pin) << "(" << names[output_id] << ")"
         << (pin + 1 == library.GetNumOutputs(cell) ? "\n" : ",\n");
    }
    os << "  );\n";
  });

  output = 0;
  network.ForEachPo([&](int id) {
    const auto port = output++;
    const auto &name = module.node_output_name(node_id, port);
    if (name.empty()) {
      return;
    }
    const int driver = network.GetFanin(id, 0);
    os << "  assign " << name << " = "
       << format_mapped_connection(names[driver], widths[driver],
                                   module.node_output_outer_dim(node_id, port))
       << ";\n";
  });
}

} // namespace

TigDumper::TigDumper(const Tig &design, Diagnostics &diagnostics, const NamingOptions &naming)
    : design_(design), diagnostics_(diagnostics), naming_(naming) {}

void TigDumper::dump(std::ostream &os) const {
  bool first = true;
  for (const Subroutine &subroutine : design_.subroutines) {
    if (subroutine.module_id != kInvalidModuleId || subroutine.output_expr_id == kInvalidExprId) {
      continue;
    }
    if (!first) {
      os << "\n";
    }
    first = false;
    emit_subroutine(subroutine, os);
  }
  for (ModuleId module_id = 0; module_id < design_.modules.size(); ++module_id) {
    if (!first) {
      os << "\n";
    }
    first = false;
    emit_module(module_id, design_.modules[module_id], os);
  }
}

void TigDumper::emit_subroutine(const Subroutine &subroutine, std::ostream &os) const {
  const std::string emitted_name = subroutine.name + subroutine.variant_suffix;
  os << "function automatic ";
  if (subroutine.expr_graph.outer_sign(subroutine.output_expr_id)) {
    os << "signed ";
  }
  if (subroutine.expr_graph.outer_dim(subroutine.output_expr_id) > 1) {
    os << "[" << (subroutine.expr_graph.outer_dim(subroutine.output_expr_id) - 1) << ":0] ";
  }
  os << emitted_name << " (\n";
  for (size_t i = 0; i < subroutine.inputs.size(); ++i) {
    os << "  input ";
    const auto *shape = subroutine.inputs.shape(i);
    if (shape ? shape->inner_sign : subroutine.inputs.outer_sign(i)) {
      os << "signed ";
    }
    if (const auto width = shape ? shape->inner_dim : subroutine.inputs.outer_dim(i); width > 1) {
      os << "[" << (width - 1) << ":0] ";
    }
    os << subroutine.inputs.name(i);
    if (shape) {
      for (SignalWidth dim : shape->unpacked_dims) {
        os << " [0:" << (dim - 1) << "]";
      }
    }
    os << (i + 1 == subroutine.inputs.size() ? "\n" : ",\n");
  }
  os << ");\n";
  emit_expr(emitted_name, false, false, subroutine.expr_graph, subroutine.output_expr_id, os, "  ",
            get_subroutine_input_names(subroutine));
  os << "endfunction\n";
}

void TigDumper::emit_module(ModuleId module_id, const Module &module, std::ostream &os) const {
  emit_module_header(module, os);
  emit_signal_decls(module, os);
  for (const Subroutine &subroutine : design_.subroutines) {
    if (subroutine.module_id == module_id && subroutine.output_expr_id != kInvalidExprId) {
      emit_subroutine(subroutine, os);
      os << "\n";
    }
  }
  emit_instances(module, os);
  emit_combinational(module, os);
  emit_sequential(module, os);
  emit_module_footer(os);
}

void TigDumper::emit_module_header(const Module &module, std::ostream &os) {
  os << "module " << module.name << module.variant_suffix << " (\n";
  bool first = true;
  for (size_t port = 0; port < module.input_ports.size(); ++port) {
    if (!first) {
      os << ",\n";
    }
    first = false;
    os << "  input ";
    const auto *shape = module.input_ports.shape(port);
    if (shape ? shape->inner_sign : module.input_ports.outer_sign(port)) {
      os << "signed ";
    }
    if (const auto width = shape ? shape->inner_dim : module.input_ports.outer_dim(port); width > 1) {
      os << "[" << (width - 1) << ":0] ";
    }
    os << module.input_ports.name(port);
    if (shape) {
      for (SignalWidth dim : shape->unpacked_dims) {
        os << " [0:" << (dim - 1) << "]";
      }
    }
  }
  for (size_t port = 0; port < module.output_ports.size(); ++port) {
    if (!first) {
      os << ",\n";
    }
    first = false;
    os << "  output ";
    os << " logic ";
    const auto *shape = module.output_ports.shape(port);
    if (shape ? shape->inner_sign : module.output_ports.outer_sign(port)) {
      os << "signed ";
    }
    if (const auto width = shape ? shape->inner_dim : module.output_ports.outer_dim(port); width > 1) {
      os << "[" << (width - 1) << ":0] ";
    }
    os << module.output_ports.name(port);
    if (shape) {
      for (SignalWidth dim : shape->unpacked_dims) {
        os << " [0:" << (dim - 1) << "]";
      }
    }
  }
  os << ");\n\n";
}

void TigDumper::emit_signal_decls(const Module &module, std::ostream &os) {
  std::unordered_set<std::string> port_names;
  port_names.insert(module.input_ports.names().begin(), module.input_ports.names().end());
  port_names.insert(module.output_ports.names().begin(), module.output_ports.names().end());
  for (size_t id = 0; id < module.signals.size(); ++id) {
    const auto &name = module.signals.name(id);
    if (port_names.contains(name)) {
      continue;
    }
    os << "  ";
    os << "logic ";
    const auto *shape = module.signals.shape(id);
    if (shape ? shape->inner_sign : module.signals.outer_sign(id)) {
      os << "signed ";
    }
    if (const auto width = shape ? shape->inner_dim : module.signals.outer_dim(id); width > 1) {
      os << "[" << (width - 1) << ":0] ";
    }
    os << name;
    if (shape) {
      for (SignalWidth width : shape->unpacked_dims) {
        os << " [0:" << (width - 1) << "]";
      }
    }
    os << ";\n";
  }
  os << "\n";
}

void TigDumper::emit_instances(const Module &module, std::ostream &os) const {
  for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
    if (module.node_kind(node_id) != Module::NodeKind::kInstance) {
      continue;
    }
    const auto &child = design_.modules[module.node_module_id(node_id)];
    const std::string inst = module.node_name(node_id);
    os << "  " << child.name << child.variant_suffix << " " << inst << " (\n";
    bool first = true;
    for (size_t i = 0; i < child.input_ports.size(); ++i) {
      if (!first) {
        os << ",\n";
      }
      first = false;
      const auto &port_name = child.input_ports.name(i);
      const auto &data_ref = module.node_inputs(node_id)[i];
      os << "    ." << port_name << "(";
      if (data_ref.node_id == kInvalidNodeId) {
        os << "1'bx";
      } else {
        const auto data_node_id = data_ref.node_id;
        const std::string data_name = module.node_output_name(data_node_id, data_ref.port_idx);
        assert(!data_name.empty());
        os << data_name;
      }
      os << ")";
    }
    for (size_t i = 0; i < child.output_ports.size(); ++i) {
      if (!first) {
        os << ",\n";
      }
      first = false;
      const auto &port_name = child.output_ports.name(i);
      const std::string sig = module.node_output_name(node_id, i);
      os << "    ." << port_name << "(" << sig << ")";
    }
    os << "  );\n";
  }
  os << "\n";
}

void TigDumper::emit_combinational(const Module &module, std::ostream &os) const {
  // TODO: handle latches
  for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
    if (module.node_kind(node_id) == Module::NodeKind::kOp) {
      assert(module.node_output_count(node_id) == module.node_output_expr_ids(node_id).size());
      std::vector<std::string> lhs_names;
      std::vector<ExprId> expr_ids;
      for (size_t i = 0; i < module.node_output_count(node_id); ++i) {
        if (!module.node_output_name(node_id, i).empty()) {
          lhs_names.push_back(module.node_output_name(node_id, i));
          expr_ids.push_back(module.node_output_expr_ids(node_id)[i]);
        }
      }
      if (lhs_names.empty()) {
        continue;
      }
      os << "  always @(*) ";
      emit_exprs(lhs_names, false, false, module.node_expr_graph(node_id), expr_ids, os, "  ",
                 get_node_input_names(module, node_id));
    } else if (module.node_kind(node_id) == Module::NodeKind::kAndNetwork) {
      assert(module.node_and_network(node_id));
      const auto &network = *module.node_and_network(node_id);
      assert(network.GetNumPis() == static_cast<int>(module.node_inputs(node_id).size()));
      assert(network.GetNumPos() == static_cast<int>(module.node_output_count(node_id)));
      std::vector<std::string> inputs;
      std::vector<SignalWidth> input_widths;
      inputs.reserve(module.node_inputs(node_id).size());
      input_widths.reserve(module.node_inputs(node_id).size());
      for (const auto input_ref : module.node_inputs(node_id)) {
        assert(input_ref.node_id != kInvalidNodeId);
        inputs.push_back(module.node_output_name(input_ref.node_id, input_ref.port_idx));
        input_widths.push_back(module.node_output_outer_dim(input_ref.node_id, input_ref.port_idx));
        assert(!inputs.back().empty());
      }
      for (PortIndex port = 0; port < module.node_output_count(node_id); ++port) {
        const std::string &name = module.node_output_name(node_id, port);
        if (name.empty()) {
          continue;
        }
        const int po = network.GetPo(port);
        assert(network.GetNumFanins(po) == 1);
        const int driver = network.GetFanin(po, 0);
        std::string expression = format_and_expr(network, driver, inputs, input_widths,
                                                 module.node_output_outer_dim(node_id, port));
        if (network.GetCompl(po, 0)) {
          expression = "~(" + expression + ")";
        }
        os << "  assign " << name << " = " << expression << ";\n";
      }
    } else if (module.node_kind(node_id) == Module::NodeKind::kBoundNetwork) {
      emit_bound_network(module, node_id, os);
    } else if (module.node_kind(node_id) == Module::NodeKind::kMultiDriver) {
      assert(!(module.node_output_count(node_id) == 0));
      assert(module.node_inputs(node_id).size() % module.node_output_count(node_id) == 0);
      const size_t driver_count = module.node_inputs(node_id).size() / module.node_output_count(node_id);
      for (PortIndex port = 0; port < module.node_output_count(node_id); ++port) {
        const std::string &name = module.node_output_name(node_id, port);
        if (name.empty()) {
          continue;
        }
        bool emitted = false;
        for (size_t driver = 0; driver < driver_count; ++driver) {
          const auto input_ref =
              module.node_inputs(node_id)[driver * module.node_output_count(node_id) + port];
          if (input_ref.node_id == kInvalidNodeId) {
            continue;
          }
          if (!emitted) {
            os << "  always @(*) begin\n";
            emitted = true;
          }
          assert(input_ref.node_id < module.size());
          const auto input_node_id = input_ref.node_id;
          if (module.node_kind(input_node_id) == Module::NodeKind::kOp) {
            assert(input_ref.port_idx < module.node_output_expr_ids(input_node_id).size());
            emit_expr(name, false, false, module.node_expr_graph(input_node_id),
                      module.node_output_expr_ids(input_node_id)[input_ref.port_idx], os, "    ",
                      get_node_input_names(module, input_node_id));
          } else {
            assert(input_ref.port_idx < module.node_output_count(input_node_id));
            const std::string &input_name = module.node_output_name(input_node_id, input_ref.port_idx);
            assert(!input_name.empty());
            os << "    " << name << " = " << input_name << ";\n";
          }
        }
        if (emitted) {
          os << "  end\n";
        }
      }
    } else if (module.node_kind(node_id) == Module::NodeKind::kFlatten) {
      assert(module.node_inputs(node_id).size() == 1);
      const auto &input = module.node_inputs(node_id).front();
      assert(input.node_id != kInvalidNodeId);
      const auto input_node_id = input.node_id;
      const std::string &input_name = module.node_output_name(input_node_id, input.port_idx);
      std::vector<SignalWidth> unpacked_dims;
      SignalWidth element_width = module.node_output_outer_dim(input_node_id, input.port_idx);
      if (module.node_kind(input_node_id) == Module::NodeKind::kInstance) {
        const auto &ports = design_.modules[module.node_module_id(input_node_id)].output_ports;
        const auto *shape = ports.shape(input.port_idx);
        if (shape) {
          unpacked_dims = shape->unpacked_dims;
        }
        element_width = shape ? shape->inner_dim : ports.outer_dim(input.port_idx);
      } else if (const auto index = module.signals.find_index(input_name)) {
        const auto *shape = module.signals.shape(*index);
        if (shape) {
          unpacked_dims = shape->unpacked_dims;
        }
        element_width = shape ? shape->inner_dim : module.signals.outer_dim(*index);
      }
      SignalWidth element_count = 1;
      for (SignalWidth dimension : unpacked_dims) {
        element_count *= dimension;
      }
      assert(module.node_output_count(node_id) == element_width * element_count);
      os << "  always @(*) begin\n";
      for (SignalWidth bit = 0; bit < module.node_output_count(node_id); ++bit) {
        const SignalWidth element = bit / element_width;
        SignalWidth remaining = element;
        std::vector<SignalWidth> indices(unpacked_dims.size());
        for (size_t dimension = unpacked_dims.size(); dimension-- > 0;) {
          indices[dimension] = remaining % unpacked_dims[dimension];
          remaining /= unpacked_dims[dimension];
        }
        os << "    " << module.node_output_name(node_id, bit) << " = " << input_name;
        if (unpacked_dims.empty()) {
          os << "[" << bit << "]";
        } else {
          for (SignalWidth index : indices) {
            os << "[" << index << "]";
          }
          os << "[" << bit % element_width << "]";
        }
        os << ";\n";
      }
      os << "  end\n";
    } else if (module.node_kind(node_id) == Module::NodeKind::kConstant) {
      assert(module.node_output_count(node_id) == 1);
      assert(module.node_inputs(node_id).empty());
      os << "  assign " << module.node_output_name(node_id, 0) << " = "
         << module.node_constant_value(node_id) << ";\n";
    } else if (module.node_kind(node_id) == Module::NodeKind::kFold) {
      assert(module.node_output_count(node_id) == 1);
      const std::string &name = module.node_output_name(node_id, 0);
      assert(!name.empty());
      const auto index = module.signals.find_index(name);
      assert(index);
      const auto *shape = module.signals.shape(*index);
      const auto emit_input = [&](size_t bit) {
        const auto &input = module.node_inputs(node_id)[bit];
        assert(input.node_id != kInvalidNodeId);
        os << module.node_output_name(input.node_id, input.port_idx);
      };
      os << "  always @(*) begin\n";
      if (!shape) {
        os << "    " << name << " = {";
        for (size_t bit = module.node_inputs(node_id).size(); bit-- > 0;) {
          if (bit + 1 != module.node_inputs(node_id).size()) {
            os << ", ";
          }
          emit_input(bit);
        }
        os << "};\n";
      } else {
        SignalWidth element_count = 1;
        for (SignalWidth dimension : shape->unpacked_dims) {
          element_count *= dimension;
        }
        assert(module.node_inputs(node_id).size() == element_count * shape->inner_dim);
        for (SignalWidth element = 0; element < element_count; ++element) {
          SignalWidth remaining = element;
          std::vector<SignalWidth> indices(shape->unpacked_dims.size());
          for (size_t dimension = shape->unpacked_dims.size(); dimension-- > 0;) {
            indices[dimension] = remaining % shape->unpacked_dims[dimension];
            remaining /= shape->unpacked_dims[dimension];
          }
          os << "    " << name;
          for (SignalWidth index : indices) {
            os << "[" << index << "]";
          }
          os << " = {";
          const SignalWidth begin = element * shape->inner_dim;
          for (SignalWidth bit = shape->inner_dim; bit-- > 0;) {
            if (bit + 1 != shape->inner_dim) {
              os << ", ";
            }
            emit_input(begin + bit);
          }
          os << "};\n";
        }
      }
      os << "  end\n";
    } else if (module.node_kind(node_id) == Module::NodeKind::kMemoryRead) {
      assert(module.node_inputs(node_id).size() >= 2);
      assert((module.node_inputs(node_id).size() - 2) % 2 == 0);
      assert(module.node_memory_region_ranges(node_id).size() ==
             (module.node_inputs(node_id).size() - 2) / 2);
      assert(module.node_output_count(node_id) == 1);
      const std::string &name = module.node_output_name(node_id, 0);
      assert(!name.empty());
      const auto memory_ref = module.node_inputs(node_id).at(1);
      const std::string &memory_name = module.node_output_name(memory_ref.node_id, memory_ref.port_idx);
      assert(!memory_name.empty());
      std::string access = memory_name;
      for (size_t dimension = 0; dimension < module.node_memory_region_ranges(node_id).size();
           ++dimension) {
        const auto index_ref = module.node_inputs(node_id).at(2 + 2 * dimension);
        const auto extent_ref = module.node_inputs(node_id).at(3 + 2 * dimension);
        const std::string &index = module.node_output_name(index_ref.node_id, index_ref.port_idx);
        assert(!index.empty());
        access += "[" + index;
        if (module.node_memory_region_ranges(node_id)[dimension]) {
          const auto extent_node_id = extent_ref.node_id;
          ExprGraph extent_graph = module.node_expr_graph(extent_node_id);
          ExprBuilder extent_builder(extent_graph, diagnostics_);
          const auto extent = extent_builder.try_evaluate(
              module.node_output_expr_ids(extent_node_id).at(extent_ref.port_idx));
          assert(extent.has_value());
          access += " +: " + std::to_string(*extent);
        }
        access += "]";
      }
      os << "  always @(*) begin\n";
      os << "    " << name << " = " << access << ";\n";
      os << "  end\n";
    }
  }
}

void TigDumper::emit_sequential(const Module &module, std::ostream &os) const {
  auto edge_to_string = [&](EdgeKind edge) -> const char * {
    switch (edge) {
    case EdgeKind::kPosedge:
      return "posedge";
    case EdgeKind::kNegedge:
      return "negedge";
    case EdgeKind::kBothEdges:
      return "edge";
    case EdgeKind::kNone:
    default:
      diagnostics_.error(DiagnosticId::kEmitterInvalidEdgeTreatedAsPosedge);
      return "posedge";
    }
  };
  std::map<NodeId, std::string> joined_edge_states;
  for (NodeId node_id = 0; node_id < module.size(); ++node_id) {
    if (module.node_kind(node_id) == Module::NodeKind::kJoin) {
      for (const auto &input : module.node_inputs(node_id)) {
        assert(input.port_idx == 0);
        joined_edge_states[input.node_id] = module.node_output_name(node_id, 0);
      }
    }
  }
  const auto emit_edge_write = [&](const auto &self, std::string_view lhs,
                                   const EdgeRef &data_ref, std::string_view indent,
                                   const std::unordered_map<std::string, bool> *assumptions,
                                   bool is_nonblocking, bool is_merge) -> void {
    assert(data_ref.node_id != kInvalidNodeId);
    assert(data_ref.node_id < module.size());
    const auto data_node_id = data_ref.node_id;
    const std::string data_name = module.node_output_name(data_node_id, data_ref.port_idx);
    if (!data_name.empty()) {
      os << indent << lhs << ((is_nonblocking && !is_merge) ? " <= " : " = ") << data_name << ";\n";
      return;
    }
    if (module.node_kind(data_node_id) == Module::NodeKind::kOp) {
      assert(data_ref.port_idx < module.node_output_expr_ids(data_node_id).size());
      emit_expr(lhs, is_nonblocking, is_merge, module.node_expr_graph(data_node_id),
                module.node_output_expr_ids(data_node_id)[data_ref.port_idx], os, indent,
                get_node_input_names(module, data_node_id), assumptions);
      return;
    }
    if (module.node_kind(data_node_id) == Module::NodeKind::kMemoryWrite) {
      assert(module.node_inputs(data_node_id).size() >= 2);
      assert((module.node_inputs(data_node_id).size() - 2) % 2 == 0);
      assert(module.node_memory_region_ranges(data_node_id).size() ==
             (module.node_inputs(data_node_id).size() - 2) / 2);
      const auto &enable_ref = module.node_inputs(data_node_id).at(0);
      const std::string &enable = module.node_output_name(enable_ref.node_id, enable_ref.port_idx);
      assert(!enable.empty());
      bool enable_is_assumed = false;
      bool enable_value = false;
      if (assumptions != nullptr) {
        const auto assumption = assumptions->find(enable);
        if (assumption != assumptions->end()) {
          enable_is_assumed = true;
          enable_value = assumption->second;
        }
      }
      if (enable_is_assumed && !enable_value) {
        return;
      }
      std::string selected_lhs(lhs);
      for (size_t input = 2; input < module.node_inputs(data_node_id).size(); input += 2) {
        const auto &index_ref = module.node_inputs(data_node_id).at(input);
        const std::string &index = module.node_output_name(index_ref.node_id, index_ref.port_idx);
        assert(!index.empty());
        selected_lhs += "[" + index;
        const auto &extent_ref = module.node_inputs(data_node_id).at(input + 1);
        const auto extent_node_id = extent_ref.node_id;
        ExprGraph extent_graph = module.node_expr_graph(extent_node_id);
        ExprBuilder extent_builder(extent_graph, diagnostics_);
        const auto extent =
            extent_builder.try_evaluate(module.node_output_expr_ids(extent_node_id).at(extent_ref.port_idx));
        assert(extent.has_value());
        const size_t dimension = (input - 2) / 2;
        if (module.node_memory_region_ranges(data_node_id)[dimension]) {
          selected_lhs += " +: " + std::to_string(*extent);
        }
        selected_lhs += "]";
      }
      const auto &update_ref = module.node_inputs(data_node_id).at(1);
      const std::string &update = module.node_output_name(update_ref.node_id, update_ref.port_idx);
      assert(!update.empty());
      std::string assignment_indent(indent);
      if (!enable_is_assumed) {
        os << indent << "if (" << enable << ") begin\n";
        assignment_indent += "  ";
      }
      os << assignment_indent << selected_lhs << ((is_nonblocking && !is_merge) ? " <= " : " = ")
         << update << ";\n";
      if (!enable_is_assumed) {
        os << indent << "end\n";
      }
      return;
    }
    assert(module.node_kind(data_node_id) == Module::NodeKind::kEdgeMultiDriver);
    assert(data_ref.port_idx == 0);
    for (size_t input = 0; input < module.node_inputs(data_node_id).size(); ++input) {
      // Merge expansion needs blocking assignments to accumulate writes within this block.
      self(self, lhs, module.node_inputs(data_node_id)[input], indent, assumptions, is_nonblocking,
           true);
    }
  };
  for (NodeId state_id = 0; state_id < module.size(); ++state_id) {
    if (module.node_kind(state_id) != Module::NodeKind::kFf &&
        module.node_kind(state_id) != Module::NodeKind::kMemory) {
      continue;
    }
    assert(module.node_inputs(state_id).size() == module.node_output_count(state_id) + 1 ||
           module.node_inputs(state_id).size() == module.node_output_count(state_id) + 2);
    const size_t control = module.node_output_count(state_id);
    const auto &clk_ref = module.node_inputs(state_id)[control];
    const auto clk_node_id = clk_ref.node_id;
    const std::string clk_name = module.node_output_name(clk_node_id, clk_ref.port_idx);
    std::string rst_name;
    if (module.node_inputs(state_id).size() == control + 2) {
      const auto &rst_ref = module.node_inputs(state_id)[control + 1];
      const auto rst_node_id = rst_ref.node_id;
      rst_name = module.node_output_name(rst_node_id, rst_ref.port_idx);
    }
    for (PortIndex output = 0; output < module.node_output_count(state_id); ++output) {
      std::string lhs_name = module.node_output_name(state_id, output);
      if (lhs_name.empty()) {
        auto it = joined_edge_states.find(state_id);
        if (it != joined_edge_states.end()) {
          lhs_name = it->second;
        }
      }
      assert(!lhs_name.empty());
      const auto &data_ref = module.node_inputs(state_id)[output];
      // TODO: handle both-edge events
      os << "  always @(" << edge_to_string(module.node_clock_edge(state_id)) << " " << clk_name;
      if (!rst_name.empty()) {
        os << " or " << edge_to_string(module.node_reset_edge(state_id)) << " " << rst_name;
      }
      os << ") begin\n";
      if (!rst_name.empty()) {
        const bool reset_value = module.node_reset_edge(state_id) == EdgeKind::kPosedge;
        const std::unordered_map<std::string, bool> reset_assumptions{{rst_name, reset_value}};
        const std::unordered_map<std::string, bool> clock_assumptions{{rst_name, !reset_value}};
        os << "    if (" << ((module.node_reset_edge(state_id) == EdgeKind::kNegedge) ? "!" : "")
           << rst_name << ") begin\n";
        // Emit top-level FF writes as NBA so Yosys can prove the sequential memory/FF pattern.
        emit_edge_write(emit_edge_write, lhs_name, data_ref, "      ", &reset_assumptions, true,
                        false);
        os << "    end else begin\n";
        // Emit top-level FF writes as NBA so Yosys can prove the sequential memory/FF pattern.
        emit_edge_write(emit_edge_write, lhs_name, data_ref, "      ", &clock_assumptions, true,
                        false);
        os << "    end\n";
      } else {
        // Emit top-level FF writes as NBA so Yosys can prove the sequential memory/FF pattern.
        emit_edge_write(emit_edge_write, lhs_name, data_ref, "    ", nullptr, true, false);
      }
      os << "  end\n";
    }
  }
}

std::unordered_map<ExprId, std::string>
TigDumper::get_subroutine_input_names(const Subroutine &subroutine) const {
  assert(subroutine.inputs.size() == subroutine.input_expr_ids.size());
  std::unordered_map<ExprId, std::string> names;
  names.reserve(subroutine.inputs.size() + subroutine.captures.size());
  for (size_t i = 0; i < subroutine.inputs.size(); ++i) {
    names.emplace(subroutine.input_expr_ids[i], subroutine.inputs.name(i));
  }
  assert(subroutine.captures.size() == subroutine.capture_expr_ids.size());
  for (size_t i = 0; i < subroutine.captures.size(); ++i) {
    const auto capture = subroutine.captures[i];
    const std::string &name =
        design_.modules.at(subroutine.module_id).node_output_name(capture.node_id, capture.port_idx);
    names.emplace(subroutine.capture_expr_ids[i], name);
  }
  return names;
}

std::unordered_map<ExprId, std::string> TigDumper::get_node_input_names(const Module &module,
                                                                        NodeId node_id) {
  assert(module.node_inputs(node_id).size() == module.node_input_expr_ids(node_id).size());
  std::unordered_map<ExprId, std::string> names;
  names.reserve(module.node_inputs(node_id).size());
  for (size_t port = 0; port < module.node_inputs(node_id).size(); ++port) {
    const auto input = module.node_inputs(node_id)[port];
    const std::string &name = module.node_output_name(input.node_id, input.port_idx);
    assert(!name.empty());
    names.emplace(module.node_input_expr_ids(node_id)[port], name);
  }
  return names;
}

bool TigDumper::lookup_assumed_condition(const ExprGraph &expr_graph, ExprId id,
                                         const std::unordered_map<ExprId, std::string> &names,
                                         const std::unordered_map<std::string, bool> *assumptions,
                                         bool &value) const {
  if (assumptions == nullptr) {
    return false;
  }
  if (id == kInvalidExprId) {
    return false;
  }
  if (expr_graph.op(id) == ExprGraph::Op::kInput) {
    const auto name = names.find(id);
    if (name == names.end()) {
      return false;
    }
    const auto assumption = assumptions->find(name->second);
    if (assumption == assumptions->end()) {
      return false;
    }
    value = assumption->second;
    return true;
  }
  if ((expr_graph.op(id) == ExprGraph::Op::kLogicalNot ||
       (expr_graph.op(id) == ExprGraph::Op::kBitwiseNot && expr_graph.outer_dim(id) == 1)) &&
      expr_graph.operands(id).size() == 1 &&
      lookup_assumed_condition(expr_graph, expr_graph.operands(id)[0], names, assumptions, value)) {
    value = !value;
    return true;
  }
  return false;
}

void TigDumper::emit_expr(std::string_view lhs, bool is_nonblocking, bool is_merge,
                          const ExprGraph &expr_graph, ExprId id, std::ostream &os,
                          std::string_view indent,
                          const std::unordered_map<ExprId, std::string> &input_names,
                          const std::unordered_map<std::string, bool> *assumptions) const {
  std::unordered_map<ExprId, std::string> names = input_names;
  names.reserve(expr_graph.size());
  std::string lhs_name(lhs);
  std::ostringstream decl_os;
  std::ostringstream stmt_os;
  std::ostringstream assign_os;
  const std::string inner_indent = std::string(indent) + "  ";
  emit_expr_unpacked(lhs_name, is_nonblocking, is_merge, expr_graph, id, names, decl_os, stmt_os,
                     assign_os, inner_indent, assumptions);
  const std::string decls = decl_os.str();
  const std::string stmts = stmt_os.str();
  const std::string assigns = assign_os.str();
  if (!decls.empty() || !stmts.empty() || !assigns.empty()) {
    os << indent << "begin\n";
    os << decls;
    os << stmts;
    os << assigns;
    os << indent << "end\n";
  }
}

void TigDumper::emit_exprs(const std::vector<std::string> &lhs_names, bool is_nonblocking,
                           bool is_merge, const ExprGraph &expr_graph,
                           const std::vector<ExprId> &expr_ids, std::ostream &os,
                           std::string_view indent,
                           const std::unordered_map<ExprId, std::string> &input_names,
                           const std::unordered_map<std::string, bool> *assumptions) const {
  assert(lhs_names.size() == expr_ids.size());
  std::unordered_map<ExprId, std::string> names = input_names;
  names.reserve(expr_graph.size());
  std::ostringstream decl_os;
  std::ostringstream stmt_os;
  std::ostringstream assign_os;
  const std::string inner_indent = std::string(indent) + "  ";
  for (size_t i = 0; i < lhs_names.size(); ++i) {
    emit_expr_unpacked(lhs_names[i], is_nonblocking, is_merge, expr_graph, expr_ids[i], names,
                       decl_os, stmt_os, assign_os, inner_indent, assumptions);
  }
  const std::string decls = decl_os.str();
  const std::string stmts = stmt_os.str();
  const std::string assigns = assign_os.str();
  if (!decls.empty() || !stmts.empty() || !assigns.empty()) {
    os << indent << "begin\n";
    os << decls;
    os << stmts;
    os << assigns;
    os << indent << "end\n";
  }
}

void TigDumper::emit_expr_unpacked(const std::string &lhs, bool is_nonblocking, bool is_merge,
                                   const ExprGraph &expr_graph, ExprId id,
                                   std::unordered_map<ExprId, std::string> &names,
                                   std::ostream &decl_os, std::ostream &os, std::ostream &assign_os,
                                   std::string_view indent,
                                   const std::unordered_map<std::string, bool> *assumptions) const {
  if (id == kInvalidExprId) {
    return;
  }
  switch (expr_graph.op(id)) {
  case ExprGraph::Op::kSequence: {
    assert(!expr_graph.operands(id).empty());
    for (ExprId operand : expr_graph.operands(id)) {
      emit_expr_unpacked(lhs, is_nonblocking, is_merge, expr_graph, operand, names, decl_os, os,
                         assign_os, indent, assumptions);
    }
    break;
  }
  case ExprGraph::Op::kConcat: {
    const bool is_partial =
        std::ranges::find(expr_graph.operands(id), kInvalidExprId) != expr_graph.operands(id).end();
    if (!is_partial) {
      const std::string rhs =
          emit_expr_packed(expr_graph, id, names, decl_os, os, indent, assumptions);
      assign_os << indent << lhs << ((is_nonblocking && !is_merge) ? " <= " : " = ") << rhs
                << ";\n";
      return;
    }
    SignalWidth bit = 0;
    for (auto operand = expr_graph.operands(id).rbegin(); operand != expr_graph.operands(id).rend();
         ++operand) {
      assert(*operand == kInvalidExprId || expr_graph.outer_dim(*operand) == 1);
      emit_expr_unpacked(lhs + "[" + std::to_string(bit++) + "]", is_nonblocking, is_merge,
                         expr_graph, *operand, names, decl_os, os, assign_os, indent, assumptions);
    }
    return;
  }
  case ExprGraph::Op::kUnpackedFold: {
    assert(expr_graph.operands(id).size() == 1);
    const ExprId data_id = expr_graph.operands(id).front();
    assert(data_id != kInvalidExprId);
    const bool is_partial = expr_graph.op(data_id) == ExprGraph::Op::kConcat &&
                            std::ranges::find(expr_graph.operands(data_id), kInvalidExprId) !=
                                expr_graph.operands(data_id).end();
    if (!is_partial) {
      const std::string rhs =
          emit_expr_packed(expr_graph, id, names, decl_os, os, indent, assumptions);
      assign_os << indent << lhs << ((is_nonblocking && !is_merge) ? " <= " : " = ") << rhs
                << ";\n";
      return;
    }
    const auto *shape = expr_graph.unpacked_shape(id);
    assert(shape);
    SignalWidth flat_bit = 0;
    for (auto operand = expr_graph.operands(data_id).rbegin();
         operand != expr_graph.operands(data_id).rend(); ++operand) {
      const SignalWidth element = flat_bit / shape->inner_dim;
      SignalWidth remaining = element;
      std::vector<SignalWidth> indices(shape->unpacked_dims.size());
      for (size_t dimension = indices.size(); dimension-- > 0;) {
        indices[dimension] = remaining % shape->unpacked_dims[dimension];
        remaining /= shape->unpacked_dims[dimension];
      }
      std::string selected_lhs = lhs;
      for (SignalWidth index : indices) {
        selected_lhs += "[" + std::to_string(index) + "]";
      }
      selected_lhs += "[" + std::to_string(flat_bit % shape->inner_dim) + "]";
      emit_expr_unpacked(selected_lhs, is_nonblocking, is_merge, expr_graph, *operand, names,
                         decl_os, os, assign_os, indent, assumptions);
      ++flat_bit;
    }
    return;
  }
  case ExprGraph::Op::kGather:
    for (size_t i = 0; i < expr_graph.operands(id).size(); ++i) {
      emit_expr_unpacked(lhs + "[" + std::to_string(i) + "]", is_nonblocking, false, expr_graph,
                         expr_graph.operands(id)[i], names, decl_os, os, assign_os, indent,
                         assumptions);
    }
    break;
  case ExprGraph::Op::kUnpackedAssign: {
    const ExprId next = expr_graph.operands(id)[0];
    const ExprId index = expr_graph.operands(id)[1];
    std::ostringstream selected_lhs;
    selected_lhs << lhs << "["
                 << emit_expr_packed(expr_graph, index, names, decl_os, os, indent, assumptions);
    selected_lhs << "]";
    emit_expr_unpacked(selected_lhs.str(), is_nonblocking, false, expr_graph, next, names, decl_os,
                       os, assign_os, indent, assumptions);
    break;
  }
  case ExprGraph::Op::kUnpackedRangeAssign: {
    const ExprId next = expr_graph.operands(id)[0];
    const auto static_base = expr_graph.unpacked_range_assign_base(id);
    const std::string base =
        static_base ? format_integral_constant(
                          *static_base, ExprBuilder::minimum_unsigned_width(*static_base), false)
                    : emit_expr_packed(expr_graph, expr_graph.operands(id)[1], names, decl_os, os,
                                       indent, assumptions);
    const auto slice_width = static_cast<BitIndex>(expr_graph.slice_width(id));
    std::ostringstream selected_lhs;
    selected_lhs << lhs << "[" << base << " +: "
                 << format_integral_constant(
                        slice_width, ExprBuilder::minimum_unsigned_width(slice_width), false);
    selected_lhs << "]";
    emit_expr_unpacked(selected_lhs.str(), is_nonblocking, false, expr_graph, next, names, decl_os,
                       os, assign_os, indent, assumptions);
    break;
  }
  case ExprGraph::Op::kMaskedAssign: {
    bool use_partial_assignment = false;
    if (kUsePartialAssignmentForMaskedAssign) {
      const ExprId current = expr_graph.operands(id)[0];
      const auto input = names.find(current);
      use_partial_assignment = input != names.end() && input->second == lhs;
    }
    if (!use_partial_assignment) {
      const std::string rhs =
          emit_expr_packed(expr_graph, id, names, decl_os, os, indent, assumptions);
      assign_os << indent << lhs << ((is_nonblocking && !is_merge) ? " <= " : " = ") << rhs
                << ";\n";
      break;
    }
    const ExprId next = expr_graph.operands(id)[1];
    const auto slice_width = static_cast<BitIndex>(expr_graph.slice_width(id));
    std::ostringstream selected_lhs;
    selected_lhs << lhs << "["
                 << emit_affine_index(expr_graph, id, names, decl_os, os, indent, assumptions);
    if (slice_width != 1) {
      selected_lhs << " +: "
                   << format_integral_constant(
                          slice_width, ExprBuilder::minimum_unsigned_width(slice_width), false);
    }
    selected_lhs << "]";
    emit_expr_unpacked(selected_lhs.str(), is_nonblocking, false, expr_graph, next, names, decl_os,
                       os, assign_os, indent, assumptions);
    break;
  }
  case ExprGraph::Op::kMux:
  case ExprGraph::Op::kUnpackedMux: {
    bool assumed = false;
    if (lookup_assumed_condition(expr_graph, expr_graph.operands(id)[0], names, assumptions,
                                 assumed)) {
      emit_expr_unpacked(lhs, is_nonblocking, is_merge, expr_graph,
                         expr_graph.operands(id)[assumed ? 1 : 2], names, decl_os, os, assign_os,
                         indent, assumptions);
      break;
    }
    const std::string cond = emit_expr_packed(expr_graph, expr_graph.operands(id)[0], names,
                                              decl_os, os, indent, assumptions);
    const std::string branch_indent = std::string(indent) + "  ";
    std::ostringstream then_assign_os;
    emit_expr_unpacked(lhs, is_nonblocking, is_merge, expr_graph, expr_graph.operands(id)[1], names,
                       decl_os, os, then_assign_os, branch_indent, assumptions);
    std::ostringstream else_assign_os;
    emit_expr_unpacked(lhs, is_nonblocking, is_merge, expr_graph, expr_graph.operands(id)[2], names,
                       decl_os, os, else_assign_os, branch_indent, assumptions);
    assign_os << indent << "if (" << cond << ") begin\n";
    assign_os << then_assign_os.str();
    assign_os << indent << "end else begin\n";
    assign_os << else_assign_os.str();
    assign_os << indent << "end\n";
    break;
  }
  case ExprGraph::Op::kCase:
  case ExprGraph::Op::kUnpackedCase: {
    assert(!expr_graph.operands(id).empty());
    const bool has_default = expr_graph.operands(id).size() % 2 == 0;
    const std::string selector = emit_expr_packed(expr_graph, expr_graph.operands(id)[0], names,
                                                  decl_os, os, indent, assumptions);
    const std::string branch_indent = std::string(indent) + "  ";
    struct CaseArm {
      std::string label;
      std::string assignments;
    };
    std::vector<CaseArm> arms;
    size_t i = 1;
    while (i + 1 < expr_graph.operands(id).size()) {
      std::ostringstream label;
      if (expr_graph.op(expr_graph.operands(id)[i]) == ExprGraph::Op::kList) {
        for (size_t k = 0; k < expr_graph.operands(expr_graph.operands(id)[i]).size(); ++k) {
          if (k) {
            label << ", ";
          }
          label << emit_expr_packed(expr_graph, expr_graph.operands(expr_graph.operands(id)[i])[k],
                                    names, decl_os, os, indent, assumptions);
        }
      } else {
        label << emit_expr_packed(expr_graph, expr_graph.operands(id)[i], names, decl_os, os,
                                  indent, assumptions);
      }
      std::ostringstream arm_assign_os;
      emit_expr_unpacked(lhs, is_nonblocking, is_merge, expr_graph, expr_graph.operands(id)[i + 1],
                         names, decl_os, os, arm_assign_os, branch_indent, assumptions);
      arms.push_back(CaseArm{label.str(), arm_assign_os.str()});
      i += 2;
    }
    std::ostringstream default_assign_os;
    if (i < expr_graph.operands(id).size()) {
      emit_expr_unpacked(lhs, is_nonblocking, is_merge, expr_graph, expr_graph.operands(id)[i],
                         names, decl_os, os, default_assign_os, branch_indent, assumptions);
    }
    assign_os << indent << "case (" << selector << ")";
    if (!has_default) {
      assign_os << " // synopsys full_case";
    }
    assign_os << "\n";
    for (const CaseArm &arm : arms) {
      assign_os << indent << arm.label << ": begin\n";
      assign_os << arm.assignments;
      assign_os << indent << "end\n";
    }
    if (!default_assign_os.str().empty()) {
      assign_os << indent << "default: begin\n";
      assign_os << default_assign_os.str();
      assign_os << indent << "end\n";
    }
    assign_os << indent << "endcase\n";
    break;
  }
  default: {
    if (kUsePartialAssignmentForMaskedAssign && expr_graph.op(id) == ExprGraph::Op::kInput) {
      const auto input = names.find(id);
      if (input != names.end() && input->second == lhs) {
        break;
      }
    }
    const std::string rhs =
        emit_expr_packed(expr_graph, id, names, decl_os, os, indent, assumptions);
    if (!rhs.empty()) {
      assign_os << indent << lhs << ((is_nonblocking && !is_merge) ? " <= " : " = ") << rhs
                << ";\n";
    }
    break;
  }
  }
}

std::string
TigDumper::emit_expr_packed(const ExprGraph &expr_graph, ExprId id,
                            std::unordered_map<ExprId, std::string> &names, std::ostream &decl_os,
                            std::ostream &os, std::string_view indent,
                            const std::unordered_map<std::string, bool> *assumptions) const {
  if (id == kInvalidExprId) {
    return "";
  }
  if (auto it = names.find(id); it != names.end()) {
    return it->second;
  }

  auto temp_name = [&]() { return naming_.dumper_temporary_signal_prefix + std::to_string(id); };
  auto declare_temp = [&](std::string_view name, const UnpackedShape *unpacked_shape = nullptr) {
    decl_os << indent << "logic ";
    const bool sign = unpacked_shape ? unpacked_shape->inner_sign : expr_graph.outer_sign(id);
    const SignalWidth width = unpacked_shape ? unpacked_shape->inner_dim : expr_graph.outer_dim(id);
    if (sign) {
      decl_os << "signed ";
    }
    if (width > 1) {
      decl_os << "[" << (width - 1) << ":0] ";
    }
    decl_os << name;
    if (unpacked_shape != nullptr) {
      for (const SignalWidth dim : unpacked_shape->unpacked_dims) {
        decl_os << " [0:" << (dim - 1) << "]";
      }
    }
    decl_os << ";\n";
  };

  auto emit_unary = [&](const char *op) {
    const std::string operand = emit_expr_packed(expr_graph, expr_graph.operands(id)[0], names,
                                                 decl_os, os, indent, assumptions);
    const std::string name = temp_name();
    declare_temp(name);
    os << indent << name << " = (" << op << operand << ");\n";
    names[id] = name;
    return name;
  };
  auto emit_bin = [&](const char *op) {
    const std::string lhs_name = emit_expr_packed(expr_graph, expr_graph.operands(id)[0], names,
                                                  decl_os, os, indent, assumptions);
    const std::string rhs_name = emit_expr_packed(expr_graph, expr_graph.operands(id)[1], names,
                                                  decl_os, os, indent, assumptions);
    const std::string name = temp_name();
    declare_temp(name);
    os << indent << name << " = (" << lhs_name << " " << op << " " << rhs_name << ");\n";
    names[id] = name;
    return name;
  };
  auto emit_variadic = [&](const char *op) {
    std::vector<std::string> operand_names;
    operand_names.reserve(expr_graph.operands(id).size());
    for (ExprId operand : expr_graph.operands(id)) {
      operand_names.push_back(
          emit_expr_packed(expr_graph, operand, names, decl_os, os, indent, assumptions));
    }
    const std::string name = temp_name();
    declare_temp(name);
    os << indent << name << " = (";
    for (size_t i = 0; i < operand_names.size(); ++i) {
      if (i) {
        os << " " << op << " ";
      }
      os << operand_names[i];
    }
    os << ");\n";
    names[id] = name;
    return name;
  };

  switch (expr_graph.op(id)) {
  case ExprGraph::Op::kInput:
    diagnostics_.error(DiagnosticId::kEmitterMissingExpressionValueReplacedWithZero, "input name");
    names[id] = "1'b0";
    return names[id];
  case ExprGraph::Op::kConst:
    return names[id] = expr_graph.constant_value(id);
  case ExprGraph::Op::kSequence: {
    const UnpackedShape *unpacked_shape = expr_graph.unpacked_shape(id);
    assert(!expr_graph.operands(id).empty());
    const std::string name = temp_name();
    declare_temp(name, unpacked_shape);
    emit_expr_unpacked(name, false, false, expr_graph, id, names, decl_os, os, os, indent,
                       assumptions);
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kLogicalNot: {
    return emit_unary("!");
  }
  case ExprGraph::Op::kBitwiseNot:
    return emit_unary("~");
  case ExprGraph::Op::kAndReduce:
    return emit_unary("&");
  case ExprGraph::Op::kOrReduce:
    return emit_unary("|");
  case ExprGraph::Op::kXorReduce:
    return emit_unary("^");
  case ExprGraph::Op::kUnaryMinus:
    return emit_unary("-");
  case ExprGraph::Op::kAdd:
    return emit_bin("+");
  case ExprGraph::Op::kSub:
    return emit_bin("-");
  case ExprGraph::Op::kMul:
    return emit_bin("*");
  case ExprGraph::Op::kDiv:
    return emit_bin("/");
  case ExprGraph::Op::kMod:
    return emit_bin("%");
  case ExprGraph::Op::kPow:
    return emit_bin("**");
  case ExprGraph::Op::kShl:
    return emit_bin("<<");
  case ExprGraph::Op::kShr:
    return emit_bin(">>");
  case ExprGraph::Op::kAshr: {
    const std::string lhs_name = emit_expr_packed(expr_graph, expr_graph.operands(id)[0], names,
                                                  decl_os, os, indent, assumptions);
    const std::string rhs_name = emit_expr_packed(expr_graph, expr_graph.operands(id)[1], names,
                                                  decl_os, os, indent, assumptions);
    const std::string name = temp_name();
    declare_temp(name);
    os << indent << name << " = ($signed(" << lhs_name << ") >>> " << rhs_name << ");\n";
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kEq:
    return emit_bin("==");
  case ExprGraph::Op::kLt:
    return emit_bin("<");
  case ExprGraph::Op::kLe:
    return emit_bin("<=");
  case ExprGraph::Op::kAnd:
    return emit_variadic("&");
  case ExprGraph::Op::kOr:
    return emit_variadic("|");
  case ExprGraph::Op::kXor:
    return emit_variadic("^");
  case ExprGraph::Op::kConvert: {
    const std::string operand = emit_expr_packed(expr_graph, expr_graph.operands(id)[0], names,
                                                 decl_os, os, indent, assumptions);
    const std::string name = temp_name();
    declare_temp(name);
    os << indent << name << " = " << operand << ";\n";
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kConcat: {
    std::vector<std::string> operand_names;
    operand_names.reserve(expr_graph.operands(id).size());
    for (ExprId operand : expr_graph.operands(id)) {
      operand_names.push_back(
          emit_expr_packed(expr_graph, operand, names, decl_os, os, indent, assumptions));
    }
    const std::string name = temp_name();
    declare_temp(name);
    os << indent << name << " = {";
    for (size_t i = 0; i < operand_names.size(); ++i) {
      if (i) {
        os << ", ";
      }
      os << operand_names[i];
    }
    os << "};\n";
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kGather: {
    std::vector<std::string> operand_names;
    operand_names.reserve(expr_graph.operands(id).size());
    for (ExprId operand : expr_graph.operands(id)) {
      operand_names.push_back(
          emit_expr_packed(expr_graph, operand, names, decl_os, os, indent, assumptions));
    }
    const std::string name = temp_name();
    const UnpackedShape *unpacked_shape = expr_graph.unpacked_shape(id);
    assert(unpacked_shape != nullptr);
    declare_temp(name, unpacked_shape);
    os << indent << name << " = '{";
    for (size_t i = 0; i < operand_names.size(); ++i) {
      if (i) {
        os << ", ";
      }
      os << operand_names[i];
    }
    os << "};\n";
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kUnpackedConcat: {
    std::vector<std::string> operand_names;
    operand_names.reserve(expr_graph.operands(id).size());
    for (ExprId operand : expr_graph.operands(id)) {
      operand_names.push_back(
          emit_expr_packed(expr_graph, operand, names, decl_os, os, indent, assumptions));
    }
    const std::string name = temp_name();
    const UnpackedShape *unpacked_shape = expr_graph.unpacked_shape(id);
    assert(unpacked_shape != nullptr);
    declare_temp(name, unpacked_shape);
    os << indent << name << " = {";
    for (size_t i = 0; i < operand_names.size(); ++i) {
      if (i) {
        os << ", ";
      }
      os << operand_names[i];
    }
    os << "};\n";
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kUnpackedFlatten: {
    assert(expr_graph.operands(id).size() == 1);
    const ExprId data_id = expr_graph.operands(id)[0];
    const UnpackedShape *shape = expr_graph.unpacked_shape(data_id);
    assert(shape != nullptr);
    const std::string data =
        emit_expr_packed(expr_graph, data_id, names, decl_os, os, indent, assumptions);
    const std::string name = temp_name();
    declare_temp(name);
    SignalWidth element_count = 1;
    for (SignalWidth dimension : shape->unpacked_dims) {
      element_count *= dimension;
    }
    for (SignalWidth element = 0; element < element_count; ++element) {
      SignalWidth remaining = element;
      std::vector<SignalWidth> indices(shape->unpacked_dims.size());
      for (size_t dimension = shape->unpacked_dims.size(); dimension-- > 0;) {
        indices[dimension] = remaining % shape->unpacked_dims[dimension];
        remaining /= shape->unpacked_dims[dimension];
      }
      os << indent << name << "[" << (element * shape->inner_dim) << " +: " << shape->inner_dim
         << "] = " << data;
      for (SignalWidth index : indices) {
        os << "[" << index << "]";
      }
      os << ";\n";
    }
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kUnpackedFold: {
    assert(expr_graph.operands(id).size() == 1);
    const UnpackedShape *shape = expr_graph.unpacked_shape(id);
    assert(shape != nullptr);
    std::string data = emit_expr_packed(expr_graph, expr_graph.operands(id)[0], names, decl_os, os,
                                        indent, assumptions);
    if (!can_emit_direct_select_source(expr_graph, expr_graph.operands(id)[0])) {
      data = "{" + data + "}";
    }
    const std::string name = temp_name();
    declare_temp(name, shape);
    SignalWidth element_count = 1;
    for (SignalWidth dimension : shape->unpacked_dims) {
      element_count *= dimension;
    }
    for (SignalWidth element = 0; element < element_count; ++element) {
      SignalWidth remaining = element;
      std::vector<SignalWidth> indices(shape->unpacked_dims.size());
      for (size_t dimension = shape->unpacked_dims.size(); dimension-- > 0;) {
        indices[dimension] = remaining % shape->unpacked_dims[dimension];
        remaining /= shape->unpacked_dims[dimension];
      }
      os << indent << name;
      for (SignalWidth index : indices) {
        os << "[" << index << "]";
      }
      os << " = " << data << "[" << (element * shape->inner_dim) << " +: " << shape->inner_dim << "];\n";
    }
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kRange: {
    const ExprId data_id = expr_graph.operands(id)[0];
    const std::string name = temp_name();
    declare_temp(name);
    const std::string data =
        emit_expr_packed(expr_graph, data_id, names, decl_os, os, indent, assumptions);
    const std::string base =
        emit_affine_index(expr_graph, id, names, decl_os, os, indent, assumptions);
    if (!kUseShiftMaskForExpressionSelects || can_emit_direct_select_source(expr_graph, data_id)) {
      os << indent << name << " = " << data << "[" << base;
      if (expr_graph.outer_dim(id) > 1) {
        os << " +: " << expr_graph.outer_dim(id);
      }
      os << "]";
    } else {
      os << indent << name << " = ((" << data << " >> (" << base << ")) & {" << expr_graph.outer_dim(id)
         << "{1'b1}})";
    }
    os << ";\n";
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kUnpackedSelect: {
    const std::string data = emit_expr_packed(expr_graph, expr_graph.operands(id)[0], names,
                                              decl_os, os, indent, assumptions);
    const std::string index = emit_expr_packed(expr_graph, expr_graph.operands(id)[1], names,
                                               decl_os, os, indent, assumptions);
    const std::string name = temp_name();
    const UnpackedShape *unpacked_shape = expr_graph.unpacked_shape(id);
    declare_temp(name, unpacked_shape);
    os << indent << name << " = " << data << "[" << index << "];\n";
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kUnpackedRange: {
    const std::string data = emit_expr_packed(expr_graph, expr_graph.operands(id)[0], names,
                                              decl_os, os, indent, assumptions);
    const std::string base = emit_expr_packed(expr_graph, expr_graph.operands(id)[1], names,
                                              decl_os, os, indent, assumptions);
    const UnpackedShape *unpacked_shape = expr_graph.unpacked_shape(id);
    assert(unpacked_shape != nullptr);
    const std::string name = temp_name();
    declare_temp(name, unpacked_shape);
    os << indent << name << " = " << data << "[" << base << " +: " << expr_graph.outer_dim(id)
       << "];\n";
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kReverse: {
    const ExprId operand_id = expr_graph.operands(id)[0];
    const std::string operand =
        emit_expr_packed(expr_graph, operand_id, names, decl_os, os, indent, assumptions);
    const UnpackedShape *unpacked_shape = expr_graph.unpacked_shape(id);
    if (unpacked_shape != nullptr) {
      const std::string name = temp_name();
      declare_temp(name, unpacked_shape);
      for (SignalWidth i = 0; i < expr_graph.outer_dim(id); ++i) {
        os << indent << name << "[" << i << "] = " << operand << "["
           << (expr_graph.outer_dim(id) - 1 - i) << "];\n";
      }
      names[id] = name;
      return name;
    }
    if (expr_graph.outer_dim(operand_id) <= 1) {
      names[id] = operand;
      return operand;
    }
    const std::string name = temp_name();
    declare_temp(name);
    os << indent << name << " = {";
    for (SignalWidth i = 0; i < expr_graph.outer_dim(operand_id); ++i) {
      if (i) {
        os << ", ";
      }
      os << operand << "[" << i << "]";
    }
    os << "};\n";
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kCall: {
    const auto target = expr_graph.call_target(id);
    if (target >= design_.subroutines.size()) {
      diagnostics_.error(DiagnosticId::kEmitterUnsupportedExpressionReplacedWithZero);
      names[id] = "1'b0";
      return names[id];
    }
    const auto &subroutine = design_.subroutines[target];
    const std::string name = temp_name();
    declare_temp(name);
    std::vector<std::string> operands;
    operands.reserve(expr_graph.operands(id).size());
    for (ExprId operand : expr_graph.operands(id)) {
      operands.push_back(
          emit_expr_packed(expr_graph, operand, names, decl_os, os, indent, assumptions));
    }
    os << indent << name << " = " << subroutine.name << subroutine.variant_suffix << "(";
    for (size_t i = 0; i < operands.size(); ++i) {
      if (i != 0) {
        os << ", ";
      }
      os << operands[i];
    }
    os << ");\n";
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kUnpackedMux: {
    assert(expr_graph.operands(id).size() == 3);
    const std::string cond = emit_expr_packed(expr_graph, expr_graph.operands(id)[0], names,
                                              decl_os, os, indent, assumptions);
    const std::string name = temp_name();
    const UnpackedShape *unpacked_shape = expr_graph.unpacked_shape(id);
    assert(unpacked_shape != nullptr);
    declare_temp(name, unpacked_shape);
    const std::string branch_indent = std::string(indent) + "  ";
    std::ostringstream then_assign_os;
    emit_expr_unpacked(name, false, false, expr_graph, expr_graph.operands(id)[1], names, decl_os,
                       os, then_assign_os, branch_indent, assumptions);
    std::ostringstream else_assign_os;
    emit_expr_unpacked(name, false, false, expr_graph, expr_graph.operands(id)[2], names, decl_os,
                       os, else_assign_os, branch_indent, assumptions);
    os << indent << "if (" << cond << ") begin\n";
    os << then_assign_os.str();
    os << indent << "end else begin\n";
    os << else_assign_os.str();
    os << indent << "end\n";
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kMux: {
    bool assumed = false;
    if (lookup_assumed_condition(expr_graph, expr_graph.operands(id)[0], names, assumptions,
                                 assumed)) {
      const std::string selected =
          emit_expr_packed(expr_graph, expr_graph.operands(id)[assumed ? 1 : 2], names, decl_os, os,
                           indent, assumptions);
      names[id] = selected;
      return selected;
    }
    const std::string cond = emit_expr_packed(expr_graph, expr_graph.operands(id)[0], names,
                                              decl_os, os, indent, assumptions);
    const std::string then_name = emit_expr_packed(expr_graph, expr_graph.operands(id)[1], names,
                                                   decl_os, os, indent, assumptions);
    const std::string else_name = emit_expr_packed(expr_graph, expr_graph.operands(id)[2], names,
                                                   decl_os, os, indent, assumptions);
    const std::string name = temp_name();
    declare_temp(name);
    os << indent << "if (" << cond << ") begin\n";
    if (!then_name.empty()) {
      os << indent << "  " << name << " = " << then_name << ";\n";
    }
    os << indent << "end else begin\n";
    if (!else_name.empty()) {
      os << indent << "  " << name << " = " << else_name << ";\n";
    }
    os << indent << "end\n";
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kPmux: {
    assert(expr_graph.operands(id).size() >= 2);
    const std::string name = temp_name();
    declare_temp(name);
    std::vector<std::string> conditions;
    std::vector<std::string> data;
    size_t operand = 0;
    while (operand + 1 < expr_graph.operands(id).size()) {
      conditions.push_back(emit_expr_packed(expr_graph, expr_graph.operands(id)[operand], names,
                                            decl_os, os, indent, assumptions));
      data.push_back(emit_expr_packed(expr_graph, expr_graph.operands(id)[operand + 1], names,
                                      decl_os, os, indent, assumptions));
      operand += 2;
    }
    std::string default_data;
    if (operand < expr_graph.operands(id).size()) {
      default_data = emit_expr_packed(expr_graph, expr_graph.operands(id)[operand], names, decl_os,
                                      os, indent, assumptions);
    }
    for (size_t i = 0; i < conditions.size(); ++i) {
      os << indent << (i == 0 ? "if (" : "else if (") << conditions[i] << ") begin\n";
      if (!data[i].empty()) {
        os << indent << "  " << name << " = " << data[i] << ";\n";
      }
      os << indent << "end\n";
    }
    if (operand < expr_graph.operands(id).size()) {
      os << indent << "else begin\n";
      if (!default_data.empty()) {
        os << indent << "  " << name << " = " << default_data << ";\n";
      }
      os << indent << "end\n";
    }
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kCase:
  case ExprGraph::Op::kUnpackedCase: {
    const bool has_default = expr_graph.operands(id).size() % 2 == 0;
    const std::string selector = emit_expr_packed(expr_graph, expr_graph.operands(id)[0], names,
                                                  decl_os, os, indent, assumptions);
    const std::string name = temp_name();
    declare_temp(name);
    const std::string branch_indent = std::string(indent) + "  ";
    struct CaseArm {
      std::string label;
      std::string data;
    };
    std::vector<CaseArm> arms;
    size_t i = 1;
    while (i + 1 < expr_graph.operands(id).size()) {
      const ExprId value_id = expr_graph.operands(id)[i];
      const ExprId data_id = expr_graph.operands(id)[i + 1];
      const std::string data_name =
          emit_expr_packed(expr_graph, data_id, names, decl_os, os, indent, assumptions);
      if (!data_name.empty()) {
        std::ostringstream label;
        if (expr_graph.op(value_id) == ExprGraph::Op::kList) {
          for (size_t k = 0; k < expr_graph.operands(value_id).size(); ++k) {
            if (k) {
              label << ", ";
            }
            label << emit_expr_packed(expr_graph, expr_graph.operands(value_id)[k], names, decl_os,
                                      os, indent, assumptions);
          }
        } else {
          label << emit_expr_packed(expr_graph, value_id, names, decl_os, os, indent, assumptions);
        }
        arms.push_back(CaseArm{label.str(), data_name});
      }
      i += 2;
    }
    std::string default_data;
    if (i < expr_graph.operands(id).size()) {
      default_data = emit_expr_packed(expr_graph, expr_graph.operands(id)[i], names, decl_os, os,
                                      indent, assumptions);
    }
    os << indent << "case (" << selector << ")";
    if (!has_default) {
      os << " // synopsys full_case";
    }
    os << "\n";
    for (const CaseArm &arm : arms) {
      os << indent << arm.label << ": begin\n";
      os << branch_indent << name << " = " << arm.data << ";\n";
      os << indent << "end\n";
    }
    if (!default_data.empty()) {
      os << indent << "default: begin\n";
      os << branch_indent << name << " = " << default_data << ";\n";
      os << indent << "end\n";
    }
    os << indent << "endcase\n";
    names[id] = name;
    return name;
  }
  case ExprGraph::Op::kMaskedAssign: {
    const ExprId current_id = expr_graph.operands(id)[0];
    const ExprId next_id = expr_graph.operands(id)[1];
    const auto slice_width = static_cast<BitIndex>(expr_graph.slice_width(id));
    const std::string current =
        emit_expr_packed(expr_graph, current_id, names, decl_os, os, indent, assumptions);
    const std::string next =
        emit_expr_packed(expr_graph, next_id, names, decl_os, os, indent, assumptions);
    const std::string base =
        emit_affine_index(expr_graph, id, names, decl_os, os, indent, assumptions);
    const std::string name = temp_name();
    declare_temp(name);
    if (!current.empty()) {
      os << indent << name << " = " << current << ";\n";
    }
    os << indent << name << "[" << base;
    if (slice_width != 1) {
      os << " +: "
         << format_integral_constant(slice_width, ExprBuilder::minimum_unsigned_width(slice_width),
                                     false);
    }
    os << "] = " << next << ";\n";
    names[id] = name;
    return name;
  }
  default:
    diagnostics_.error(DiagnosticId::kEmitterUnsupportedExpressionReplacedWithZero);
    names[id] = "1'b0";
    return names[id];
  }
}

std::string
TigDumper::emit_affine_index(const ExprGraph &expr_graph, ExprId id,
                             std::unordered_map<ExprId, std::string> &names, std::ostream &decl_os,
                             std::ostream &os, std::string_view indent,
                             const std::unordered_map<std::string, bool> *assumptions) const {
  const auto &operands = expr_graph.operands(id);
  const auto &strides = expr_graph.affine_strides(id);
  const size_t first = expr_graph.op(id) == ExprGraph::Op::kRange ? 1 : 2;
  assert(operands.size() == first + strides.size());
  const BitIndex offset = expr_graph.affine_offset(id);
  const auto offset_width = offset < 0 ? ExprBuilder::minimum_signed_width(offset)
                                       : ExprBuilder::minimum_unsigned_width(offset);
  const auto offset_literal = format_integral_constant(offset, offset_width, offset < 0);
  if (strides.empty()) {
    return offset_literal;
  }

  SignalWidth width = offset_width;
  for (size_t i = 0; i < strides.size(); ++i) {
    const auto index = operands[first + i];
    width = std::max(width, expr_graph.outer_dim(index) + (expr_graph.outer_sign(index) ? 0 : 1) +
                                ExprBuilder::minimum_signed_width(strides[i]));
  }
  width += ExprBuilder::minimum_unsigned_width(static_cast<BitIndex>(strides.size())) + 1;
  auto extend = [width](const std::string &value, bool sign) {
    if (!sign) {
      return "$signed(" + std::to_string(width) + "'({1'b0, " + value + "}))";
    }
    return "$signed(" + std::to_string(width) + "'($signed(" + value + ")))";
  };
  std::string result = extend(offset_literal, offset < 0);
  for (size_t i = 0; i < strides.size(); ++i) {
    const auto index = operands[first + i];
    const auto value = emit_expr_packed(expr_graph, index, names, decl_os, os, indent, assumptions);
    const auto stride =
        format_integral_constant(strides[i], ExprBuilder::minimum_signed_width(strides[i]), true);
    result = "(" + result + " + (" + extend(value, expr_graph.outer_sign(index)) + " * " +
             extend(stride, true) + "))";
  }
  return result;
}

bool TigDumper::can_emit_direct_select_source(const ExprGraph &expr_graph, ExprId id) {
  if (id == kInvalidExprId) {
    return false;
  }
  return expr_graph.op(id) == ExprGraph::Op::kInput ||
         expr_graph.op(id) == ExprGraph::Op::kUnpackedSelect;
}

void TigDumper::emit_module_footer(std::ostream &os) {
  os << "endmodule\n";
}

} // namespace abys::ir
