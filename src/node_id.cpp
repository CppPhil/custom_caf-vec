#include "node_id.hpp"
#include "io/istream_char_consumer.hpp"
#include "io/line_reader.hpp"
#include "io/skip_whitespaces.hpp"

namespace caf {
std::istream& operator>>(std::istream& in, caf::node_id& x) {
  in >> vec::io::skip_whitespaces;
  if (in.peek() == 'i') {
    x = caf::node_id{};
    return in >> vec::io::consume("invalid-node");
  }
  std::string node_hex_id;
  uint32_t pid;
  if (in >> vec::io::rd_line(node_hex_id, '#') >> pid) {
    if (auto nid = caf::make_node_id(pid, node_hex_id))
      x = std::move(*nid);
    else
      in.setstate(std::ios::failbit);
  }
  return in;
}
} // namespace caf
