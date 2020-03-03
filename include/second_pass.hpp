#pragma once
#include <iosfwd>
#include <mutex>
#include <string>
#include <vector>

#include <caf/all.hpp>

#include "entity_set.hpp"
#include "verbosity_level.hpp"

void second_pass(caf::blocking_actor* self, const caf::group& grp,
                 const entity_set& entities, const caf::node_id& nid,
                 const std::vector<std::string>& json_names, std::istream& in,
                 std::ostream& out, std::mutex& out_mtx,
                 bool drop_hidden_actors, verbosity_level vl);
