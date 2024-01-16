#pragma once

#include <cstddef>
#include <cassert>
#include <cmath>
#include <array>
#include <initializer_list>
#include <algorithm>
#include <memory>


namespace xmat {

using uint = std::size_t;   // just to have short name
const uint kMaxNDim = 4;

enum class Order: char {
  C = 'C',    // C-language style order: by rows
  F = 'F'     // Fortran style order:    by columns
};


using IndexBase = std::array<uint, kMaxNDim>;

class Index : public IndexBase {
 public:
  
  using IndexBase::IndexBase;

  Index(const IndexBase& arr) { static_cast<IndexBase&>(*this) = arr; }

  Index(std::initializer_list<uint> shape) {
    fill(0);
    const uint n0 = shape.size();
    assert(n0 <= kMaxNDim);
    const uint n1 = std::min(n0, kMaxNDim);
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

  Map() : ndim(0) { init(); }
  Map(const Index& shape) : ndim(shape.ndim()), shape(shape) { init(); }
  Map(uint sz0) : Map(Index({sz0})) { }
  Map(uint sz0, uint sz1) : Map(Index({sz0, sz1})) { }
  Map(uint sz0, uint sz1, uint sz2) : Map(Index({sz0, sz1, sz2})) { }

  uint at(const Index& idx) const {
    uint I = 0;
    for (uint n = 0; n < kMaxNDim; ++n) {
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

} // namespace xmat
