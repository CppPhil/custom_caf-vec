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
} // namespace vec
