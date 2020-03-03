#pragma once
#include "vector_timestamp.hpp"

#include <iosfwd>
#include <string>

namespace vec {
struct log_entry;
struct entity;

/// Stores a log event along with context information.
struct enhanced_log_entry {
  /// The original log entry without context information.
  const log_entry& data;
  /// The actual ID of the logging entity.
  const entity& id;
  /// Current vector time as seen by `id`.
  vector_timestamp& vstamp;
  /// JSON representation of `vstamp`.
  std::string json_vstamp;
};

std::ostream& operator<<(std::ostream& out, const enhanced_log_entry& x);
} // namespace vec
