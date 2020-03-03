#pragma once
#include <set>

#include "entity.hpp"

/// Set of `entity` sorted in ascending order by node ID, actor ID,
/// and thread ID (in that order).
using entity_set = std::set<entity>;
