#pragma once
#include <cctype>

#include <string>

#include "read_until.hpp"

namespace vec::io {
namespace {
auto read_until_decimal_digit(std::string& buffer) {
  return read_until([](auto c) { return isdigit(c) != 0; })(buffer);
}
} // namespace
} // namespace vec::io
