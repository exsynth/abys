#pragma once

#include "abys/infra/diagnostics.h"
#include "abys/ir/tig.h"

namespace abys::ir {

class TigTransformer {
public:
  TigTransformer(Tig &design, Diagnostics &diagnostics);

  void flatten_subroutine();

private:
  Tig &design_;
  Diagnostics &diagnostics_;
};

} // namespace abys::ir
