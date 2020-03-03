#include <algorithm>

#include "actor_cmp.hpp"
#include "thread_range.hpp"

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
