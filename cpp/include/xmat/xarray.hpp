/*


i     ndim  numel shape
----- ----- ----- -----
0     c     c     c
1     c     c     r
2     c     r     c
3     c     r     r
4     r     c     c
5     r     c     r
6     r     r     c
7     r     r     r


*/

#pragma once

#include <cstddef>
#include <cassert>
#include <cmath>
#include <complex>
#include <iostream>
#include <iomanip>
#include <array>
#include <vector>
#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <memory>
#include <type_traits>

#include "xutil.hpp"


namespace xmat {

enum class Order: char {
  C = 'C',    // C-language style order: by rows
  F = 'F'     // Fortran style order:    by columns
};


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


namespace details {
template<typename T0, typename T1>
T0 conv_impl(const T0* a, const T1 index) { return *a * static_cast<T0>(index); }

template<typename T0, typename T1, typename ... Args>
T0 conv_impl(const T0* a, const T1 index, Args ... args) {
  auto tmp = conv_impl(a, index);
  return  tmp + conv_impl(/*???*/++a, args...);
}
} // namespace details

template<szt N, typename T, typename ... Args>
std::enable_if_t<(sizeof ... (Args)) <= N, T>
/*T*/ conv(const std::array<T, N>& a, Args ... args) {
  return details::conv_impl(a.data(), args ...);
}

template<typename Iter0, typename Iter1, typename T>
T conv(Iter0 first0, Iter0 last0, Iter1 first1, T inital) {
  for (; first0 != last0; ++first0, ++first1) { inital += *first0 * *first1; }
  return inital;
}

template <typename T, std::size_t N>
constexpr T cumprod(T const (&a)[N], std::size_t i = 0U) { 
  return i < N ? (a[i] * cumprod(a, i+1U)) : T{1};
}


// run-time shape
template<szt ND>
struct Index : std::array<szt, ND> {
  static const szt ndim = ND;
  using base_t = std::array<szt, ND>;
  using base_t::data;
  using base_t::begin;
  using base_t::cbegin;
  using base_t::end;
  using base_t::cend;
  using base_t::fill;

  Index() { /*???*/ fill(0); }

  template<typename Iter>
  Index(Iter first, szt n) noexcept {assert(n == ndim); fill(first, n);}

  Index(const std::array<szt, ND>& arr) noexcept : Index(arr.begin(), arr.size()) {}
  Index(const Index& other) noexcept : Index(other.begin(), other.size()) {}
  Index(std::initializer_list<szt> iis) noexcept : Index(iis.begin(), iis.size()) {}

  Index& operator=(const Index& other) noexcept {
    fill(other.begin(), other.size());
    return *this;
  }

  Index& operator=(std::initializer_list<szt> iis) noexcept {
    fill(iis.begin(), iis.size());
    return *this;
  }

  template<typename Iter>
  void fill(Iter first, szt n) noexcept {
    n = std::min(n, ND);
    std::fill(std::copy_n(first, n, begin()), end(), 0);
  }

  bool operator==(const Index& other) const noexcept {
    return std::equal(cbegin(), cend(), other.cbegin(), other.cend());
  }

  bool operator!=(const Index& other) const noexcept { return !(*this == other); }

  // props
  // -----
  szt numel() const noexcept {
    szt size = 1;
    for (auto item : (*this)) { size *= item; }
    return size;
  }

  // for dynamic ndim. just in case
  // ------------------------------
  // return: true - [x, x, ... x, 0, 0, ..., 0],  false - [x, x, 0, x, ... ]
  bool check() const noexcept {
    auto out = std::find(begin(), end(), 0);
    return std::all_of(out, end(), [](szt i){return i == 0;});
  }

  // return first nonzero element index
  szt top() const noexcept {
    auto out = std::find(begin(), end(), 0);
    return std::distance(begin(), out);
  }

  szt topnumel() const noexcept {
    szt size = 1;
    for (auto iter_ = begin(), end_ = end(); (iter_ != end_) && (*iter_ != 0); ++iter_) {
      size *= *iter_;
    }
    return size;
  }
};


// compile-time Index
template<szt ... Args>
struct Shape {
  static const szt ndim = sizeof ... (Args);
  static constexpr const szt shape[] = {Args ...};
  static const szt numel = cumprod(shape);
  
  using index_t = Index<ndim>;
  static index_t index() {return index_t{Args ...}; }
};


struct SliceArg {
  SliceArg(szt i) : i{i} {}
  szt i = 0;
};

struct Slice { 
  Slice(szt start=0, szt stop=0, szt step=1) noexcept
    : start{start}, stop{stop}, step{step} {}

  szt start = 0;
  szt stop = 0;
  szt step = 1;

