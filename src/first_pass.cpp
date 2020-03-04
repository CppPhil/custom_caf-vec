#include "first_pass.hpp"
#include "io/istream_char_consumer.hpp"
#include "io/line_reader.hpp"
#include "io/skip_to_next_line.hpp"
#include "io/skip_word.hpp"
#include "node_id.hpp"

namespace vec {
caf::expected<first_pass_result>
first_pass(caf::blocking_actor* self, std::istream& in, verbosity_level vl) {
  first_pass_result res;
  // read first line to extract the node ID of local actors
  // _ caf INFO actor0 _ caf.logger start _:_ level = _, node = NODE
  if (!(in >> io::skip_word >> io::consume("caf") >> io::consume("DEBUG")
        >> io::consume("actor0") >> io::skip_word >> io::consume("caf.logger")
        >> io::consume("log_first_line") >> io::skip_word
        >> io::consume("level =") >> io::skip_word >> io::consume("node = ")
        >> res.this_node >> io::skip_to_next_line)) {
    std::cerr << "*** malformed log file, expect the first line to contain "
              << "an INFO entry of the logger" << std::endl;
    return caf::sec::invalid_argument;
  }
  if (vl >= verbosity_level::informative)
    aout(self) << "found node " << res.this_node << std::endl;
  logger_id id;
  std::string message;
  while (in >> io::skip_word >> io::skip_word >> io::skip_word >> id
         >> io::skip_word >> io::skip_word >> io::skip_word
         >> io::read_line(message)) {
    // store in map
    auto i
      = res.entities.emplace(id, logger_id_meta_data{false, "actor"}).first;
    if (caf::starts_with(message, "INIT ; NAME = ")) {
      std::istringstream iss{message};
      iss >> io::consume("INIT ; NAME = ")
        >> io::read_line(i->second.pretty_name, ';');
      if (caf::ends_with(message, "HIDDEN = true"))
        i->second.hidden = true;
    }
  }
  if (vl >= verbosity_level::informative)
    aout(self) << "found " << res.entities.size() << " entities for node "
               << res.this_node << std::endl;
  return res;
}
} // namespace vec
