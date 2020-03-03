#pragma once
#include <caf/all.hpp>

#include "se_event.hpp"

struct enhanced_log_entry;

caf::expected<se_event> parse_event(const enhanced_log_entry& x);
