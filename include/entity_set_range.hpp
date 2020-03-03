#pragma once
#include "entity_set.hpp"

namespace vec {
class entity_set_range {
public:
  using iterator = entity_set::const_iterator;

  entity_set_range();
  entity_set_range(const entity_set_range&);
  entity_set_range& operator=(const entity_set_range&);

  iterator begin() const;

  iterator end() const;

protected:
  iterator begin_;
  iterator end_;
};
} // namespace vec
