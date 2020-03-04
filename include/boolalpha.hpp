#pragma once

namespace vec {
constexpr const char* boolalpha(bool value) {
  return &"false\0true"[value * 6];
}
} // namespace vec
