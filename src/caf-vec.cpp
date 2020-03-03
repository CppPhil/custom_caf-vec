#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "caf/all.hpp"

#include "actor_cmp.hpp"
#include "enhanced_log_entry.hpp"
#include "entity.hpp"
#include "entity_set.hpp"
#include "entity_set_range.hpp"
#include "get.hpp"
#include "io/istream_char_consumer.hpp"
#include "io/line_reader.hpp"
#include "io/skip_to_next_line.hpp"
#include "io/skip_whitespaces.hpp"
#include "io/skip_word.hpp"
#include "log_entry.hpp"
#include "log_level.hpp"
#include "logger_id.hpp"
#include "mailbox_id.hpp"
#include "merge.hpp"
#include "node_cmp.hpp"
#include "node_id.hpp"
#include "node_range.hpp"
#include "se_event.hpp"
#include "se_type.hpp"
#include "thread_id.hpp"
#include "thread_range.hpp"
#include "trim.hpp"
#include "vector_timestamp.hpp"

bool field_key_compare(const std::pair<const std::string, std::string>& x,
                       const std::string& y) {
  return x.first == y;
}

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
  se_event y{&x.id, x.vstamp, se_type::none,
             std::map<std::string, std::string>{}};
  std::istringstream in{x.data.message};
  std::string type;
  if (!(in >> type))
    return caf::sec::invalid_argument;
  std::string field_name;
  std::string field_content;
  in >> consume(";");
  while (in >> field_name >> consume("=") >> rd_line(field_content, ';'))
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

struct logger_id_meta_data {
  bool hidden;
  std::string pretty_name;
};

/// Stores all log entities and their node ID.
struct first_pass_result {
  /// Node ID used in the parsed file.
  caf::node_id this_node;
  /// Entities of the parsed file. The value is `true` if an entity is
  /// hidden, otherwise `false`.
  std::map<logger_id, logger_id_meta_data> entities;
};

enum verbosity_level { silent, informative, noisy };

caf::expected<first_pass_result>
first_pass(caf::blocking_actor* self, std::istream& in, verbosity_level vl) {
  first_pass_result res;
  // read first line to extract the node ID of local actors
  // _ caf INFO actor0 _ caf.logger start _:_ level = _, node = NODE
  if (!(in >> skip_word >> consume("caf") >> consume("DEBUG")
        >> consume("actor0") >> skip_word >> consume("caf.logger")
        >> consume("log_first_line") >> skip_word >> consume("level =")
        >> skip_word >> consume("node = ") >> res.this_node
        >> skip_to_next_line)) {
    std::cerr << "*** malformed log file, expect the first line to contain "
              << "an INFO entry of the logger" << std::endl;
    return caf::sec::invalid_argument;
  }
  if (vl >= verbosity_level::informative)
    aout(self) << "found node " << res.this_node << std::endl;
  logger_id id;
  std::string message;
  while (in >> skip_word >> skip_word >> skip_word >> id >> skip_word
         >> skip_word >> skip_word >> rd_line(message)) {
    // store in map
    auto i
      = res.entities.emplace(id, logger_id_meta_data{false, "actor"}).first;
    if (caf::starts_with(message, "INIT ; NAME = ")) {
      std::istringstream iss{message};
      iss >> consume("INIT ; NAME = ") >> rd_line(i->second.pretty_name, ';');
      if (caf::ends_with(message, "HIDDEN = true"))
        i->second.hidden = true;
    }
  }
  if (vl >= verbosity_level::informative)
    aout(self) << "found " << res.entities.size() << " entities for node "
               << res.this_node << std::endl;
  return res;
}

const std::string& get(const std::map<std::string, std::string>& xs,
                       const std::string& x) {
  auto i = xs.find(x);
  if (i != xs.end())
    return i->second;
  CAF_RAISE_ERROR("key not found");
}

