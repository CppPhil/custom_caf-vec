#pragma once
#include "entity.hpp"

#include <caf/all.hpp>

struct node_cmp_t {
  bool operator()(const entity& x, const caf::node_id& y) const;

  bool operator()(const caf::node_id& x, const entity& y) const;
};

constexpr node_cmp_t node_cmp = node_cmp_t{};
