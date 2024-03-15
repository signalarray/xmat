#pragma once

#include <exception>
#include <algorithm>
#include <type_traits>

#include "xformat.hpp"
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


template<uint N>
bool check_shape_0d(const std::array<uint, N>& s, uint ndim) {
  bool out = true;
  for(uint n : s) { out = out && n == 0; }
  return out;
}

template<uint N>
bool check_shape_1d(uint len, const std::array<uint, N>& s, uint ndim) {
  uint k = 0, m = 0;
  for(uint n = 0; n < ndim; ++n) {
    const uint a = s[n];
    if(a == len) { ++k; } 
    else if(a == 1) { ++m; }
  }
  bool out = k == 1 && m == ndim - 1;
  for(uint n = ndim; n < N; ++n) { out = out && s[n] == 0; }
  return out;
}


// ------
// scalar
// ------
template <typename T>
struct XSerial<T, std::enable_if_t<TypeInfo<T>::registered>> {
  static Block dump(const T& x) {
    Block block;
    TypeInfo<T> info;
    assign(block.typename_, info.name);
    block.shape_ = {1};
    block.numel_ = 1;
    block.typesize_ = info.size;
    block.ndim_ = 1;
    block.ptr_ = reinterpret_cast<const char*>(&x);
    return block;
  }

  static bool is_dyn(const Block& block) {
    return block.check_element<T>() &&
           block.numel_ == 1 &&
           check_shape_1d(1, block.shape_, block.ndim_);
  }

  static bool is_fix(const Block& block, const T&x) { (void)x; return is_dyn(block); }

  static void load(const Block& block, StreamBase& stream, T& y) {
    if(!block.check_element<T>()) { 
      throw DeserializationError("Scalar load(): wrong scalar type"); 
    }
    if( !(block.numel_ == 1)){ 
      throw DeserializationError("Scalar load(): wrong numel|ndim"); 
    }
    if(!check_shape_1d(1, block.shape_, block.ndim_)) {
      throw DeserializationError("Scalar load(): wrong shape ");
    }
    stream.seek(block.pos_, std::ios_base::beg);
    stream.read(y);
  }

  static T load(const Block& block, StreamBase& stream) { 
    T y; load(block, stream, y); return y;
  }
};

// ---------------------------------------------------------
// std containers continuous sequense: vector, string, array
// ---------------------------------------------------------
namespace impl_std {
  template<typename U>
  Block xserial_dump(const U& x){
    using T = typename U::value_type;
    Block block;
    TypeInfo<T> info;
    assign(block.typename_, info.name);
    block.shape_ = {x.size()};
    block.numel_ = x.size();
    block.typesize_ = info.size;
    block.ndim_ = 1;
    block.ptr_ = reinterpret_cast<const char*>(x.data());
    return block;
  }

  template<typename U>
  bool xserial_is_dyn(const Block& block) {
    using T = typename U::value_type;
    return block.check_element<T>();
  }

  template<typename U>
  bool xserial_is_fix(const Block& block, const U&y) {
    using T = typename U::value_type;
    return block.check_element<T>()
           && block.numel_ != y.size()
           && check_shape_1d(y.size(), block.shape_, block.ndim_);
  }

  template<typename U>
  void xserial_load(const Block& block, StreamBase& stream, U& y, bool flag_fix=true) {
    using T = typename U::value_type;
    if(!block.check_element<T>()) {
      throw DeserializationError("wrong scalar type");
    }
    if(flag_fix && block.numel_ != y.size()) {
      throw DeserializationError("frong size");
    }
    if(flag_fix && !check_shape_1d(y.size(), block.shape_, block.ndim_)) {
      throw DeserializationError("wrong shape for array[]. one none-zero element expected");
    }
    y.resize(block.numel_);
    stream.seek(block.pos_, std::ios_base::beg);
    stream.read(&y[0], y.size());
  }

  template<typename U>
  void xserial_load_fix(const Block& block, StreamBase& stream, U& y) {
    using T = typename U::value_type;
    if(!block.check_element<T>()) { 
      throw DeserializationError("wrong scalar type"); 
    }
    if(block.numel_ != y.size()){ 
      throw DeserializationError("wrong numel|ndim"); 
    }
    if(!check_shape_1d(y.size(), block.shape_, block.ndim_)) { 
      throw DeserializationError("wrong shape for array[]. one none-zero element expected");
    }
    stream.seek(block.pos_, std::ios_base::beg);
    stream.read(&y[0], y.size());
  }
}

