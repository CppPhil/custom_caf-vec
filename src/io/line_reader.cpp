#include <istream>

#include "io/line_reader.hpp"
#include "trim.hpp"

namespace vec::io {
std::istream& operator>>(std::istream& in, line_reader x) {
  std::getline(in, x.line, x.delim);
  trim(x.line);
  return in;
}

line_reader rd_line(std::string& line, char delim) {
  return {line, delim};
}
} // namespace vec::io
