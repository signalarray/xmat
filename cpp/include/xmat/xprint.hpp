#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <iostream>
#include <iomanip>

#include <type_traits>

#include "xarray.hpp"
#include "xdatastream.hpp"

namespace xmat {

template<typename T, std::enable_if_t<std::is_scalar<T>::value, int> = 0>
void print_bytes(std::ostream& os, T v, char term = '\n') {
  os << std::hex << std::uppercase << std::setfill('0')
     << std::setw(sizeof(T) * 2) << v << " : ";
  for (std::size_t i{}; i != sizeof(T); ++i, v >>= 8) {
    os << std::setw(2) << static_cast<unsigned>(T(0xFF) & v) << ' ';
  }
  os << std::dec << term;
}

} // namespace end
