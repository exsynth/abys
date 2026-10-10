#pragma once

#include <cstddef>
#include <iosfwd>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "abys/infra/diagnostics.h"
#include "abys/liberty/library.h"
#include "boop/library/cell_library.h"

namespace abys::mapping {

class Library {
public:
  struct Options {
    std::optional<double> input_slew;
  };

  enum class ExclusionReason {
    kTooManyInputs,
    kMissingFunction,
    kThreeState,
    kUnsupportedFunction,
  };

  struct ExcludedCell {
    std::string name;
    ExclusionReason reason;
  };

  Library(const liberty::CellLibrary &library, Diagnostics &diagnostics,
          const Options &options = {});

  Library(const Library &) = delete;
  Library &operator=(const Library &) = delete;
  Library(Library &&) noexcept = default;
  Library &operator=(Library &&) noexcept = default;

  size_t cell_count() const;
  std::span<const ExcludedCell> excluded_cells() const;
  const boop::CellLibrary &cells() const;
  void write_genlib(std::ostream &output) const;

private:
  boop::CellLibrary cells_;
  std::vector<ExcludedCell> excluded_cells_;
};

} // namespace abys::mapping
