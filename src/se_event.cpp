#include "se_event.hpp"
#include "entity.hpp"

std::string to_string(const se_event& x) {
  std::string res;
  res += "node{";
  res += caf::to_string(*x.source);
  res += ", ";
  res += caf::deep_to_string(x.vstamp);
  res += ", ";
  res += to_string(x.type);
  res += ", ";
  res += caf::deep_to_string(x.fields);
  res += "}";
  return res;
}
