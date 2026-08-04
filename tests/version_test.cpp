#include "catch_amalgamated.hpp"

#include "abys/version.h"

TEST_CASE("version is available", "[version]") {
  CHECK_FALSE(abys::version().empty());
}
