#include "se_type.hpp"

namespace vec {
std::string to_string(se_type x) {
  const char* tbl[] = {"spawn", "init", "send",     "reject",    "receive",
                       "drop",  "skip", "finalize", "terminate", "none"};
  return tbl[static_cast<int>(x)];
}
} // namespace vec
