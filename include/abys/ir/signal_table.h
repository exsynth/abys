#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "abys/ir/type.h"

namespace abys::ir {

class SignalTable {
public:
  size_t size() const { return names_.size(); }
  const std::vector<std::string> &names() const { return names_; }
  const std::string &name(size_t id) const { return names_.at(id); }
  SignalWidth outer_dim(size_t id) const { return outer_dims_.at(id); }
  bool sign(size_t id) const { return signs_.at(id); }

  const UnpackedShape *shape(size_t id) const {
    const UnpackedShapeId shape_id = unpacked_shape_ids_.at(id);
    return shape_id == kInvalidShape ? nullptr : &unpacked_shapes_[shape_id];
  }

  std::optional<size_t> find_index(std::string_view name) const {
    return find_index(name, std::hash<std::string_view>{}(name));
  }

  [[nodiscard]] bool append(std::string name, SignalWidth outer_dim, bool sign,
                            const UnpackedShape *shape = nullptr) {
    assert(!shape ||
           (!sign && !shape->unpacked_dims.empty() && shape->unpacked_dims.front() == outer_dim));
    const size_t hash = std::hash<std::string_view>{}(name);
    if (find_index(name, hash)) {
      return false;
    }
    name_indices_.emplace(hash, size());
    names_.push_back(std::move(name));
    outer_dims_.push_back(outer_dim);
    signs_.push_back(sign);
    if (!shape) {
      unpacked_shape_ids_.push_back(kInvalidShape);
    } else {
      unpacked_shape_ids_.push_back(unpacked_shapes_.size());
      unpacked_shapes_.push_back(*shape);
    }
    return true;
  }

private:
  using UnpackedShapeId = size_t;
  static constexpr auto kInvalidShape = std::numeric_limits<UnpackedShapeId>::max();

  std::optional<size_t> find_index(std::string_view name, size_t hash) const {
    const auto [begin, end] = name_indices_.equal_range(hash);
    for (auto it = begin; it != end; ++it) {
      if (names_[it->second] == name) {
        return it->second;
      }
    }
    return std::nullopt;
  }

  std::vector<std::string> names_;
  std::vector<SignalWidth> outer_dims_;
  std::vector<bool> signs_;
  std::vector<UnpackedShapeId> unpacked_shape_ids_;
  std::vector<UnpackedShape> unpacked_shapes_;
  std::unordered_multimap<size_t, size_t> name_indices_;
};

} // namespace abys::ir
