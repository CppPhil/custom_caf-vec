#pragma once
#include <string>

#include <caf/all.hpp>

#include "logger_id.hpp"
#include "mailbox_id.hpp"
#include "thread_id.hpp"

namespace vec {
/// An entity in our distributed system, i.e., either an actor or a thread.
struct entity {
  /// The ID of this entity if it is an actor, otherwise 0.
  caf::actor_id aid;
  /// The ID of this entity if it is a thread, otherwise empty.
  thread_id tid;
  /// The ID of the node this entity is running at.
  caf::node_id nid;
  /// The ID of this node in the vector clock.
  size_t vid;
  /// Marks system-level actors to enable filtering.
  bool hidden;
  /// A human-redable name, e.g., "actor42" or "thread23".
  std::string pretty_name;
};

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, entity& x) {
  return f(caf::meta::type_name("entity"), x.aid, x.tid, x.nid, x.vid, x.hidden,
           x.pretty_name);
}

mailbox_id to_mailbox_id(const entity& x);

logger_id to_logger_id(const entity& x);

/// Sorts entities by `nid` first, then places threads before actors
/// and finally compares `aid` or `tid`.
bool operator<(const entity& x, const entity& y);
} // namespace vec
