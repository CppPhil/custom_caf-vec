#include <algorithm>

#include "actor_cmp.hpp"
#include "node_cmp.hpp"
#include "node_range.hpp"
#include "thread_range.hpp"

namespace vec {
node_range::node_range(const entity_set& xs, const caf::node_id& y) {
  // get range for the node
  begin_ = std::lower_bound(xs.begin(), xs.end(), y, node_cmp);
  end_ = std::upper_bound(begin_, xs.end(), y, node_cmp);
}

node_range::node_range(const node_range&) = default;

node_range& node_range::operator=(const node_range&) = default;

const caf::node_id& node_range::node() const {
  return node_;
}

const entity* node_range::get(const thread_id& y) const {
  thread_range subrange{*this};
  return subrange.get(y);
}

const entity* node_range::get(caf::actor_id y) const {
  if (y == 0)
    return nullptr;
  // range [xs.first, xs.second) is sorted by actor ID
  auto i = std::lower_bound(begin(), end(), y, actor_cmp);
  if (i->aid == y)
    return &(*i);
  return nullptr;
}

const entity* node_range::get(const logger_id& y) const {
  return y.aid > 0 ? get(y.aid) : get(y.tid);
}
} // namespace vec
