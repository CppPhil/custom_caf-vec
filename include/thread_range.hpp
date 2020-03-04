#pragma once
#include "entity_set_range.hpp"
#include "node_range.hpp"

namespace vec {
/// Range within an `entity_set` containing all entities for a given node.
class thread_range : public entity_set_range {
public:
  explicit thread_range(const node_range& xs);
};
} // namespace vec
