#pragma once
#include <map>

#include <caf/all.hpp>

#include "logger_id.hpp"
#include "logger_id_meta_data.hpp"
#include "verbosity_level.hpp"

namespace vec {
/// Stores all log entities and their node ID.
struct first_pass_result {
  /// Node ID used in the parsed file.
  caf::node_id this_node;
  /// Entities of the parsed file. The value is `true` if an entity is
  /// hidden, otherwise `false`.
  std::map<logger_id, logger_id_meta_data> entities;
};

caf::expected<first_pass_result>
first_pass(caf::blocking_actor* self, std::istream& in, verbosity_level vl);
} // namespace vec
