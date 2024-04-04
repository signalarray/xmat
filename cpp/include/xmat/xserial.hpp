#pragma once

#include <string>
#include <vector>
#include <array>
#include <exception>
#include <algorithm>
#include <type_traits>

#include "xutil.hpp"
#include "xstream.hpp"
#include "xarray.hpp"


namespace xmat
{

class SerializationError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

class DeserializationError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};


template<size_t N>
bool check_shape_0d(const std::array<size_t, N>& s, size_t ndim) {
  bool out = true;
  for(size_t n : s) { out = out && n == 0; }
  return out;
}

template<size_t N>
bool check_shape_1d(size_t len, const std::array<size_t, N>& s, size_t ndim) {
  size_t k = 0, m = 0;
  for(size_t n = 0; n < ndim; ++n) {
    const size_t a = s[n];
    if(a == len) { ++k; } 
    else if(a == 1) { ++m; }
  }
  bool out = k == 1 && m == ndim - 1;
  for(size_t n = ndim; n < N; ++n) { out = out && s[n] == 0; }
  return out;
}


// ------
// scalar
// ------
template <typename T>
struct Serializer<T, std::enable_if_t<TypeInfo<T>::registered>> {
  static const bool enabled = true;

  static StreamBlock dump(const T& x) {
    StreamBlock block;
    TypeInfo<T> info;
    assign(block.typename_, info.name);
    block.shape_ = {1};
    block.numel_ = 1;
    block.typesize_ = info.size;
    block.ndim_ = 1;
    block.ptr_ = reinterpret_cast<const char*>(&x);
    return block;
  }

  template<typename IBuff>
  static T& load_to(const StreamBlock& block, IBuff& ibuff, T& y) {
    if(!block.check_element<T>()) throw DeserializationError("Scalar load(): wrong scalar type");
    if( !(block.numel_ == 1)) throw DeserializationError("Scalar load(): wrong numel|ndim");
    if(!check_shape_1d(1, block.shape_, block.ndim_)) throw DeserializationError("Scalar load(): wrong shape ");

    ibuff.seekg(block.pos_ + k_block_size, std::ios_base::beg);
    util_read(ibuff, y);
    return y;
  }

  template<typename IBuff>
  static T load(const StreamBlock& block, IBuff& ibuf) {
    T y; 
    return load_to(block, ibuf, y);
  }
};


//--------
// pointer
// -------
template <typename T>
struct Serializer<T*, std::enable_if_t<TypeInfo<T>::registered>> {
  static const bool enabled = true;

  static StreamBlock dump_n(const T* xptr, size_t n) {
    StreamBlock block;
    TypeInfo<T> info;
    assign(block.typename_, info.name);
    block.shape_ = {n};
    block.numel_ = n;
    block.typesize_ = info.size;
    block.ndim_ = 1;
    block.ptr_ = reinterpret_cast<const char*>(xptr);
    return block;
  }

  // not save
  template<typename IBuff>
  static T* load_to(const StreamBlock& block, IBuff& ibuff, T* yptr) {
    if(!block.check_element<T>()) throw DeserializationError("pointer load(): wrong scalar type");
    if(!check_shape_1d(block.numel_, block.shape_, block.ndim_)) throw DeserializationError("pointer load(): wrong shape ");
    
    ibuff.seekg(block.pos_ + k_block_size, std::ios_base::beg);
    util_read(ibuff, yptr, block.numel_);
    return yptr;
  }
};


// ---------------------------------------------------------
// std containers continuous sequense: vector, string, array
// ---------------------------------------------------------
namespace impl_std {
  template<typename U>
  StreamBlock dump(const U& x){
    using T = typename U::value_type;
    return Serializer<T*>::dump_n(&x[0], x.size());
  }

  template<typename U, typename IBuff>
  U& load_to(const StreamBlock& block, IBuff& ibuff, U& y) {
    using T = typename U::value_type;
    if(!block.check_element<T>()) throw DeserializationError("wrong scalar type");
    if(!check_shape_1d(block.numel_, block.shape_, block.ndim_)) {
      throw DeserializationError("wrong shape for std::container");
    }
    y.resize(block.numel_);
    ibuff.seekg(block.pos_ + k_block_size, std::ios_base::beg);
    T* ptr = &y[0];
    util_read(ibuff, ptr, block.numel_);
    return y;
  }
} // namespase impl_std

// vector
template<typename T, typename A>
struct Serializer<std::vector<T, A>, std::enable_if_t<TypeInfo<T>::registered>> {
  static const bool enabled = true;
  using U = std::vector<T, A>;