void second_pass(caf::blocking_actor* self, const caf::group& grp,
                 const entity_set& entities, const caf::node_id& nid,
                 const std::vector<std::string>& json_names, std::istream& in,
                 std::ostream& out, std::mutex& out_mtx,
                 bool drop_hidden_actors, verbosity_level vl) {
  assert(entities.size() == json_names.size());
  node_range local_entities{entities, nid};
  if (local_entities.begin() == local_entities.end())
    return;
  // state for each local entity
  struct state_t {
    const entity& eid;
    vector_timestamp clock;
  };
  std::map<logger_id, state_t> local_entities_state;
  for (auto& x : local_entities) {
    vector_timestamp vzero;
    vzero.resize(entities.size());
    local_entities_state.emplace(logger_id{x.aid, x.tid},
                                 state_t{x, std::move(vzero)});
  }
  // lambda for accessing state via logger ID
  auto state = [&](const logger_id& x) -> caf::optional<state_t&> {
    auto i = local_entities_state.find(x);

    if (i == local_entities_state.end()) {
      if (x.aid == 0)
        return caf::none;

      CAF_RAISE_ERROR("logger ID not found");
    } else
      return i->second;
  };
  // additional state for second pass
  size_t line = 0;
  log_entry plain_entry;
  std::vector<se_event> in_flight_messages;
  std::vector<se_event> in_flight_spawns;
  // maps scoped actor IDs to their parent ID
  std::map<logger_id, logger_id> scoped_actors;
  // lambda for broadcasting events that could cross node boundary
  auto bcast = [&](const se_event& x) {
    if (vl >= verbosity_level::noisy)
      aout(self) << "broadcast event from " << nid << ": "
                 << caf::deep_to_string(x) << std::endl;
    if (self != nullptr)
      self->send(grp, x);
  };
  // fetch message from another node via the group
  auto fetch_message
    = [&](const std::map<std::string, std::string>& fields) -> se_event& {
    // TODO: this receive unconditionally waits on a message,
    //       i.e., is a potential deadlock
    if (vl >= verbosity_level::noisy)
      aout(self) << "wait for send from another node matching fields "
                 << caf::deep_to_string(fields) << std::endl;
    se_event* res = nullptr;
    self->receive_while([&] { return res == nullptr; })([&](const se_event& x) {
      switch (x.type) {
        default:
          break;
        case se_type::send:
          in_flight_messages.emplace_back(x);
          if (x.fields == fields)
            res = &in_flight_messages.back();
          break;
      }
    });
    return *res;
  };
  // second pass
  while (in >> plain_entry) {
    ++line;
    // increment local time
    auto opt = state(plain_entry.id);

    if (!opt)
      continue;

    auto& st = *opt;

    // do not produce log output for internal actors but still track messages
    // through those actors, because they might be forwarding messages
    bool internal = drop_hidden_actors && st.eid.hidden;
    if (!internal)
      st.clock[st.eid.vid] += 1;
    // generate enhanced entry (with incomplete JSON timestamp for now)
    enhanced_log_entry entry{plain_entry, st.eid, st.clock, std::string{}};
    // check whether entry contains an SE-0001 event
    auto tmp = parse_event(entry);
    if (tmp) {
      auto& event = *tmp;
      switch (event.type) {
        default:
          break;
        case se_type::send:
          bcast(event);
          in_flight_messages.emplace_back(std::move(event));
          break;
        case se_type::receive: {
          auto pred = [&](const se_event& x) {
            assert(x.type == se_type::send);
            return event.fields == x.fields;
          };
          auto e = in_flight_messages.end();
          auto i = std::find_if(in_flight_messages.begin(), e, pred);
          if (i != e) {
            merge(st.clock, i->vstamp);
          } else {
            merge(st.clock, fetch_message(event.fields).vstamp);
          }
          break;
        }
        case se_type::spawn:
          in_flight_spawns.emplace_back(std::move(event));
          break;
        case se_type::init: {
          auto id_field = std::to_string(st.eid.aid);
          auto pred = [&](const se_event& x) {
            assert(x.type == se_type::spawn);
            return get(x.fields, "ID") == id_field;
          };
          auto e = in_flight_spawns.end();
          auto i = std::find_if(in_flight_spawns.begin(), e, pred);
          if (i != e) {
            merge(st.clock, i->vstamp);
            // keep book on scoped actors since their terminate
            // event propagates back to the parent
            if (get(event.fields, "NAME") == "scoped_actor")
              scoped_actors.emplace(plain_entry.id, to_logger_id(*i->source));
            in_flight_spawns.erase(i);
          } else {
            std::cerr << "*** cannot match init event to a previous spawn"
                      << std::endl;
          }
          break;
        }
        case se_type::terminate:
          auto i = scoped_actors.find(plain_entry.id);
          if (i != scoped_actors.end()) {
            // merge timestamp with parent to capture happens-before relation
            auto parent_state_opt = state(i->second);

            if (!parent_state_opt)
              continue;

            auto& parent_state = *parent_state_opt;

            merge(parent_state.clock, st.clock);
            scoped_actors.erase(i);
          }
          break;
      }
    }
    // create ShiViz compatible JSON-formatted vector timestamp
    std::ostringstream oss;
    oss << '{';
    bool need_comma = false;
    for (size_t i = 0; i < st.clock.size(); ++i) {
      auto x = st.clock[i];
      if (x > 0) {
        if (need_comma)
          oss << ',';
        else
          need_comma = true;
        oss << '"' << json_names[i] << '"' << ':' << x;
      }
    }
    oss << '}';
    entry.json_vstamp = oss.str();
    // print entry to output file
    if (!internal) {
      std::lock_guard<std::mutex> guard{out_mtx};
      out << entry << '\n';
    }
  }
}

