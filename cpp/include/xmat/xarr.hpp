#pragma once

#include <cstddef>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <array>
#include <initializer_list>
#include <algorithm>
#include <memory>


namespace xmat {

using uint = std::size_t;   // just to have short name
const uint kMaxNDimIndex = 4;

enum class Order: char {
  C = 'C',    // C-language style order: by rows
  F = 'F'     // Fortran style order:    by columns
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

// complex<T> conversion
// ---------------------
template<typename To, typename From>
std::enable_if_t<std::is_fundamental<To>::value && std::is_fundamental<From>::value, To> 
/*To*/ cast(const From& a) {
  return static_cast<To>(a);
}

template<typename Complex, typename From>
std::enable_if_t<std::is_same<std::complex<typename Complex::value_type>, Complex>::value && std::is_fundamental<From>::value, Complex> 
/*Complex*/ cast(const From& a) {
  return Complex{static_cast<typename Complex::value_type>(a), typename Complex::value_type{0}};
}

template<typename Complex1, typename Complex2>
std::enable_if_t<std::is_same<std::complex<typename Complex1::value_type>, Complex1>::value && 
                 std::is_same<std::complex<typename Complex2::value_type>, Complex2>::value, Complex1> 
/*Complex2*/ cast(const Complex2& a) {
  return Complex1{static_cast<typename Complex1::value_type>(a.real()), 
                  static_cast<typename Complex1::value_type>(a.imag())};
}


using IndexBase = std::array<uint, kMaxNDimIndex>;

class Index : public IndexBase {
 public:
  
  using IndexBase::IndexBase;

  Index(const IndexBase& arr) { static_cast<IndexBase&>(*this) = arr; }

  template<typename It>
  Index(It begin, It end) {
    fill(0);
    uint n = 0;
    for(It it=begin; it != end && n < size(); ++it, ++n) { at(n) = *it; }
  }

  Index(std::initializer_list<uint> shape) {
    fill(0);
    const uint n0 = shape.size();
    assert(n0 <= kMaxNDimIndex);
    const uint n1 = std::min(n0, kMaxNDimIndex);
    const auto cend = shape.begin() + n1;
    auto out = this->begin();
    for (auto iter = shape.begin(); iter != cend; ++iter, ++out) {
      *out = *iter;
    }
  }

  uint ndim() const {
    uint ndim = 0;
    for (auto item : (*this)) { ndim += uint(item != 0 ); }
    return ndim;
  }

  uint numel() const {
    uint size = 1;
    for (auto item : (*this)) { size *= (item + uint(item == 0)); }
    return size;
  }

  void fill(uint v) { std::fill(this->begin(), this->end(), v); }
};


class Map {
 public:

  Map() { init(); }
  Map(const Index& shape) : ndim(shape.ndim()), shape(shape) { init(); }
  Map(uint sz0) : Map(Index({sz0})) { }
  Map(uint sz0, uint sz1) : Map(Index({sz0, sz1})) { }
  Map(uint sz0, uint sz1, uint sz2) : Map(Index({sz0, sz1, sz2})) { }

  uint at(const Index& idx) const {
    uint I = 0;
    for (uint n = 0; n < ndim; ++n) {
      const uint in = idx[n];
      assert(in < shape[n]);
      I += in * stride[n];
    }
    return I;
  }

  uint at(uint i0, uint i1=0, uint i2=0, uint i3=0) const {return at(Index{{i0,i1,i2,i3}});}

  Order set_order(Order order_) {
    Order prev_order = order;
    if (order_ != order) {
      order = order_;
      init_stride();
    }
    return prev_order;
  }

 // properties
  Order order = Order::C;
  uint ndim = 0;
  uint numel = 0;
  Index shape{0};
  Index stride{0};

 private:
  void init() {
    numel = shape.numel();
    init_stride();
  }