  static StreamBlock dump(const U& x) { return impl_std::dump(x); }

  template<typename IBuff>
  static U& load_to(const StreamBlock& block, IBuff& ibuff, U& y) {
    return impl_std::load_to(block, ibuff, y);
  }

  template<typename IBuff>
  static U load(const StreamBlock& block, IBuff& ibuff) {
    U y; return load_to(block, ibuff, y);
  }
};

// std::string
template<>
struct Serializer<std::string, 
                  std::enable_if_t<TypeInfo<typename std::string::value_type>::registered>> {
  static const bool enabled = true;
  using U = std::string;

  static StreamBlock dump(const U& x) { return impl_std::dump(x); }

  template<typename IBuff>
  static U& load_to(const StreamBlock& block, IBuff& ibuff, U& y) {
    return impl_std::load_to(block, ibuff, y);
  }

  template<typename IBuff>
  static U load(const StreamBlock& block, IBuff& ibuff) {
    U y; return load_to(block, ibuff, y);
  }
};

// std::array
template<typename T, size_t N>
struct Serializer<std::array<T, N>, std::enable_if_t<TypeInfo<T>::registered>> {
  static const bool enabled = true;
  using U = std::array<T, N>;

  static StreamBlock dump(const U& x) { return impl_std::dump(x); }

  template<typename IBuff>
  static U& load_to(const StreamBlock& block, IBuff& ibuff, U& y) {
    if(!block.check_element<T>()) throw DeserializationError("wrong scalar type");
    if(block.numel_ != y.size()) throw DeserializationError("wrong size for std::array<T,N>");
    if(!check_shape_1d(y.size(), block.shape_, block.ndim_)) {
      throw DeserializationError("wrong shape for std::container");
    }
    ibuff.seekg(block.pos_ + k_block_size, std::ios_base::beg);
    util_read(ibuff, y.data(), N);
    return y;
  }

  template<typename IBuff>
  static U load(const StreamBlock& block, IBuff& ibuff) {
    U y; return load_to(block, ibuff, y);
  }
};

// -------------
// xmat::Array<>
// -------------
template <typename T, std::size_t ND, class S>
struct Serializer<NArray_<T, ND, S>, std::enable_if_t<TypeInfo<T>::registered>> {
  static const bool enabled = true;
  using array_t = NArray_<T, ND, S>;
  
  static StreamBlock dump(const array_t& x) {
    StreamBlock block;
    TypeInfo<T> info;
    assign(block.typename_, info.name);
    assert(x.ndim() <= block.shape_.size());
    std::copy_n(x.shape().begin(), x.ndim(), block.shape_.begin());
    block.numel_ = x.numel();
    block.typesize_ = info.size;
    block.ndim_ = x.ndim();
    block.ptr_ = reinterpret_cast<const char*>(x.data());
    return block;
  }

  template<typename IBuff>
  static array_t& load_to(const StreamBlock& block, IBuff& ibuff, array_t& y) {
    if(!block.check_element<T>()) throw DeserializationError("wrong scalar type");
    if(block.ndim_ > ND) throw DeserializationError("wrong ndim");

    auto shape = y.shape();
    if(!std::equal(shape.cbegin(), shape.cbegin() + block.ndim_, block.shape_.cbegin())) {
      throw DeserializationError("wrong array`s shape");
    }
    ibuff.seekg(block.pos_ + k_block_size, std::ios_base::beg);
    util_read(ibuff, y.data(), y.numel());
    return y;
  }

  template<typename IBuff>
  static array_t load_with_allocator(const StreamBlock& block, IBuff& ibuff, const S& memsrc) {
    if(!block.check_element<T>()) throw DeserializationError("wrong scalar type");
    if(block.ndim_ > ND) throw DeserializationError("wrong ndim");

    ibuff.seekg(block.pos_ + k_block_size, std::ios_base::beg);

    typename array_t::index_t shape;
    shape.fill(1);
    std::copy_n(block.shape_.cbegin() + (ND - block.ndim_), block.ndim_, shape.begin());
    
    array_t y{shape, S{memsrc}};
    util_read(ibuff, y.data(), y.numel());
    return y;
  }

  
  template<typename IBuff, 
           bool HDC = array_t::k_has_default_constructor, 
           std::enable_if_t<HDC, bool> = false>
  static array_t load(const StreamBlock& block, IBuff& ibuff) {
    return load_with_allocator(block, ibuff, S{});
  }
};
} // namespace xmat
