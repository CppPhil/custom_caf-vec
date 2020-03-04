#include <algorithm>

#include "actor_cmp.hpp"
#include "entity.hpp"
#include "thread_range.hpp"

namespace vec {
thread_range::thread_range(const node_range& xs) : node_(xs.node()) {
  caf::actor_id dummy = 0;
  // get range for the node
  begin_ = xs.begin();
  end_ = std::upper_bound(begin_, xs.end(), dummy, actor_cmp);
}

thread_range::thread_range(const thread_range&) = default;

thread_range& thread_range::operator=(const thread_range&) = default;

const caf::node_id& thread_range::node() const {
  return node_;
}

const entity* thread_range::get(const thread_id& y) const {
  // only compares thread ID
  auto thread_cmp = [](const entity& lhs, thread_id rhs) {
    return lhs.tid < rhs;
  };
  // range [xs.first, xs.second) is sortd by thread ID
  auto i = std::lower_bound(begin(), end(), y, thread_cmp);
  if (i->tid == y)
    return &(*i);
  return nullptr;
}
} // namespace vec