  szt at(szt i) const noexcept { return start + i * step; }
};


template<class C1, class C2>
szt init_stride_order_c(const C1& shape, C2& stride, szt Nd) {
  stride[Nd - 1] = 1;
  for (szt i = Nd - 2, end_(-1); i != end_; --i) {
    stride[i] = stride[i + 1] * shape[i + 1];
  }
  return Nd;
}

template<class C1, class C2>
szt init_stride_order_f(const C1& shape, C2& stride, szt Nd) {
  stride[0] = 1;
  for (uint i = 1; i != Nd; ++i) {
    stride[i] = stride[i - 1] * shape[i - 1];
  }
  return Nd;
}


// \brief Converts a tuple of index arrays into an array of flat indices,
// applying boundary modes to the multi-index.
// \see numpy.ravel_multi_index, numpy.unravel_index
template<szt ND>
struct Ravel {
  static_assert(ND > 0, "aaa");

  using index_t = Index<ND>;
  static const szt ndim = index_t::ndim;
  static szt size() { return ndim; }
  static const szt iter_ndim = ND-1;

  Ravel() = default;
  Ravel(index_t shape, Order order=Order::C) : shape{shape}, order{order} { init(); }

  szt at(index_t idx) const noexcept {
    return origin + conv(idx.begin(), idx.end(), stride.begin(), szt{0});
  }

  template<typename ... Args>
  std::enable_if_t<(sizeof ... (Args)) == ND, std::size_t>
  /*std::size_t*/ at(Args ... args) const {
    return origin + conv(stride, args...);
  }

  Ravel slice(std::array<Slice, ndim> s) const noexcept {
    Ravel out;
    index_t idx;
    for (szt n = 0; n < ndim; ++n) {
      Slice& s_ = s[n];
      idx[n] = s_.start;
      out.stride[n] = s_.step * stride[n];
      assert(s_.stop >= s_.start);
      // out.shape[n] = s_.stop >= s_.start ? s_.stop - s_.start : s_.start - s_.stop;
      out.shape[n] = s_.stop >= s_.start ? s_.stop - s_.start : 0;
      out.shape[n] /= s_.step;
    }
    out.origin = at(idx);
    out.order = order;
    return out;
  }

  Ravel<iter_ndim> slice_begin() noexcept {
    // iter_ndim = ND - 1
    Ravel<iter_ndim> out;
    for (szt n = 0; n < iter_ndim; ++n){
      out.shape[n] = shape[n+1];
      out.stride[n] = stride[n+1];
    }
    out.origin = at(index_t{});
    out.order = order;
    return out;
  }

  template<szt N>
  std::enable_if_t<(N < ND), Ravel<N>>
  /*Ravel<N>*/ squeese() const {
    Ravel<N> out;
    szt k = -1;
    for (szt n = 0; n < ndim; ++n) {
      if(shape[n] == 1 ) { continue; }
      ++k;
      assert(k < N);
      out.shape[k] = shape[n];
      out.stride[k] = stride[n];
    }
    assert(k == N-1); //
    out.origin = origin;
    out.order = order;
    return out;
  }

  template<szt N>
  Ravel<N> reshape(Index<N> shape_new) const {
    assert(numel() == shape_new.numel());
    Ravel<N> out{shape_new, order};
    out.origin = origin;
    return out;
  }

  // properties
  // ----------
  // ???
  void set_order(Order order_new) noexcept {
    if (order_new != order) {
      order = order_new;
      init();
    }
  }

  szt numel() const noexcept { return shape.numel(); }

  szt origin = 0;
  index_t shape;
  index_t stride;
  Order order = Order::C;

  void init() {
    auto _ = order == Order::C ? 
        init_stride_order_c(shape, stride, ndim) :
        init_stride_order_f(shape, stride, ndim);
  }
};


template<typename T, szt ND> struct View;

template<typename T, szt ND>
struct Iterator {
  static const szt ndim = ND;
  using value_type = View<T, ND>;
  using pointer = value_type*;
  using reference = value_type&;

  using ravel_t = Ravel<1>;
  
  Iterator() = default;

  View<T, ND>* view = nullptr;
  szt delta = 0;
};


template<typename T>
struct Iterator<T, 1> {
  static const szt ndim = 1;
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using ravel_t = Ravel<1>;

  Iterator() = default;

  Iterator(T* data, ravel_t ravel) : data_{data + ravel.origin}, delta_{ravel.stride[0]} { }

  Iterator(T* data, ravel_t ravel, bool end_) : delta_{ravel.stride[0]} {
    data_ = data + ravel.origin + ravel.shape[0] * delta_;
  }
  
  T& operator*() const { return *data_; }
  T* operator->() {return data_; };

  Iterator operator++() { 
    data_ += delta_;
    return *this;
  }
  
  Iterator operator++(int) {
    Iterator tmp = *this;
    ++(*this);
    return tmp;
  }

