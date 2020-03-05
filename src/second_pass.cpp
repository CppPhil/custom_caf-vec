#include <cassert>

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include "enhanced_log_entry.hpp"
#include "log_entry.hpp"
#include "merge.hpp"
#include "node_range.hpp"
#include "parse_event.hpp"
#include "se_event.hpp"
#include "second_pass.hpp"

namespace vec {
namespace {
const std::string& get(const std::map<std::string, std::string>& xs,
                       const std::string& x) {
  auto i = xs.find(x);

  if (i != xs.end())
    return i->second;

  CAF_RAISE_ERROR("key not found");
}

// state for each local entity
struct state_t {
  const entity& entity_id;
  std::vector<size_t> vector_timestamp;
};

template <class OutputStream>
OutputStream& operator<<(OutputStream& os, const state_t& state) {
  return os << "state_t{entity_id: " << state.entity_id
            << ", vector_timestamp: " << caf::join(state.vector_timestamp, ", ")
            << '}';
}
} // namespace

void second_pass(caf::blocking_actor* self, const caf::group& grp,
                 const entity_set& entities, const caf::node_id& nid,
                 const std::vector<std::string>& json_names, std::istream& in,
                 std::ostream& out, std::mutex& out_mtx,
                 bool drop_hidden_actors, verbosity_level vl) {
  assert(entities.size() == json_names.size());

  SPDLOG_INFO("entities: {}", entities);

  node_range local_entities{entities, nid};

  SPDLOG_INFO("local_entities: {}", local_entities);

  if (local_entities.begin() == local_entities.end())
    return;

  std::map<logger_id, state_t> local_entities_state;

  for (auto& x : local_entities) {
    std::vector<size_t> zero_vector_timestamp;
    zero_vector_timestamp.resize(entities.size());
    local_entities_state.emplace(logger_id{x.aid, x.tid},
                                 state_t{x, std::move(zero_vector_timestamp)});
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
  log_entry plain_entry = {};
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
    SPDLOG_INFO("Read: {}", plain_entry);

    ++line;
    // increment local time
    auto opt = state(plain_entry.id);

    if (!opt) {
      SPDLOG_WARN("Couldn't get state for {}, skipping.", plain_entry);
      continue;
    }

    auto& current_state = *opt;

    SPDLOG_INFO("Got state for \"{}\": \"{}\"", plain_entry, current_state);

    // do not produce log output for internal actors but still track messages
    // through those actors, because they might be forwarding messages
    bool internal = drop_hidden_actors && current_state.entity_id.hidden;

    if (!internal) {
      current_state.vector_timestamp[current_state.entity_id.vid] += 1;
      SPDLOG_INFO("Incremented vector timestamp for \"{}\": \"{}\"",
                  plain_entry, current_state);
    } else
      SPDLOG_INFO(
        "Didn't increment vector timestamp for \"{}\", it was \"internal\"",
        plain_entry);

    // generate enhanced entry (with incomplete JSON timestamp for now)
    enhanced_log_entry entry{plain_entry, current_state.entity_id,
                             current_state.vector_timestamp, std::string{}};

    // check whether entry contains an SE-0001 event
    auto tmp = parse_event(entry);

    if (tmp) {
      auto& event = *tmp;

      SPDLOG_INFO("Parsed event: {}", to_string(event));

      switch (event.type) {
        default:
          break;
        case se_type::send:
          bcast(event);
          in_flight_messages.emplace_back(std::move(event));
          SPDLOG_INFO("changed in_flight_messages to \"{}\"",
                      caf::deep_to_string(in_flight_messages));

          break;
        case se_type::receive: {
          auto pred = [&](const se_event& x) {
            assert(x.type == se_type::send);
            return event.fields == x.fields;
          };

          auto e = in_flight_messages.end();
          auto i = std::find_if(in_flight_messages.begin(), e, pred);

          if (i != e) {
            merge(current_state.vector_timestamp, i->vector_timestamp);
            SPDLOG_INFO("State for \"{}\" is now: \"{}\"", plain_entry,
                        current_state);
          } else {
            merge(current_state.vector_timestamp,
                  fetch_message(event.fields).vector_timestamp);
            SPDLOG_INFO("State for \"{}\" is now: \"{}\"", plain_entry,
                        current_state);
          }

          break;
        }
        case se_type::spawn:
          in_flight_spawns.emplace_back(std::move(event));
          SPDLOG_INFO("changed in_flight_spawns to \"{}\"",
                      caf::deep_to_string(in_flight_spawns));
          break;
        case se_type::init: {
          auto id_field = std::to_string(current_state.entity_id.aid);
          auto pred = [&](const se_event& x) {
            assert(x.type == se_type::spawn);
            return get(x.fields, "ID") == id_field;
          };

          auto e = in_flight_spawns.end();
          auto i = std::find_if(in_flight_spawns.begin(), e, pred);

          if (i != e) {
            merge(current_state.vector_timestamp, i->vector_timestamp);
            SPDLOG_INFO("State for \"{}\" is now: \"{}\"", plain_entry,
                        current_state);

            // keep book on scoped actors since their terminate
            // event propagates back to the parent
            if (get(event.fields, "NAME") == "scoped_actor") {
              scoped_actors.emplace(plain_entry.id, to_logger_id(*i->source));
              SPDLOG_INFO("changed scoped_actors to \"{}\"",
                          caf::deep_to_string(scoped_actors));
            }

            in_flight_spawns.erase(i);
            SPDLOG_INFO("changed in_flight_spawns to \"{}\"",
                        caf::deep_to_string(in_flight_spawns));
          } else
            std::cerr << "*** cannot match init event to a previous spawn"
                      << std::endl;

          break;
        }
        case se_type::terminate:
          auto i = scoped_actors.find(plain_entry.id);

          if (i != scoped_actors.end()) {
            // merge timestamp with parent to capture happens-before relation
            auto parent_state_opt = state(i->second);

            if (!parent_state_opt) {
              SPDLOG_INFO(
                "Couldn't get state for \"{}\" parent of \"{}\", skipping.",
                i->second, plain_entry);
              continue;
            }

            auto& parent_state = *parent_state_opt;
            SPDLOG_INFO("Found state for \"{}\" parent of \"{}\": \"{}\"",
                        i->second, plain_entry, parent_state);

            merge(parent_state.vector_timestamp,
                  current_state.vector_timestamp);

            SPDLOG_INFO(
              "Changed vector_timestamp of \"{}\" parent of \"{}\" to \"{}\"",
              i->second, plain_entry, parent_state);

            scoped_actors.erase(i);

            SPDLOG_INFO("changed scoped_actors to \"{}\"",
                        caf::deep_to_string(scoped_actors));
          }

          break;
      }
    } else
      SPDLOG_ERROR("Couldn't parse event! plain_entry: {}", plain_entry);

    // create ShiViz compatible JSON-formatted vector timestamp
    std::ostringstream oss;
    oss << '{';
    bool need_comma = false;

    for (size_t i = 0; i < current_state.vector_timestamp.size(); ++i) {
      auto x = current_state.vector_timestamp[i];
      if (x > 0) {
        if (need_comma)
          oss << ',';
        else
          need_comma = true;
        oss << '"' << json_names[i] << '"' << ':' << x;
      }
    }

    oss << '}';
    entry.json_vector_timestamp = oss.str();
    SPDLOG_INFO("Created JSON vector timestamp: \"{}\"",
                entry.json_vector_timestamp);

    // print entry to output file
    if (!internal) {
      SPDLOG_INFO("About to write \"{}\" to \"out\"", entry);

      std::lock_guard<std::mutex> guard{out_mtx};
      out << entry << '\n';
    }
  }
}
} // namespace vec
