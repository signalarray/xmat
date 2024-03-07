#pragma once

#include <cstddef>
#include <cstring>
#include <array>
#include <algorithm>
#include <exception>


namespace xmat
{

using uint = std::size_t;
using szt = std::size_t;
using ptdt = std::ptrdiff_t;


template<uint N>
uint assign(std::array<char, N>& dest, const char* src) {
  auto n = std::strlen(src) + 1;
  if (n > N) { throw std::overflow_error("cxx::assing()"); }
  std::copy_n(src, n, dest.begin());
  return n;
}

} // namespace xmat

