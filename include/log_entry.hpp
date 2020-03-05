#pragma once
#include <cstdint>

#include <iosfwd>
#include <string>

#include "log_level.hpp"
#include "logger_id.hpp"

namespace vec {
/// A single entry in a logfile.
struct log_entry {
  /// A UNIX timestamp.
  int64_t timestamp;
  /// Identifies the logging component, e.g., "caf".
  std::string component;
  /// Severity level of this entry.
  log_level level;
  /// ID of the logging entity.
  logger_id id;
  /// Context information about currently active class.
  std::string class_name;
  /// Context information about currently executed function.
  std::string function_name;
  /// Context information about currently executed source file.
  std::string file_name;
  /// Context information about currently executed source line.
  int32_t line_number;
  /// Description of the log entry.
  std::string message;
};

template <class OutputStream>
OutputStream& operator<<(OutputStream& os, const log_entry& entry) {
  return os << "log_entry{timestamp: " << entry.timestamp
            << ", component: " << entry.component << ", level: " << entry.level
            << ", id: " << entry.id << ", class_name: " << entry.class_name
            << ", function_name: " << entry.function_name
            << ", file_name: " << entry.file_name
            << ", line_number: " << entry.line_number << ", message: \""
            << entry.message << "\"}";
}

std::istream& operator>>(std::istream& in, log_entry& x);
} // namespace vec
