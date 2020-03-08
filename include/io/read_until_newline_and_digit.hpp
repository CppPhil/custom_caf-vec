#pragma once
#include <cctype>

#include <string>

#include "read_until.hpp"

namespace vec::io {
inline auto read_until_newline_and_digit(std::string& buffer) {
  return read_until([has_seen_new_line = false](auto c) mutable {
    if (!has_seen_new_line) {
      has_seen_new_line = c == '\n';
      return false;
    }

    return has_seen_new_line && isdigit(static_cast<unsigned char>(c)) != 0;
  })(buffer);
}
} // namespace vec::io
