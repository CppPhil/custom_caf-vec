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
} // namespace vec
