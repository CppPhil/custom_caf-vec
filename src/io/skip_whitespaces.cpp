#include <istream>

namespace vec::io {
std::istream& skip_whitespaces(std::istream& in) {
  while (in.peek() == ' ')
    in.get();

  return in;
}
} // namespace vec::io
