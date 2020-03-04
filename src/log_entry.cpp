#include "log_entry.hpp"
#include "io/istream_char_consumer.hpp"
#include "io/line_reader.hpp"
#include "io/skip_whitespaces.hpp"

namespace vec {
std::istream& operator>>(std::istream& in, log_entry& x) {
  in >> x.timestamp >> x.component >> x.level >> io::consume("actor")
    >> x.id.aid >> x.id.tid >> x.class_name >> x.function_name
    >> io::skip_whitespaces >> io::read_line(x.file_name, ':') >> x.line_number
    >> io::skip_whitespaces >> io::read_line(x.message);
  if (x.level == log_level::invalid)
    in.setstate(std::ios::failbit);
  return in;
}
} // namespace vec
