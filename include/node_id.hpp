#pragma once
#include <iosfwd>

#include <caf/all.hpp>

namespace caf {
std::istream& operator>>(std::istream& in, caf::node_id& x);
} // namespace caf
