#pragma once
#include <iosfwd>

struct istream_char_consumer {
  const char* what;
  size_t count;
};

std::istream& operator>>(std::istream& in, istream_char_consumer x);

template <size_t S>
istream_char_consumer consume(const char (&what)[S]) {
  return {what, S - 1};
}
