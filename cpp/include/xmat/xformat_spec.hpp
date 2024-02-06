#pragma once

#include <exception>
#include <algorithm>

#include "xformat.hpp"
#include "xarr.hpp"


namespace xmat {


class FormatError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};


// ------
// scalar
// ------
template<typename T>
Block xblock(const T& X) {
  Block block;
  TypeInfo<T> info;
  assign(block.typename_, info.name);
  block.shape_ = {0};
  block.numel_ = 1;
  block.typesize_ = info.size;
  block.ndim_ = 0;
  block.ptr_ = reinterpret_cast<const char*>(&X);
  return block;
}

template<typename T>
std::enable_if_t<TypeInfo<T>::registered, T>
from_xblock(const Block& block, StreamBase& stream) {
  T y;
  from_xblock(block, stream, y);
  return y;
}

template<typename T>
std::enable_if_t<TypeInfo<T>::registered, void>
from_xblock(const Block& block, StreamBase& stream, T& y) {
  if(!block.check_element<T>()) { throw FormatError("wrong scalar type"); }
  if( !(block.numel_ == 1 && block.ndim_ == 0 )){ throw FormatError("wrong numel|ndim"); }
  if(!check_shape_0d(block.shape_, block.ndim_)) { 
    throw FormatError("wrong shape "); 
  }
  stream.seek(block.pos_, std::ios_base::beg);
  stream.read(y);
}

// ---------
// c++ array
// ---------
template<typename T, uint N>
Block xblock(const T (&x)[N]) {
  Block block;
  static_assert(TypeInfo<T>::registered, "unsupported type");
  TypeInfo<T> info;
  assign(block.typename_, info.name);
  block.shape_ = {N};
  block.numel_ = N;
  block.typesize_ = info.size;
  block.ndim_ = 1;
  block.ptr_ = reinterpret_cast<const char*>(x.data());
  return block;
}

template<typename T, uint N>
std::enable_if_t<TypeInfo<T>::registered, void>
from_xblock(const Block& block, StreamBase& stream, T(&y)[N]) {
  if(!block.check_element<T>()) { throw FormatError("wrong scalar type"); }
  if(!(block.numel_ == N && block.ndim_ == 1 )){ throw FormatError("wrong numel|ndim"); }
  if(!check_shape_1d(N, block.shape_, block.ndim_)) {
    throw FormatError("wrong shape for array[]. one none-zero element expected"); 
  }

  stream.seek(block.pos_, std::ios_base::beg);
  stream.read(y, N);
}


// ---------------------------------------------------------
// std continuous sequense containers: vector, array, string
// ---------------------------------------------------------
template<typename C>
Block impl_xblock_std(const C& x) {
  using value_t = typename C::value_type;

  Block block;
  TypeInfo<value_t> info;
  assign(block.typename_, info.name);
  block.shape_ = {x.size()};
  block.numel_ = x.size();
  block.typesize_ = info.size;
  block.ndim_ = 1;
  block.ptr_ = reinterpret_cast<const char*>(x.data());
  return block;
}

template<typename T, typename A> 
Block xblock(const std::vector<T, A>& x) { return impl_xblock_std(x); }

template<typename T, uint A> Block 
xblock(const std::array<T, A>& x) { return impl_xblock_std(x); }

Block xblock(const std::string& s) { return impl_xblock_std(s); }


// if sizes aren't equal: exception
template<typename C>
void impl_from_xblock_std_fix(const Block& block, StreamBase& stream, C& y) {
  using T = typename C::value_type;
  if(!block.check_element<T>()) { throw FormatError("wrong scalar type"); }
  if(!(block.numel_ == y.size())){ throw FormatError("wrong numel|ndim"); }
  if(!check_shape_1d(y.size(), block.shape_, block.ndim_)) { 
    throw FormatError("wrong shape for array[]. one none-zero element expected");
  }
  stream.seek(block.pos_, std::ios_base::beg);
  stream.read(&y[0], y.size());
}

template<typename T, typename A> 
void from_xblock(const Block& block, StreamBase& stream, std::vector<T, A>& y) {
  return impl_from_xblock_std_fix(block, stream, y);
}

template<typename T, uint N>
void from_xblock(const Block& block, StreamBase& stream, std::array<T, N>& y) {
  return impl_from_xblock_std_fix(block, stream, y);
}

void from_xblock(const Block& block, StreamBase& stream, std::string& y) {
  return impl_from_xblock_std_fix(block, stream, y);
}

// if sizes aren't equal: resize
template<typename C>
void impl_from_xblock_std_dyn(const Block& block, StreamBase& stream, C& y) {
  using T = typename C::value_type;
  if(!block.check_element<T>()) { throw FormatError("wrong scalar type"); }
  if(!check_shape_1d(y.size(), block.shape_, block.ndim_)) {
    throw FormatError("wrong shape for array[]. one none-zero element expected");
  }
  y.resize(block.numel_);
  stream.seek(block.pos_, std::ios_base::beg);
  stream.read(&y[0], y.size());
}

template<typename T, typename A> 
void from_xblock_dyn(const Block& block, StreamBase& stream, std::vector<T, A>& y) {
  return impl_from_xblock_std_dyn(block, stream, y);
}

void from_xblock_dyn(const Block& block, StreamBase& stream, std::string& y) { 
  return impl_from_xblock_std_dyn(block, stream, y);
}

template<typename T, typename A>
std::vector<T, A> from_xblock_dyn(const Block& block, StreamBase& stream) {
  std::vector<T, A> y;
  impl_from_xblock_std_dyn(block, stream, y);
  return y;
}

std::string from_xblock_dyn(const Block& block, StreamBase& stream) {
  std::string y;
  impl_from_xblock_std_dyn(block, stream, y);
  return y;
}

// -----------
// xmat::Array
// -----------
template<typename T>
xmat::Block xblock(const xmat::Array<T>& x) {
  xmat::Block block;
  xmat::TypeInfo<T> info;
  xmat::assign(block.typename_, info.name);
  assert(x.map.ndim <= block.shape_.size());
  std::copy_n(x.map.shape.begin(), x.map.ndim, block.shape_.begin());
  block.numel_ = x.map.numel;
  block.typesize_ = info.size;
  block.ndim_ = x.map.ndim;
  block.ptr_ = reinterpret_cast<const char*>(x.data_);
  return block;
}


// fixed shape
template<typename T>
void from_xblock(const Block& block, StreamBytes& stream, Array<T>& y) {
  if(!block.check_element<T>()) { throw FormatError("wrong scalar type"); }
  if(!(block.numel_ == y.numel())){ throw FormatError("wrong numel|ndim"); }
  if(!std::equal(y.shape().cbegin(), y.shape().cend(), block.shape_)) {
    throw FormatError("wrong shape");
  }

  stream.seek(block.pos_, std::ios_base::beg);
  stream.read(y.data_(), y.numel());
}

template<typename T>
Array<T> from_xblock(const Block& block, StreamBytes& stream) {
  
}
} // namespace xmat


