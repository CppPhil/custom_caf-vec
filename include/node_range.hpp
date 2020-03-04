#pragma once
#include "entity_set_range.hpp"

namespace vec {
class node_range : public entity_set_range {
public:
  node_range(const entity_set& xs, const caf::node_id& y);

  node_range(const node_range&);
  node_range& operator=(const node_range&);

  [[nodiscard]] const caf::node_id& node() const;

private:
  caf::node_id node_;
};
} // namespace vec
