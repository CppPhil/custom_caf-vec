#pragma once
#include "entity_set_range.hpp"
#include "node_range.hpp"

namespace vec {
/// Range within an `entity_set` containing all entities for a given node.
class thread_range : public entity_set_range {
public:
  explicit thread_range(const node_range& xs);

  thread_range(const thread_range&);
  thread_range& operator=(const thread_range&);

  [[nodiscard]] const caf::node_id& node() const;

  [[nodiscard]] const entity* get(const thread_id& y) const;

private:
  caf::node_id node_;
};
} // namespace vec