template<typename T, typename A>
struct XSerial<std::vector<T, A>, std::enable_if_t<TypeInfo<T>::registered>> {
  using U = std::vector<T, A>;

  static Block dump(const U& x) { return impl_std::xserial_dump(x); }

  static bool is_dyn(const Block& block) { return impl_std::xserial_is_dyn<U>(block); }
  static bool is_fix(const Block& block, const U& y) { return impl_std::xserial_is_fix<U>(block, y); }

  static void load(const Block& block, StreamBase& stream, U& y) {
    return impl_std::xserial_load(block, stream, y, false);
  }

  static U load(const Block& block, StreamBase& stream) {
    U y; load(block, stream, y); return y;
  }
};


template<typename T, std::size_t N>
struct XSerial<std::array<T, N>, std::enable_if_t<TypeInfo<T>::registered>> {
  using U = std::array<T, N>;

  static Block dump(const U& x) { return impl_std::xserial_dump(x); }

  static bool is_dyn(const Block& block) {
    return block.check_element<T>() 
           && block.numel_ != N
           && check_shape_1d(N, block.shape_, block.ndim_);
  }

  static bool is_fix(const Block& block, const U& y) { return impl_std::xserial_is_fix<U>(block, y); }

  static void load(const Block& block, StreamBase& stream, U& y) {
    return impl_std::xserial_load_fix(block, stream, y);
  }

  static U load(const Block& block, StreamBase& stream) {
    U y; load(block, stream, y); return y;
  }
};


template<>
struct XSerial<std::string> {
  using T = std::string::value_type;
  using U = std::string;

  static Block dump(const U& x) { return impl_std::xserial_dump(x); }

  static bool is_dyn(const Block& block) { return impl_std::xserial_is_dyn<U>(block); }
  static bool is_fix(const Block& block, const U& y) { return impl_std::xserial_is_fix<U>(block, y); }

  static void load(const Block& block, StreamBase& stream, U& y) {
    return impl_std::xserial_load(block, stream, y, false);
  }

  static U load(const Block& block, StreamBase& stream) {
    U y; load(block, stream, y); return y;
  }
};

// -----------
// xmat::Array
// -----------
template <typename T, std::size_t ND, class StorageT>
struct XSerial<Array<T, ND, StorageT>, std::enable_if_t<TypeInfo<T>::registered>> {
  using U = Array<T, ND, StorageT>;
  
  static Block dump(const U& x) {
    xmat::Block block;
    xmat::TypeInfo<T> info;
    xmat::assign(block.typename_, info.name);
    assert(x.ndim() <= block.shape_.size());
    std::copy_n(x.shape().begin(), x.ndim(), block.shape_.begin());
    block.numel_ = x.numel();
    block.typesize_ = info.size;
    block.ndim_ = x.ndim();
    block.ptr_ = reinterpret_cast<const char*>(x.data());
    return block;
  }

  static bool is_dyn(const Block& block) { return block.check_element<T>(); }
  
  static bool is_fix(const Block& block, const U& y) { 
    auto shape = y.shape();
    return block.check_element<T>()
           && block.numel_ != y.numel()
           && block.ndim_ > ND
           && std::equal(shape.cbegin(), shape.cbegin() + block.ndim_, 
                         block.shape_.cbegin());
  }

  static void load(const Block& block, StreamBase& stream, U& y, bool flag_fix=true) {
    if(!block.check_element<T>()) { 
      throw DeserializationError("wrong scalar type");
    }
    if(flag_fix && block.numel_ != y.numel()){
      throw DeserializationError("wrong numel|ndim");
    }
    if(block.ndim_ > ND){ 
      throw DeserializationError("ndim is too big");
    }

    auto shape = y.shape();
    if(!std::equal(shape.cbegin(), shape.cbegin() + block.ndim_, block.shape_.cbegin())) {
      if(flag_fix) { 
        throw DeserializationError("wrong shape");
      } else {
        typename U::index_t shape;
        shape.fill(1);
        // std::copy_n(block.shape_.cbegin(), block.ndim_, shape.begin());
        std::copy_n(block.shape_.cbegin() + (ND - block.ndim_), block.ndim_, shape.begin());
        y = U{shape}; // TODO - spetialize for array_static
      }
    }
    stream.seek(block.pos_, std::ios_base::beg);
    stream.read(y.data(), y.numel());
  }

  static U load(const Block& block, StreamBase& stream) {
    U y; load(block, stream, y, false); return y;
  }
};

} // namespace xmat
