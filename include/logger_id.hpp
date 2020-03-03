#pragma once
#include <iosfwd>

#include <caf/all.hpp>

namespace vec {
/// The ID of entities as used in a logfile. If the logger field is "actor0"
/// then this line represents a thread. Otherwise, the thread field is ignored.
struct logger_id {
  /// Content of the [LOGGER] field (0 if logger is a thread).
  caf::actor_id aid;
  /// Content of the [THREAD] field.
  std::string tid;
};

bool operator<(const logger_id& x, const logger_id& y);

std::istream& operator>>(std::istream& in, logger_id& x);
} // namespace vec