namespace {

struct config : public caf::actor_system_config {
  std::string output_file;
  bool include_hidden_actors = false;
  size_t verbosity = 0;
  config() {
    opt_group{custom_options_, "global"}
      .add(output_file, "output-file,o", "Path for the output file")
      .add(include_hidden_actors, "include-hidden-actors,i",
           "Include hidden (system-level) actors")
      .add(verbosity, "verbosity,v", "Debug output (from 0 to 2)");
    // shutdown logging per default
    set("logger.verbosity", "quiet");
  }
};

// two pass parser for CAF log files that enhances logs with vector
// clock timestamps
void caf_main(caf::actor_system& sys, const config& cfg) {
  if (cfg.output_file.empty()) {
    std::cerr << "*** no output file specified" << std::endl;
    return;
  }
  verbosity_level vl;
  switch (cfg.verbosity) {
    case 0:
      vl = silent;
      break;
    case 1:
      vl = verbosity_level::informative;
      break;
    default:
      vl = verbosity_level::noisy;
  }
  // open output file
  std::ofstream out{cfg.output_file};
  if (!out) {
    std::cerr << "unable to open output file: " << cfg.output_file << std::endl;
    return;
  }

  static constexpr size_t irsize = sizeof(std::string) + sizeof(std::ifstream)
                                   + sizeof(first_pass_result);

  struct intermediate_res {
    std::string file_path;
    std::unique_ptr<std::ifstream> fstream;
    first_pass_result res;
    char pad[irsize >= CAF_CACHE_LINE_SIZE ? 1 : CAF_CACHE_LINE_SIZE - irsize];
    intermediate_res() = default;
    intermediate_res(intermediate_res&&) = default;
    intermediate_res& operator=(intermediate_res&&) = default;
    intermediate_res(std::string file_path, std::unique_ptr<std::ifstream> fs,
                     first_pass_result&& fr)
      : file_path(std::move(file_path)),
        fstream(std::move(fs)),
        res(std::move(fr)) {
      // nop
    }
  };
  // do a first pass on all files to extract node IDs and entities
  std::vector<intermediate_res> intermediate_results;
  intermediate_results.resize(cfg.remainder.size());
  for (size_t i = 0; i < cfg.remainder.size(); ++i) {
    auto& file = cfg.remainder[i];
    auto ptr = &intermediate_results[i];
    ptr->file_path = file;
    ptr->fstream = std::make_unique<std::ifstream>(file);
    if (!*ptr->fstream) {
      std::cerr << "could not open file: " << file << std::endl;
      continue;
    }
    sys.spawn([ptr, vl](caf::blocking_actor* self) {
      auto& f = *ptr->fstream;
      auto res = first_pass(self, f, vl);
      if (res) {
        // rewind stream and push intermediate results
        f.clear();
        f.seekg(0);
        ptr->res = std::move(*res);
      }
    });
  }
  sys.await_all_actors_done();
  // post-process collected entity IDs before second pass
  entity_set entities;
  std::vector<std::string> entity_names;
  auto sort_pred = [](const intermediate_res& x, const intermediate_res& y) {
    return x.res.this_node < y.res.this_node;
  };
  std::map<std::string, size_t> pretty_actor_names;
  size_t thread_count = 0;
  // make sure we insert in sorted order into the entities set
  std::sort(intermediate_results.begin(), intermediate_results.end(),
            sort_pred);
  for (auto& ir : intermediate_results) {
    auto node_as_string = caf::to_string(ir.res.this_node);
    for (auto& kvp : ir.res.entities) {
      std::string pretty_name;
      // make each (pretty) actor and thread name unique
      auto& pn = kvp.second.pretty_name;
      if (kvp.first.aid != 0)
        pretty_name = pn + std::to_string(++pretty_actor_names[pn]);
      //"actor" + std::to_string(kvp.first.aid);
      else
        pretty_name = "thread" + std::to_string(++thread_count);
      auto vid = entities.size(); // position in the vector timestamp
      entity_names.emplace_back(pretty_name);
      entities.emplace(entity{kvp.first.aid, kvp.first.tid, ir.res.this_node,
                              vid, kvp.second.hidden, std::move(pretty_name)});
    }
  }
  // check whether entities set is in the right order
  auto vid_cmp = [](const entity& x, const entity& y) { return x.vid < y.vid; };
  if (!std::is_sorted(entities.begin(), entities.end(), vid_cmp)) {
    std::cerr << "*** ERROR: entity set not sorted by vector timestamp ID:\n"
              << caf::deep_to_string(entities) << std::endl;
    return;
  }
  // do a second pass for all log files
  // first line is the regex to parse the remainder of the file
  out << R"((?<clock>\S+) (?<timestamp>\d+) (?<component>\S+) )"
      << R"((?<level>\S+) (?<host>\S+) (?<class>\S+) (?<function>\S+) )"
      << R"((?<file>\S+):(?<line>\d+) (?<event>.+))" << std::endl;
  // second line is the separator for multiple runs
  out << std::endl;
  std::mutex out_mtx;
  auto grp = sys.groups().anonymous();
  for (auto& fpr : intermediate_results) {
    sys.spawn_in_group(grp, [&](caf::blocking_actor* self) {
      try {
        second_pass(self, grp, entities, fpr.res.this_node, entity_names,
                    *fpr.fstream, out, out_mtx, !cfg.include_hidden_actors, vl);
      } catch (const std::runtime_error& ex) {
        fprintf(stderr, "Caught std::runtime_error: \"%s\"\n", ex.what());
      }
    });
  }
  sys.await_all_actors_done();
}
} // namespace

CAF_MAIN()
