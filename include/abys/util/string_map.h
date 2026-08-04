#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace abys::util {

struct TransparentStringHash {
  using is_transparent = void;

  size_t operator()(std::string_view s) const noexcept { return std::hash<std::string_view>{}(s); }
  size_t operator()(const std::string &s) const noexcept {
    return std::hash<std::string_view>{}(s);
  }
  size_t operator()(const char *s) const noexcept { return std::hash<std::string_view>{}(s); }
};

struct TransparentStringEq {
  using is_transparent = void;

  bool operator()(std::string_view a, std::string_view b) const noexcept { return a == b; }
};

template <typename T>
using StringMap = std::unordered_map<std::string, T, TransparentStringHash, TransparentStringEq>;

} // namespace abys::util
