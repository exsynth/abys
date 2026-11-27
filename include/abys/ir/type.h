#pragma once

#include <array>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>
#include <vector>

namespace abys::ir {

using PortIndex = uint32_t;
using SignalWidth = uint64_t; // TODO: add a build-time option to use 32-bit widths for performance.
using BitIndex = int64_t;
using NodeId = uint32_t;
using ModuleId = uint32_t;
using ExprId = uint32_t;
using SubrId = uint32_t;

static constexpr NodeId kInvalidNodeId = std::numeric_limits<NodeId>::max();
static constexpr ModuleId kInvalidModuleId = std::numeric_limits<ModuleId>::max();
static constexpr ExprId kInvalidExprId = std::numeric_limits<ExprId>::max();
static constexpr SubrId kInvalidSubrId = std::numeric_limits<SubrId>::max();
static constexpr PortIndex kInvalidPortIndex = std::numeric_limits<PortIndex>::max();

enum class EdgeKind : uint8_t { kNone, kPosedge, kNegedge, kBothEdges };

constexpr int kSignalWidthBitSize = 64;

struct EdgeRef {
  NodeId node_id = kInvalidNodeId;
  PortIndex port_idx = 0;
};

struct AffineIndex {
  struct Term {
    ExprId index;
    BitIndex stride;
  };

  BitIndex offset = 0;
  std::vector<Term> terms;

  void add(const AffineIndex &source) {
    if (this == &source) {
      const AffineIndex copy = source;
      add(copy);
      return;
    }
    terms.insert(terms.end(), source.terms.begin(), source.terms.end());
    add_offset(source.offset);
  }

  void add_offset(BitIndex delta) { offset += delta; }
};

struct UnpackedShape {
  std::vector<SignalWidth> unpacked_dims;
  SignalWidth inner_dim = 0;
  bool inner_sign = false;

  SignalWidth flattened_width() const {
    SignalWidth result = inner_dim;
    for (SignalWidth dimension : unpacked_dims) {
      assert(dimension > 0);
      assert(result <= std::numeric_limits<SignalWidth>::max() / dimension);
      result *= dimension;
    }
    return result;
  }
};

inline std::string format_integral_constant(BitIndex index, SignalWidth width, bool sign) {
  using UnsignedBitIndex = std::make_unsigned_t<BitIndex>;
  const bool negative = index < 0;
  assert(!negative || sign);
  const UnsignedBitIndex magnitude = negative ? static_cast<UnsignedBitIndex>(-(index + 1)) + 1
                                              : static_cast<UnsignedBitIndex>(index);
  std::array<char, kSignalWidthBitSize> digits;
  const auto [end, error] =
      std::to_chars(digits.data(), digits.data() + digits.size(), magnitude, 2);
  assert(error == std::errc{});
  [[maybe_unused]] const SignalWidth digit_count = static_cast<SignalWidth>(end - digits.data());
  assert(digit_count <= width);
  assert(!sign || magnitude == 0 || digit_count < width);
  std::string literal;
  if (negative) {
    literal += '-';
  }
  literal += std::to_string(width) + "'";
  if (sign) {
    literal += 's';
  }
  literal += 'b';
  literal.append(digits.data(), end);
  return literal;
}

} // namespace abys::ir
