#pragma once
#include <string>

namespace vec {
/// CAF events according to SE-0001.
enum class se_type {
  spawn,
  init,
  send,
  reject,
  receive,
  drop,
  skip,
  finalize,
  terminate,
  none
};

std::string to_string(se_type x);
} // namespace vec
