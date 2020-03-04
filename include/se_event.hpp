#pragma once
#include <map>
#include <string>
#include <vector>

#include <caf/all.hpp>

#include "se_type.hpp"

namespace vec {
struct entity;

/// An SE-0001 event, see http://actor-framework.github.io/rfcs/
struct se_event {
  const entity* source;
  std::vector<size_t> vector_timestamp;
  se_type type;
  std::map<std::string, std::string> fields;
};

std::string to_string(const se_event& x);
} // namespace vec

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(vec::se_event)
