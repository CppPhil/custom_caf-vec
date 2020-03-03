#pragma once
#include <string>
#include <utility>

namespace vec {
bool field_key_compare(const std::pair<const std::string, std::string>& x,
                       const std::string& y);
} // namespace vec
