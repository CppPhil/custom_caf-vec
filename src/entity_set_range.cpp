#include "entity_set_range.hpp"

namespace vec {
entity_set_range::iterator entity_set_range::begin() const {
  return begin_;
}

entity_set_range::iterator entity_set_range::end() const {
  return end_;
}
} // namespace vec