  bool operator==(const Iterator& other) const { return data_ == other.data_; }
  bool operator!=(const Iterator& other) const { return data_ != other.data_; }

  T* data_ = nullptr;
  szt delta_ = 0;
};


template<typename T, szt ND>
struct View {
  using ravel_t = Ravel<ND>;
  using index_t = Index<ND>;
  static const szt iter_ndim = ND-1;

  View() = default;
  View(T* data, ravel_t ravel) : data_{data}, ravel_{ravel} {}

  // index element access
  // --------------------
  T& at(const index_t& ii) noexcept { return data_[ravel_.at(ii)]; }

  const T& at(const index_t& ii) const noexcept { return data_[ravel_.at(ii)]; }

  template<typename ... Args>
  std::enable_if_t<(sizeof...(Args)) == ND ,T&>
  /*T&*/ at(Args ... args) { return data_[ravel_.at(args...)]; }

  template<typename ... Args>
  std::enable_if_t<(sizeof...(Args)) == ND , const T&>
  /*const T&*/ at(Args ... args) const { return data_[ravel_.at(args...)]; }

  // slice
  // ---------------
  View slice(std::array<Slice, ND> s) noexcept {
    auto rv = ravel_.slice(s);
    return {data_, rv};
  }

  template<szt N>
  View<T, N> reshape(Index<N> shape_new) noexcept {
    assert(numel() == shape_new.numel());
    Ravel<N> rv = ravel_.reshape(shape_new);
    return View<T, N>{data_, rv};
  }

  template<szt N>
  View<T, N> squeese() noexcept {
    Ravel<N> rv = ravel_.template squeese<N>();
    return View<T, N>{data_, rv};
  }

  // iterators
  // ---------
  Iterator<T, ND> begin() noexcept { return Iterator<T, ND>{data_, ravel_}; }
  Iterator<T, ND> end() noexcept { return Iterator<T, ND>{data_, ravel_, true}; }

  // getters
  //-------------
  T* data() noexcept { return data_; }
  const T* data() const noexcept { return data_; }

  const index_t& shape() const noexcept { return ravel_.shape; }
  const index_t& stride() const noexcept { return ravel_.stride; }
  szt numel() const noexcept { return ravel_.numel(); }
  static szt ndim() noexcept { return ravel_t::ndim; }

  Order order() const noexcept { return ravel_.order; }
  bool is_order_c() const noexcept { return ravel_.order == Order::C; }
  bool is_order_f() const noexcept { return ravel_.order == Order::F; }

  T* data_ = nullptr;
  ravel_t ravel_;
};


// Array
// ---------------------------------------------------
template<typename T>
struct ArrayStorageDynamic : std::vector<T> {
  using base_t = std::vector<T>;
  template<typename U> using cast_t = ArrayStorageDynamic<U>;
  static constexpr const char* tagstr = "dynamic";
  ArrayStorageDynamic() {}
  ArrayStorageDynamic(std::size_t n) : base_t(n) {}
  using base_t::data;
};

template<typename T, std::size_t N>
struct ArrayStorageStatic : std::array<T, N> {
  using base_t = std::array<T, N>;
  template<typename U> using cast_t = ArrayStorageStatic<U, N>;
  static constexpr const char* tagstr = "static";
  ArrayStorageStatic() { }
  ArrayStorageStatic(std::size_t n) { assert(n <= N); }
  using base_t::data;
};


template<typename T, szt ND, class StorageT = ArrayStorageDynamic<T>>
class Array {
 public:
  using ravel_t = Ravel<ND>;
  using index_t = typename ravel_t::index_t;
  using storage_t = StorageT;
  using view_t = View<T, ND>;

  template<typename U>
  using cast_t = Array<U, ND, typename storage_t::cast_t<U>>;

  Array() = default;

  Array(index_t shape) : ravel_{shape}, storage_(ravel_.numel()) {}

  void swap(Array& other) noexcept {
    using std::swap;
    swap(ravel_, other.ravel_);
    swap(storage_, other.storage_);
  }

  friend void swap(Array& lhs, Array& rhs) noexcept { lhs.swap(rhs); }

  View<T, ND> view() & noexcept { return View<T, ND>{storage_.data(), ravel_}; }

  View<const T, ND> view() const & noexcept { return View<const T, ND>{storage_.data(), ravel_}; }

  template<typename U>
  cast_t<U> cast() const {
    cast_t<U> arr{shape()};
    auto item = this->data();
    auto item_arr = arr.data();
    for (uint n = 0, N = numel(); n < N; ++n, ++item, ++item_arr) {
      *item_arr = xmat::cast<U>(*item);
    }
    return arr;
  }

  // element access
  // --------------
  T& at(const index_t& ii) noexcept { return storage_[ravel_.at(ii)]; }
  const T& at(const index_t& ii) const noexcept { return storage_[ravel_.at(ii)]; }

