#include "field_key_compare.hpp"

namespace vec {
bool field_key_compare(const std::pair<const std::string, std::string>& x,
                       const std::string& y) {
  return x.first == y;
}
} // namespace vec
