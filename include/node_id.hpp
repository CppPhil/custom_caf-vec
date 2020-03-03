#pragma once
#include <iosfwd>

#include <caf/all.hpp>

std::istream& operator>>(std::istream& in, caf::node_id& x);
