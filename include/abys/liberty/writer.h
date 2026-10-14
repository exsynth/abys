#pragma once

#include <iosfwd>

#include "abys/liberty/library.h"

namespace abys::liberty {

void write(const CellLibrary &library, std::ostream &output);

} // namespace abys::liberty
