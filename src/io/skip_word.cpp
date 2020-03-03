#include <istream>

#include "io/skip_whitespaces.hpp"

namespace vec::io {
std::istream& skip_word(std::istream& in) {
  skip_whitespaces(in);

  auto nonspace = [](char x) { return (isprint(x) != 0) && (isspace(x) == 0); };

  while (nonspace(static_cast<char>(in.peek())))
    in.get();

  return in;
}
} // namespace vec::io
