#pragma once
#include <string>

namespace vec {
struct logger_id_meta_data {
  bool hidden;
  std::string pretty_name;

  template <class OutputStream>
  friend OutputStream&
  operator<<(OutputStream& os, const logger_id_meta_data& meta_data) {
    return os << "logger_id_meta_data{hidden: "
              << "false\0true" + meta_data.hidden * 6
              << ", pretty_name: " << meta_data.pretty_name << '}';
  }
};
} // namespace vec
