#pragma once
#include <caf/all.hpp>

#include "thread_id.hpp"

struct entity;
class thread_range;
class node_range;
class logger_id;

const entity* get(const thread_range& xs, const thread_id& y);

const entity* get(const node_range& xs, const thread_id& y);

/// Returns the entity for `y` from the node range `xs`.
const entity* get(const node_range& xs, caf::actor_id y);

const entity* get(const node_range& xs, const logger_id& y);
