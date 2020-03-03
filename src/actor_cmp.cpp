#include "actor_cmp.hpp"

namespace vec {bool actor_cmp_t::operator()(const entity& x, caf::actor_id y) const {
  return x.aid < y;
}

bool actor_cmp_t::operator()(caf::actor_id x, const entity& y) const {
  return x < y.aid;
}
} // namespace vec
