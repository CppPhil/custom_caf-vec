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
  auto b = std::begin(log_level_name);
  auto e = std::end(log_level_name);
  auto i = std::find_if(b, e, pred);
  if (i == e)
    lvl = log_level::invalid;
  else
    lvl = static_cast<log_level>(std::distance(b, i));
  return in;
}
} // namespace vec
