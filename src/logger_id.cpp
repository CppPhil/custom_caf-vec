#include <istream>

#include "io/istream_char_consumer.hpp"
#include "io/skip_whitespaces.hpp"
#include "logger_id.hpp"

bool operator<(const logger_id& x, const logger_id& y) {
  return x.aid == 0 && y.aid == 0 ? x.tid < y.tid : x.aid < y.aid;
}

std::istream& operator>>(std::istream& in, logger_id& x) {
  return in >> consume("actor") >> x.aid >> skip_whitespaces >> x.tid;
}
