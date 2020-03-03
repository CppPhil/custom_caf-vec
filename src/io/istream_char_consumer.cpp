#include <istream>

#include "io/istream_char_consumer.hpp"
#include "io/skip_whitespaces.hpp"

namespace vec::io {
std::istream& operator>>(std::istream& in, istream_char_consumer x) {
  if (!in)
    return in;
  // ignore leading whitespaces
  skip_whitespaces(in);
  // ignore trailing '\0'
  for (size_t i = 0; i < x.count; ++i) {
    // cout << "in: " << (char) in.peek() << ", x: " << x.what[i] << endl;
    if (in.get() != x.what[i]) {
      in.setstate(std::ios::failbit);
      break;
    }
  }
  return in;
}
} // namespace vec::io
