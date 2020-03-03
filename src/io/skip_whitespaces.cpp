#include <istream>

std::istream& skip_whitespaces(std::istream& in) {
  while (in.peek() == ' ')
    in.get();

  return in;
}