  template<typename ... Args>
  std::enable_if_t<(sizeof...(Args)) == ND ,T&>
  /*T&*/ at(Args ... args) noexcept { return storage_[ravel_.at(args...)]; }

  template<typename ... Args>
  std::enable_if_t<(sizeof...(Args)) == ND ,const T&>
  /*const T&*/ at(Args ... args) const noexcept { return storage_[ravel_.at(args...)]; }

  // operator()
  // ----------
  T& operator()(const index_t& ii) noexcept { return at(ii); }
  const T& operator()(const index_t& ii) const noexcept { return at(ii); }

  template<typename ... Args> std::enable_if_t<(sizeof...(Args)) == ND ,T&> 
  /*T&*/ operator()(Args ... args) noexcept { return at(args...); }

  template<typename ... Args> std::enable_if_t<(sizeof...(Args)) == ND ,const T&>
  /*const T&*/ operator()(Args ... args) const noexcept { return at(args...); }


  void fill(T a) {std::fill_n(data(), numel(), a); }

  void fill(T start, const T delta) {
    // for(auto& item : storage_) {
    for (T* item = data(), *end = data() + ravel_.numel(); item != end; ++item) {
      *item = start;
      start += delta;
    }
  }

  void arange() { fill(T{0}, T{1}); }
  void enumerate() { arange(); }

  // access
  T* data() noexcept { return storage_.data(); }
  const T* data() const noexcept { return storage_.data(); }

  const index_t& shape() const noexcept { return ravel_.shape; }
  const index_t& stride() const noexcept { return ravel_.stride; }
  szt numel() const noexcept { return ravel_.numel(); } 
  static szt ndim() noexcept { return ND; }

  Order order() const noexcept { return ravel_.order; }
  bool is_order_c() const noexcept { return ravel_.order == Order::C; }
  bool is_order_f() const noexcept { return ravel_.order == Order::F; }

  static const char* storage_tag() { return storage_t::tagstr; }

 public:
  ravel_t ravel_;
  storage_t storage_;
};


// array - make functions
// ----------------------
template<typename T, szt ND, typename U>
Array<T, ND, ArrayStorageDynamic<T>>
array_dynamic(const U(&ii)[ND]) {
  Index<ND> idx{std::begin(ii), ND};
  return Array<T, ND, ArrayStorageDynamic<T>>{idx};
}

template<typename T, szt ... Args>
Array<T, Shape<Args...>::ndim, ArrayStorageStatic<T, Shape<Args...>::numel>>
array_static() {
  using shape_t = Shape<Args...>;
  return Array<T, shape_t::ndim, ArrayStorageStatic<T, shape_t::numel>>{shape_t::index()};
}


// print functions
// ------------------------
inline std::ostream& operator<<(std::ostream& os, const xmat::Slice& s) {
  os << "["<< s.start <<  ", " << s.stop <<  ", " <<  s.step <<  "]";
  return os;
}

template<typename T, szt ND>
struct PrintView {
  static void print_(std::ostream& os, View<T, ND> x, szt space) {
    os << "print " << ND << "D xarray";
  }
};

template<typename T>
struct PrintView <T, 1> {
  static const szt ND = 1;
  static void print_(std::ostream& os, View<T, ND> x, szt space) {
    for (uint i = 0, N0 = x.shape()[0]; i < N0; ++i) {
        if (space) os << std::setw(space);
      os << x.at(i) << ", ";
    }
  }
};


template<typename T>
struct PrintView <T, 2> {
  static const szt ND = 2;
  static void print_(std::ostream& os, View<T, ND> x, szt space) {
    for (uint i0 = 0, N0 = x.shape()[0]; i0 < N0; ++i0) {
      for (uint i1 = 0, N1 = x.shape()[1]; i1 < N1; ++i1) {
        if (space) os << std::setw(space);
      os << x.at(i0, i1) << ", ";
    }
    os << "\n";
    }
  }
};

template<typename T, szt ND>
std::ostream& print(std::ostream& os, View<T, ND> x, szt space=4) {
  os << "view" << ND << "d\n";
  PrintView<T, ND>::print_(os, x, space);
  return  os;
}

template<typename T, szt ND, class S>
std::ostream& print(std::ostream& os, const Array<T, ND, S>& x, szt space=4) {
  os << "array" << ND << "d\n";
  PrintView<const T, ND>::print_(os, x.view(), space);
  return  os;
}

template<typename T, szt ND>
std::ostream& operator<<(std::ostream& os, View<T, ND> x) {
  return print(os, x, 4);
}

template<typename T, szt ND, class S>
std::ostream& operator<<(std::ostream& os, const Array<T, ND, S>& x) {
  return print(os, x, 4);
}
} // namespace xmat
