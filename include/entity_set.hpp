#pragma once
#include <set>

#include "entity.hpp"

namespace vec {
/// Set of `entity` sorted in ascending order by node ID, actor ID,
/// and thread ID (in that order).
using entity_set = std::set<entity>;

template <class OutputStream>
OutputStream& operator<<(OutputStream& os, const entity_set& set) {
  os << "set{\n";

  for (const auto& entity : set) {
    os << entity << ",\n";
  }

  return os << '}';
}
} // namespace vec
