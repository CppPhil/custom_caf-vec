#pragma once
#include "entity_set_range.hpp"

namespace vec {
class node_range : public entity_set_range {
public:
  node_range(const entity_set& xs, const caf::node_id& y);
};
} // namespace vec
