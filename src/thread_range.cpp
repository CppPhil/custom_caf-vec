#include <algorithm>

#include "actor_cmp.hpp"
#include "thread_range.hpp"

namespace vec {
thread_range::thread_range(const node_range& xs) {
  caf::actor_id dummy = 0;
  // get range for the node
  begin_ = xs.begin();
  end_ = std::upper_bound(begin_, xs.end(), dummy, actor_cmp);
}
} // namespace vec
