#pragma once

#include <iosfwd>
#include <string>
#include <vector>

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
  std::vector<size_t>& vector_timestamp;
  /// JSON representation of `vector_timestamp`.
  std::string json_vector_timestamp;
};

std::ostream& operator<<(std::ostream& out, const enhanced_log_entry& x);
} // namespace vec
