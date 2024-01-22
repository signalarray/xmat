#pragma once

#include <cstddef>
#include <cassert>
#include <cmath>
#include <iostream>
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


using IndexBase = std::array<uint, kMaxNDimIndex>;

class Index : public IndexBase {
 public:
  
  using IndexBase::IndexBase;

  Index(const IndexBase& arr) { static_cast<IndexBase&>(*this) = arr; }

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

  T& operator()(const Index& idx) {return data[map.at(idx)];}
  T& operator()(uint i0, uint i1=0, uint i2=0, uint i3=0) {return data[map.at(i0, i1, i2, i3)];}

  const T& operator()(const Index& idx) const {return data[map.at(idx)];}
  const T& operator()(uint i0, uint i1=0, uint i2=0, uint i3=0) const {return data[map.at(i0, i1, i2, i3)];}

  void fill(const T& v) { std::fill(data, data + map.numel, v); }

  // properties
  Map map;
  T* /*const*/ data;

 private:
  void init(T* data_) {
    if (data_) {
      data = data_;
    }
    else {
      ptr_ = std::make_unique<T[]>(map.numel);
      data = ptr_.get();
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
