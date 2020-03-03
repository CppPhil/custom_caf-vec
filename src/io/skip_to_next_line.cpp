#include <istream>

namespace vec::io {
std::istream& skip_to_next_line(std::istream& in) {
  in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  return in;
}
} // namespace vec::io
