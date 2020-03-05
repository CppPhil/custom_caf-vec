#pragma once
#include "entity_set.hpp"

namespace vec {
class entity_set_range {
public:
  using iterator = entity_set::const_iterator;

  [[nodiscard]] iterator begin() const;

  [[nodiscard]] iterator end() const;

protected:
  iterator begin_;
  iterator end_;
};

template <class OutputStream>
OutputStream& operator<<(OutputStream& os, const entity_set_range& range) {
  os << "entity_set_range{\n";

  for (const auto& element : range) {
    os << element << ",\n";
  }

  return os << '}';
}
} // namespace vec
