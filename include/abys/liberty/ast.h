#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace abys::liberty {

struct Value {
  std::string text;
  bool quoted = false;
};

enum class AttributeKind { kSimple, kComplex, kVariable };

struct Attribute {
  std::string name;
  AttributeKind kind = AttributeKind::kSimple;
  std::vector<Value> values;
  size_t line = 0;
  size_t order = 0;
};

struct Group {
  std::string name;
  std::vector<Value> arguments;
  std::vector<Attribute> attributes;
  std::vector<Group> groups;
  size_t line = 0;
  size_t order = 0;
};

struct Ast {
  std::vector<Group> groups;
};

} // namespace abys::liberty
