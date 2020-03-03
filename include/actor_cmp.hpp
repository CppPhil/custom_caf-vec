#pragma once
#include "entity.hpp"

/// Range within an `entity_set` containing all entities for a given actor.
struct actor_cmp_t {
  bool operator()(const entity& x, caf::actor_id y) const;

  bool operator()(caf::actor_id x, const entity& y) const;
};

constexpr actor_cmp_t actor_cmp = actor_cmp_t{};
