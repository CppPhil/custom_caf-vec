#pragma once
#include <iosfwd>
#include <string>

namespace vec::io {
struct line_reader {
  std::string& line;
  char delim;
};

std::istream& operator>>(std::istream& in, line_reader x);

line_reader read_line(std::string& line, char delim = '\n');
} // namespace vec::io
