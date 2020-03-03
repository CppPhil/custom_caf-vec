#include <istream>
#include <ostream>

#include "io/istream_char_consumer.hpp"
#include "mailbox_id.hpp"
#include "node_id.hpp"

namespace vec {
std::string to_string(const mailbox_id& x) {
  auto res = std::to_string(x.aid);
  res += '@';
  res += to_string(x.nid);
  return res;
}

std::istream& operator>>(std::istream& in, mailbox_id& x) {
  // format is <actor>@<node>
  return in >> x.aid >> io::consume("@") >> x.nid;
}

std::ostream& operator<<(std::ostream& out, const mailbox_id& x) {
  return out << x.aid << '@' << to_string(x.nid);
}
} // namespace vec
