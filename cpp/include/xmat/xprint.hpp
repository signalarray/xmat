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


// xdatastream.hpp
////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const XHead& h) {
  const size_t sw = 24;
  os  << "xmat::XHead:\n" 
      << std::setw(sw) << "signature: "   << h.sign_        << "\n"
      << std::setw(sw) << "bom: "         << h.bom_         << "\n"
      << std::setw(sw) << "total: "       << h.total_size_  << "\n"
      << std::setw(sw) << "sizeof_int: "  << h.i_           << "\n"
      << std::setw(sw) << "maxndim: "     << h.s_           << "\n"
      << std::setw(sw) << "maxname: "     << h.b_           << "\n";
  return os;
}

std::ostream& operator<<(std::ostream& os, const XBlock& b) {
  // ones(5, 6) + 1j >> name: <cx_double, tid>, F:[5, 6]
  const size_t sw = 15;
  const size_t sw2 = 3;
  os  << std::setw(sw) << b.name_.data() << ": "
      << b.o_ << ", " 
      << std::setw(sw2) << std::hex << static_cast<int>(b.t_) << std::dec << ", " 
      << std::setw(sw2) << static_cast<int>(b.s_) << ", " 
      << std::setw(sw2) << static_cast<int>(b.b_)
      << ", [";
  for (int n = 0; n < b.ndim(); ++n) os << b.shape_[n] << ", ";
  os << "]";
  return os;
}

template<typename IDStreamT>
std::ostream& print(std::ostream& os, IMapStream_<IDStreamT>& xin) {
  os << xin.head() 
     << "blocks:\n";
  int n = 0;
  for(auto& item : xin) { 
    os << "[" << std::setw(4) << n++ << "] " << item << "\n";
  }
  return os;
}

template<typename ODStreamT>
std::ostream& print(std::ostream& os, OMapStream_<ODStreamT>& xout) {
  auto xoutms = xout.stream().get_memsource();
  xmat::IDStreamMS<xmat::Endian::native> xin_ibb{&xoutms};
  xin_ibb.push_all();
  xmat::IMapStreamMS<xmat::Endian::native> xin{std::move(xin_ibb)};
  print(os, xin);
  return os;
}

template<typename IDStreamT>
std::ostream& operator<<(std::ostream& os, IMapStream_<IDStreamT>& xin) {
  return print(os, xin);
}

template<typename ODStreamT>
std::ostream& operator<<(std::ostream& os, OMapStream_<ODStreamT>& xout) {
  return print(os, xout);
}
} // namespace end
