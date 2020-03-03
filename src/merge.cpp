#include <cassert>

#include "merge.hpp"

vector_timestamp& merge(vector_timestamp& x, const vector_timestamp& y) {
  assert(x.size() == y.size());
  for (size_t i = 0; i < x.size(); ++i)
    x[i] = std::max(x[i], y[i]);
  return x;
}
