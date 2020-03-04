#include <algorithm>
#include <ostream>

#include "log_level.hpp"

namespace vec {
constexpr const char* log_level_name[] = {"ERROR", "WARN",  "INFO",
                                          "DEBUG", "TRACE", "?????"};

std::ostream& operator<<(std::ostream& out, const log_level& lvl) {
  return out << log_level_name[static_cast<size_t>(lvl)];
}

std::istream& operator>>(std::istream& in, log_level& lvl) {
  std::string tmp;
  in >> tmp;

  auto pred = [&](const char* cstr) { return cstr == tmp; };
  auto begin = std::begin(log_level_name);
  auto end = std::end(log_level_name);
  auto it = std::find_if(begin, end, pred);

  if (it == end)
    lvl = log_level::invalid;
  else
    lvl = static_cast<log_level>(std::distance(begin, it));

  return in;
}
} // namespace vec
