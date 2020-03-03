#pragma once
#include <iosfwd>
#include <string>

struct line_reader {
  std::string& line;
  char delim;
};

std::istream& operator>>(std::istream& in, line_reader x);

line_reader rd_line(std::string& line, char delim = '\n');