  //
  void init_stride() {
    if (order == Order::C) {
      stride.fill(0);
      if (ndim == 0) return;
      stride[ndim - 1] = 1;
      for (uint i = ndim - 2, end_(-1); i != end_; --i) {
        stride[i] = stride[i + 1] * shape[i + 1];
      }
    }
    else if (order == Order::F) {
      stride.fill(0);
      if (ndim == 0) return;
      stride[0] = 1;
      for (uint i = 1; i != ndim; ++i) {
        stride[i] = stride[i - 1] * shape[i - 1];
      }
    }
  }
};


template<typename T>
class Array {
 public:
  using value_type = T;
  using index_type = uint;
  using idx_t = index_type;

  Array() = default;
  Array(const Index& shape, T* data=nullptr) : map(shape) { init(data); }

  Array(const Array& arr) = delete;
  Array& operator=(const Array& arr) = delete;

  Array(Array&& other) { swap(other); }

  Array& operator=(Array&& other) {
    Array tmp{std::move(other)};
    swap(tmp);
    return *this;
  }

  void swap(Array& other) noexcept {
    std::swap(other.map, map);
    other.ptr_.swap(ptr_);
    std::swap(other.data_, data_);
  }

  template<typename U>
  Array<U> cast() const {
    Array<U> arr{map.shape};
    auto item = data();
    auto item_arr = arr.data();
    for (uint n = 0, N = numel(); n < N; ++n, ++item, ++item_arr) {
      *item_arr = xmat::cast<U>(*item);
    }
    return arr;
  }



  T& operator()(const Index& idx) {return data_[map.at(idx)]; }
  T& operator()(uint i0, uint i1=0, uint i2=0, uint i3=0) {return data_[map.at(i0, i1, i2, i3)];}

  const T& operator()(const Index& idx) const {return data_[map.at(idx)];}
  const T& operator()(uint i0, uint i1=0, uint i2=0, uint i3=0) const {return data_[map.at(i0, i1, i2, i3)];}

  // fillers
  // -------
  Array& enumerate() {
    auto item = data();
    for (uint n = 0, N = numel(); n < N; ++n, ++item) *item = n;
    return *this;
  }

  Array& fill(const T& v) { 
    std::fill(data_, data_ + map.numel, v); 
    return *this;
  }

 public:
  T* data() { return data_; }
  const T* data() const { return data_; }

  bool is_c_order() const noexcept { return order() == Order::C; }
  bool is_f_order() const noexcept { return order() == Order::F; }
  Order order() const noexcept { return map.order; }
  uint ndim() const noexcept { return map.ndim; }
  uint numel() const noexcept { return map.numel; }
  const Index& shape() const noexcept { return map.shape; }
  const Index& stride() const noexcept { return map.stride; }

  // properties
  Map map;
  T* data_ = nullptr;

 private:
  void init(T* ptr) {
    if (ptr) {
      ptr = ptr;
    }
    else {
      ptr_ = std::make_unique<T[]>(map.numel);
      data_ = ptr_.get();
    }
  }

  std::unique_ptr<T[]> ptr_;
};


class Slice {
 public:
  uint start = 0;
  uint stop = 0;
  uint step = 0;
};


class View {
 public:
  Slice s[kMaxNDimIndex];
};


template<typename T>
class Box {

};


template<typename T>
class Grid {

};


template<typename T>
class Linspace {

};


template<typename T>
void print(std::ostream& os, const xmat::Array<T>& x, uint space=0) {
  if (x.map.ndim == 1) {
      for (uint i = 0, N0 = x.map.shape[0]; i < N0; ++i) {
        if (space) os << std::setw(space);
      os << x(i) << ", ";
    }
  }
  if (x.map.ndim == 2) {
      for (uint i0 = 0, N0 = x.map.shape[0]; i0 < N0; ++i0) {
        for (uint i1 = 0, N1 = x.map.shape[1]; i1 < N1; ++i1) {
          if (space) os << std::setw(space);
        os << x(i0, i1) << ", ";
      }
      os << "\n";
    }
  }
}

} // namespace xmat
