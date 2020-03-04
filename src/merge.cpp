#include <cassert>

#include "merge.hpp"

namespace vec {
std::vector<size_t>& merge(std::vector<size_t>& merge_into_vector_timestamp,
                           const std::vector<size_t>& other_vector_timestamp) {
  assert(merge_into_vector_timestamp.size() == other_vector_timestamp.size());
  for (size_t i = 0; i < merge_into_vector_timestamp.size(); ++i)
    merge_into_vector_timestamp[i] = std::max(merge_into_vector_timestamp[i],
                                              other_vector_timestamp[i]);

  return merge_into_vector_timestamp;
}
} // namespace vec
