#include <algorithm>

#include "actor_cmp.hpp"
#include "entity.hpp"
#include "get.hpp"
#include "node_range.hpp"
#include "thread_range.hpp"

const entity* get(const thread_range& xs, const thread_id& y) {
  // only compares thread ID
  auto thread_cmp = [](const entity& lhs, thread_id rhs) {
    return lhs.tid < rhs;
  };
  // range [xs.first, xs.second) is sortd by thread ID
  auto i = std::lower_bound(xs.begin(), xs.end(), y, thread_cmp);
  if (i->tid == y)
    return &(*i);
  return nullptr;
}

const entity* get(const node_range& xs, const thread_id& y) {
  thread_range subrange{xs};
  return get(subrange, y);
}

/// Returns the entity for `y` from the node range `xs`.
const entity* get(const node_range& xs, caf::actor_id y) {
  if (y == 0)
    return nullptr;
  // range [xs.first, xs.second) is sortd by actor ID
  auto i = std::lower_bound(xs.begin(), xs.end(), y, actor_cmp);
  if (i->aid == y)
    return &(*i);
  return nullptr;
}

const entity* get(const node_range& xs, const logger_id& y) {
  return y.aid > 0 ? get(xs, y.aid) : get(xs, y.tid);
}
