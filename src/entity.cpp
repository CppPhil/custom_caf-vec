#include "entity.hpp"

namespace vec {
mailbox_id to_mailbox_id(const entity& x) {
  if (x.aid == 0)
    CAF_RAISE_ERROR("threads do not have a mailbox ID");

  return {x.aid, x.nid};
}

logger_id to_logger_id(const entity& x) {
  return {x.aid, x.tid};
}

bool operator<(const entity& x, const entity& y) {
  // We sort by node ID first.
  auto comparison_result = x.nid.compare(y.nid);

  if (comparison_result != 0)
    return comparison_result < 0;

  return (x.aid == 0 && y.aid == 0) ? x.tid < y.tid : x.aid < y.aid;
}
} // namespace vec
