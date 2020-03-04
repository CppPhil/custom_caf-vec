#include <algorithm>
#include <set>
#include <string>

#include <caf/all.hpp>

#include "enhanced_log_entry.hpp"
#include "entity.hpp"
#include "field_key_compare.hpp"
#include "io/istream_char_consumer.hpp"
#include "io/line_reader.hpp"
#include "log_entry.hpp"
#include "parse_event.hpp"

namespace vec {
#define CHECK_FIELDS(...)                                                      \
  {                                                                            \
    std::set<std::string> keys{__VA_ARGS__};                                   \
    if (y.fields.size() != keys.size())                                        \
      return caf::sec::invalid_argument;                                       \
    if (!std::equal(y.fields.begin(), y.fields.end(), keys.begin(),            \
                    field_key_compare))                                        \
      return caf::sec::invalid_argument;                                       \
  }                                                                            \
  static_cast<void>(0)

#define CHECK_NO_FIELDS()                                                      \
  if (!y.fields.empty())                                                       \
    return caf::sec::invalid_argument;

caf::expected<se_event> parse_event(const enhanced_log_entry& x) {
  se_event y{&x.id, x.vector_timestamp, se_type::none,
             std::map<std::string, std::string>{}};
  std::istringstream in{x.data.message};
  std::string type;
  if (!(in >> type))
    return caf::sec::invalid_argument;
  std::string field_name;
  std::string field_content;
  in >> io::consume(";");
  while (in >> field_name >> io::consume("=")
         >> io::read_line(field_content, ';'))
    y.fields.emplace(std::move(field_name), std::move(field_content));
  if (type == "SPAWN") {
    y.type = se_type::spawn;
    CHECK_FIELDS("ID", "ARGS");
  } else if (type == "INIT") {
    y.type = se_type::init;
    CHECK_FIELDS("NAME", "HIDDEN");
  } else if (type == "SEND") {
    y.type = se_type::send;
    CHECK_FIELDS("TO", "FROM", "STAGES", "CONTENT");
  } else if (type == "REJECT") {
    y.type = se_type::reject;
    CHECK_NO_FIELDS();
  } else if (type == "RECEIVE") {
    y.type = se_type::receive;
    CHECK_FIELDS("FROM", "STAGES", "CONTENT");
    // insert TO field to allow comparing SEND and RECEIVE events easily
    y.fields.emplace("TO", to_string(to_mailbox_id(x.id)));
  } else if (type == "DROP") {
    y.type = se_type::drop;
    CHECK_NO_FIELDS();
  } else if (type == "SKIP") {
    y.type = se_type::skip;
    CHECK_NO_FIELDS();

  } else if (type == "FINALIZE") {
    y.type = se_type::finalize;
    CHECK_NO_FIELDS();
  } else if (type == "TERMINATE") {
    y.type = se_type::terminate;
    CHECK_FIELDS("REASON");
  }
  return {std::move(y)};
}
} // namespace vec
