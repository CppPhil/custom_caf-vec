#include "node_cmp.hpp"

namespace vec {
bool node_cmp_t::operator()(const entity& x, const caf::node_id& y) const {
  return x.nid < y;
}

bool node_cmp_t::operator()(const caf::node_id& x, const entity& y) const {
  return x < y.nid;
}
} // namespace vec
