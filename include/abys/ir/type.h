#pragma once

#include <cassert>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace abys::ir {

using PortIndex = uint32_t;
using SignalWidth = uint64_t; // TODO: add a build-time option to use 32-bit widths for performance.
using BitIndex = int64_t;
using ExprId = uint32_t;
using SubrId = uint32_t;

struct UnpackedShape {
  std::vector<SignalWidth> unpacked_dims;
  SignalWidth width = 0;
  bool sign = false;

  SignalWidth flattened_width() const {
    SignalWidth result = width;
    for (SignalWidth dimension : unpacked_dims) {
      assert(dimension > 0);
      assert(result <= std::numeric_limits<SignalWidth>::max() / dimension);
      result *= dimension;
    }
    return result;
  }
};

enum class EdgeKind : uint8_t { kNone, kPosedge, kNegedge, kBothEdges };

constexpr int kSignalWidthBitSize = 64;

static constexpr ExprId kInvalidExprId = std::numeric_limits<ExprId>::max();
static constexpr SubrId kInvalidSubrId = std::numeric_limits<SubrId>::max();
static constexpr PortIndex kInvalidPortIndex = std::numeric_limits<PortIndex>::max();

} // namespace abys::ir
