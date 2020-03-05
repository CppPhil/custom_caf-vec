#pragma once
#include <cctype>

#include <functional>
#include <istream>
#include <string>
#include <utility>

#include "trim.hpp"

namespace vec::io {
namespace detail {
template <class UnaryInvocable>
struct read_until_t {
  explicit read_until_t(UnaryInvocable&& unary_invocable)
    : invocable(std::move(unary_invocable)) {
  }

  UnaryInvocable invocable;

  friend std::istream& operator>>(std::istream& istr,
                                  read_until_t&& structure) {
    return std::invoke(structure.invocable, istr);
  }
};
} // namespace detail

template <class UnaryPredicate>
auto read_until(UnaryPredicate pred) {
  return [pred = std::move(pred)](std::string& buffer) {
    return detail::read_until_t{[pred = std::move(pred), &buffer](
                                  std::istream& istream) -> std::istream& {
      buffer.clear();

      for (std::istream::traits_type::int_type i
           = std::istream::traits_type::eof();
           (i = istream.peek(),
           (i != std::istream::traits_type::eof()
            && !std::invoke(pred, static_cast<char>(i))));) {
        buffer.push_back(static_cast<char>(istream.get()));
        volatile bool b = istream.operator bool();
        (void) b;
      }

      trim(buffer);
      return istream;
    }};
  };
}
} // namespace vec::io
