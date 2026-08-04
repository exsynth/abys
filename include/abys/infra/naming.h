#pragma once

#include <string>

namespace abys {

struct NamingOptions {
  std::string lowering_scope_marker = "_abys";
  std::string lowering_scope_separator = "_";
  std::string lowering_anonymous_block_name = "block";
  std::string builder_temporary_signal_prefix = "abys_builder_tmp";
  std::string builder_module_variant_prefix = "_abys_variant";
  std::string dumper_temporary_signal_prefix = "abys_dumper_tmp";
};

} // namespace abys
