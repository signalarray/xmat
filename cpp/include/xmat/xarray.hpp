#pragma once

#include <cstddef>
#include <cassert>
#include <cmath>

#include <iostream>
#include <iomanip>

#include <array>
#include <vector>
#include <complex>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <utility>
#include <type_traits>


#include "xutil.hpp"
#include "xmemory.hpp"


namespace xmat {

// Row- and column-major order
enum class MOrder: char {
  C = 'C',     // C-language style order: row-major order
  F = 'F'      // Fortran style order:    column-major order
};


namespace hidx { // help index functions namespace

template<typename It0, typename It1, typename T2>
T2 conv(It0 first0, It1 first1, size_t n, T2 inital) {
  for (; n != 0; --n, ++first0, ++first1) { 
    inital += *first0 * *first1; 
  }
  return inital;
}

template<size_t N, typename T0, typename T1, typename T2>
T2 conv(T0* first0, T1* first1, T2 inital) {
  for (size_t n = 0; n < N; ++n, ++first0, ++first1) {
    inital += *first0 * *first1; 
  }
  return inital;
}

template<typename T0, typename T1, typename T2>
T2 rconv(T0* first0, T1* first1, size_t n, T2 inital) {
  for (first0 += n-1; n != 0; --n, --first0, ++first1) {
    inital += *first0 * *first1; 
  }
  return inital;
}

template<size_t N, typename T0, typename T1, typename T2>
T2 rconv(T0* first0, T1* first1, T2 inital) {
  first0 += N-1;
  for (size_t n = 0; n < N; ++n, --first0, ++first1) {
    inital += *first0 * *first1; 
  }
  return inital;
}

// most data-local dimention
constexpr size_t morderlowi(MOrder morder, size_t ndim) noexcept {
  return morder == MOrder::C ? ndim - 1 : 0;
}

// least data-local dimention
constexpr size_t morderhidhi(MOrder morder, size_t ndim) noexcept {
  return morder == MOrder::C ? 0 : ndim - 1;
}

// MOrder::C
template<MOrder MOrderT, class RandomIt1, class RandomIt2,
         typename std::enable_if_t<MOrderT == MOrder::C, int> = 0>
void stride(RandomIt1 shape, RandomIt2 out, size_t ndim) {
#ifdef XMAT_HIDX_STRIDE_0
  out[Nd - 1] = 1;
  for (size_t i = Nd - 2, end_(-1); i != end_; --i) { 
    out[i] = out[i + 1] * shape[i + 1]; 
  }
#else
  auto shape_p = shape + ndim - 1;
  out += ndim - 1;
  *out = 1;
  auto out_p = out--;
  for (; shape_p != shape; --shape_p, --out, --out_p) {
    *out = *out_p * *shape_p;
  }
#endif
}

// MOrder::F
template<MOrder MOrderT, class RandomIt1, class RandomIt2,
         typename std::enable_if_t<MOrderT == MOrder::F, int> = 0>
void stride(RandomIt1 shape, RandomIt2 out, size_t ndim) {
  *out = 1;
  auto out_p = out++;
  for (auto end = shape + ndim - 1; shape != end; ++shape, ++out, ++out_p) {
    *out = *out_p * *shape;
  }
}

// MOrder::C
template<MOrder MOrderT, size_t ND, class RandomIt1, class RandomIt2,
         typename std::enable_if_t<MOrderT == MOrder::C, int> = 0>
void stride(RandomIt1 shape, RandomIt2 out) { return stride<MOrderT>(shape, out, ND); }

// MOrder::F
template<MOrder MOrderT, size_t ND, class RandomIt1, class RandomIt2,
         typename std::enable_if_t<MOrderT == MOrder::F, int> = 0>
void stride(RandomIt1 shape, RandomIt2 out) { return stride<MOrderT>(shape, out, ND); }

// MOrder::C
template<MOrder MOrderT, class RandomIt0, class RandomIt1, class RandomIt2,
         typename std::enable_if_t<MOrderT == MOrder::C, int> = 0>
auto increment(size_t idx, size_t nd, size_t ndim,
               RandomIt0 index, RandomIt1 shape, RandomIt2 stride) {
  assert(nd < ndim && "xmat::hidx::increment<>. nd must be less than ND");
#ifdef XMAT_INCREMENT_IMPL_0
  size_t nd0 = nd;
  index[nd] += 1;
  for (; index[nd] == shape[nd] && nd > 0; --nd) {
    index[nd] = 0;
    ++index[nd-1];
  }
  const size_t s = stride[nd];
  idx = nd == nd0 ? idx + s : idx + s - idx % s;
  return idx;
#else
  size_t dd;
  index += nd;
  shape += nd;
  stride += nd;
  *index += 1;
  if (*index != *shape) {
    dd = *stride;
  } else {
    do {
      *index = 0;
      *(index-1) += 1;
      --nd, --index, --shape, --stride;
    } while (*index == *shape && nd > 0);
    dd = *stride - idx % *stride;
  }
  return dd;
#endif
}


// MOrder::F
template<MOrder MOrderT, class RandomIt0, class RandomIt1, class RandomIt2,
         typename std::enable_if_t<MOrderT == MOrder::F, int> = 0>
auto increment(size_t idx, size_t nd, size_t ndim,
               RandomIt0 index, RandomIt1 shape, RandomIt2 stride) {
  assert(nd < ndim && "xmat::hidx::increment<>. nd must be less than ND");
#ifdef XMAT_INCREMENT_IMPL_0
  size_t nd0 = nd;
  index[nd] += 1;
  for (; index[nd] == shape[nd] && nd > 0; --nd) {
    index[nd] = 0;
    ++index[nd-1];
  }
  const size_t s = stride[nd];
  idx = nd == nd0 ? idx + s : idx + s - idx % s;
  return idx;
#else
  size_t dd;
  *index += 1;
  if (*index != *shape) {
    dd = *stride;
  } else {
    nd = ndim - nd;
    do {
      *index = 0;
      *(index+1) += 1;
      --nd, ++index, ++shape, ++stride;
    } while (*index == *shape && nd > 1);
    dd = *stride - idx % *stride;
  }
  return dd;
#endif
}

// MOrder::C
template<MOrder MOrderT, class RandomIt0, class RandomIt1, class RandomIt2,
         typename std::enable_if_t<MOrderT == MOrder::C, int> = 0>
auto increment(size_t idx, size_t ndim, RandomIt0 index, RandomIt1 shape, RandomIt2 stride) {
  return increment<MOrderT>(idx, ndim-1, ndim, index, shape, stride);
}

// MOrder::F
template<MOrder MOrderT, class RandomIt0, class RandomIt1, class RandomIt2,
         typename std::enable_if_t<MOrderT == MOrder::F, int> = 0>
auto increment(size_t idx, size_t ndim, RandomIt0 index, RandomIt1 shape, RandomIt2 stride) {
  return increment<MOrderT>(idx, 0, ndim, index, shape, stride);
}


// unravel
// -------
// MOrder::C
template<MOrder MOrderT, typename T0, typename T1,
         typename std::enable_if_t<MOrderT == MOrder::C, int> = 0>
void unravel(size_t i, T0* stride, T1* out, size_t ndim) {
  for (auto it = out, end = out + ndim; i && it != end; ++it, ++stride) {
    *it = i / *stride;
    i %= *stride;
  }
}

template<MOrder MOrderT, typename T0,
         typename std::enable_if_t<MOrderT == MOrder::C, int> = 0>
size_t unraveli(size_t i, T0* stride, size_t ndim) {
  size_t i2 = 0;
  for (auto end = stride + ndim; i && stride != end; ++stride) {
    auto stride_ = *stride;
    i2 += (i / stride_) * stride_;
    i %= stride_;
  }
  return i2;
}

// MOrder::F
template<MOrder MOrderT, typename T0, typename T1,
         typename std::enable_if_t<MOrderT == MOrder::F, int> = 0>
void unravel(size_t i, T0* stride, T1* out, size_t ndim) {
  stride += ndim - 1;
  for (auto it = out + ndim - 1, end = out-1; i && it != end; --it, --stride) {
    *it = i / *stride;
    i %= *stride;
  }
}

// MOrder::C
template<MOrder MOrderT, typename T0,
         typename std::enable_if_t<MOrderT == MOrder::F, int> = 0>
size_t unraveli(size_t i, T0* stride, size_t ndim) {
  size_t i2 = 0;
  stride += ndim - 1;
  for (auto end = stride - ndim ; i && stride != end; --stride) {
    auto stride_ = *stride;
    i2 += (i / stride_) * stride_;
    i %= stride_;
  }
  return i2;
}

// MOrder::C
template<MOrder MOrderT, typename T0, typename T1,
         typename std::enable_if_t<MOrderT == MOrder::C, int> = 0>
size_t ndcontigous(const T0* shape, const T1* stride, size_t ndim) {
  auto shape_it = shape + ndim - 1;
  stride += ndim - 1;
  auto expect = *stride;
  for (shape -= 1;
       *stride == expect && shape_it != shape;
       --shape_it, --stride) {
    expect *= *shape_it;
  }
  return ndim - (shape_it - shape);
}

// MOrder::F
template<MOrder MOrderT, typename T0, typename T1,
         typename std::enable_if_t<MOrderT == MOrder::F, int> = 0>
size_t ndcontigous(const T0* shape, const T1* stride, size_t ndim) {
  auto shape_it = shape;
  auto expect = *stride;
  for (shape += ndim;
       *stride == expect && shape_it != shape;
       ++shape_it, ++stride) {
    expect *= *shape_it;
  }
  return ndim - (shape - shape_it);
}


template<typename T>
bool isunique(const T* begin, const T* end) {
  auto it = begin;
  for (auto end_ = end - 1; it != end_; ++it) {
    for (auto it2 = it + 1; it2 != end; ++it2) { 
      if (*it2 == *it) {
        return false;
      }
    }
  }
  return true;
}

// due to be shure in not using heap memory
template<typename T, typename Compare = std::less<T>>
void booblesort(T* begin, T* end, Compare comp = Compare{}) {
  using std::swap;
  auto end_ = end - 1;
  for (auto it = begin; it != end_; ++it) {
    for (auto it2 = it + 1; it2 != end; ++it2) {
      if (comp(*it2, *it)) {
        // auto tmp = *it; *it = *it2; *it2 = tmp;
        swap(*it, *it2); 
      }
    }
  }
}

// std::set_difference is just since C++17
template<typename T0, typename T1, typename T2>
T2* setdiff(const T0* a_begin, const T0* a_end,
             const T1* b_begin, const T1* b_end,
             T2* out_begin, T2* out_end = nullptr)
{
  size_t num = 0;
  for (auto a_it = a_begin; a_it != a_end && out_begin != out_end; ++a_it) {
    bool contains = false;
    for (auto b_it = b_begin; b_it != b_end; ++b_it) {
      if (*a_it == *b_it) {
        contains = true;
        break;
      }
    }
    if (!contains) {
      assert(out_begin != out_end && "exceeds bounds");
      *(out_begin++) = *a_it; 
    }
  }
  return out_begin;
}

} // namespace hidx


// Slicing
// -------
enum class SlTag {
  all,      // alone
  newaxis,  // alone
  dropaxis, // alone
  undef,  
};


struct Slice {

  constexpr Slice(SlTag flag = SlTag::all) : tag{flag} {}

  constexpr Slice(ptrdiff_t start, ptrdiff_t stop, size_t step = 1)
  : start{start}, stop{stop}, step{step} { assert(start <= stop); }

  SlTag tag = SlTag::undef;
  ptrdiff_t start = 0;
  ptrdiff_t stop = 0;
  size_t step = 1;
};


// index: [0, 2, 3, 4, 5, 6, 7], -> N = 8
struct sl {
  // self-contained tags
  // -------------------
  static constexpr Slice all{SlTag::all};
  static constexpr Slice nax{SlTag::newaxis};
  static constexpr Slice newaxis{SlTag::newaxis};

  // index-dependent tags
  // --------------------
  // ilen(3, 2) -> [3+0 := 3, 3+1 := 2]
  // ilen(-3, 2) -> [8-3+0 := 5, 8-3+1 := 6]
  // ilen(-3, 3) -> [8-3+0 := 5, 8-3+1 := 6, 8-3+2 := 7]
  static Slice ilen(ptrdiff_t i, ptrdiff_t len, size_t step = 1) { return {i, i + len, step}; }
};

struct sl_ {
  static Slice all() noexcept { return SlTag::all; }
  static Slice nax() noexcept { return SlTag::newaxis; }
  static Slice newaxis() noexcept { return SlTag::newaxis; }
  static Slice ilen(ptrdiff_t i, ptrdiff_t len, size_t step = 1) { return {i, i + len, step}; }
};

template<size_t ND> using NSlice = std::array<Slice, ND>;


// Indexing
// --------
template<typename IntT, size_t ND>
struct Index_ final {
  using value_type = IntT;
  static const IntT ndim = ND;

  // std::array<> like methods
  // -------------------------
  IntT& operator[](size_t i) noexcept { assert(i < ND); return a_[i]; }

  const IntT& operator[](size_t i) const noexcept { assert(i < ND); return a_[i]; }

  IntT& at(IntT i) {
    if (i > ND) throw std::overflow_error("Index::at()");
    return a_[i]; 
  }

  const IntT& at(IntT i) const { 
    if (i > ND) throw std::overflow_error("Index::at() const ");
    return a_[i]; 
  }
  
  IntT* begin() noexcept { return a_; }

  const IntT* begin() const noexcept { return a_; }

  const IntT* cbegin() const noexcept { return begin(); }


  IntT* end() noexcept { return a_ + ND; }

  const IntT* end() const noexcept { return a_ + ND; }

  const IntT* cend() const noexcept { return end(); }


  // Index<> specific methods
  // -------------------------
  bool operator==(const Index_& other) const noexcept {
    return std::equal(cbegin(), cend(), other.cbegin(), other.cend());
  }

  bool operator!=(const Index_& other) const noexcept { return !(*this == other); }

  IntT numel() const noexcept {
    return std::accumulate(begin(), end(), IntT{1}, std::multiplies<IntT>());
  }

  // filling content
  Index_& fill(IntT a) noexcept { std::fill(begin(), end(), a); return *this; }

  template<typename U> // not welcome
  Index_& fill(const U* ptr) noexcept { std::copy_n(ptr, ND, begin()); return *this; }

  template<typename U>
  Index_& fill(const U* ptr, size_t nend) noexcept { 
    std::fill(std::copy_n(ptr, nend, begin()), end(), IntT{0});
    return *this; 
  }

  template<size_t N, typename U, typename std::enable_if_t<N <= ND, int> = 0>
  Index_& fill(const U* ptr) noexcept { 
    std::fill(std::copy_n(ptr, N, begin()), end(), IntT{0});
    return *this; 
  }

  IntT a_[ND] = {};
};

template<size_t ND> using Index = Index_<size_t, ND>;


template<size_t ND, MOrder MOrderT, typename IntT = size_t>
struct Increment_ {
  static const size_t ndim = ND;
  using index_t = Index_<IntT, ND>;

  Increment_() = default;

  Increment_(const index_t& shape) : shape{shape} {
    hidx::stride<MOrderT, ND>(shape.cbegin(), stride.begin());
  }

  Increment_(const index_t& shape, const index_t& stride) : shape{shape}, stride{stride} { }

  // required reset before using
  void to_end() noexcept {
    // index.fill(0);
    const size_t nd = MOrderT == MOrder::C ? 0 : ND - 1;
    index[nd] = shape[nd];
    fidx = stride[nd] * index[nd];
  }

  void reset() noexcept { fidx = 0; index.fill(0); }

  void next() noexcept {
    fidx += hidx::increment<MOrderT>(fidx, ND, index.begin(), shape.begin(), stride.begin());
  }

  size_t nextd() noexcept {
    auto dd = hidx::increment<MOrderT>(fidx, ND, index.begin(), shape.begin(), stride.begin());
    fidx += dd;
    return dd;
  }

  void next(size_t nd) noexcept {
    fidx += hidx::increment<MOrderT>(fidx, nd, ND, index.begin(), shape.begin(), stride.begin());
  }

  size_t nextd(size_t nd) noexcept {
    auto dd = hidx::increment<MOrderT>(fidx, nd, ND, index.begin(), shape.begin(), stride.begin());
    fidx += dd;
    return dd;
  }

  index_t shape;
  index_t stride;
  index_t index;
  size_t fidx = 0;   // flat index
};

template<size_t ND, MOrder MOrderT> using Increment = Increment_<ND, MOrderT>;
template<size_t ND> using IncrementC = Increment_<ND, MOrder::C>;
template<size_t ND> using IncrementF = Increment_<ND, MOrder::F>;


// for indexing variadic arguments
// stackoverflow.com/questions/13636290
// check if all args 
template <typename...> struct AllType;

template <typename Type> struct AllType<Type> : std::true_type { };

template <typename Type, typename T, typename ...Rest> 
struct AllType<Type, T, Rest...> : std::integral_constant<bool, 
  std::is_convertible<T, Type>::value && AllType<Type, Rest...>::value> { };

template<size_t N, typename Type, typename ... Args>
struct AllNType {
  static constexpr bool value = (sizeof ... (Args) == N) && AllType<Type, Args...>::value;
};

template<size_t ND, MOrder MOrderT, typename IntT = size_t>
struct Ravel_ {
  using index_t = Index_<IntT, ND>;
  static const szt ndim = index_t::ndim;
  static szt size() { return ndim; }

  Ravel_() = default;

  Ravel_(index_t shape) : shape(shape) {
    hidx::stride<MOrderT, ND>(shape.cbegin(), stride.begin());
  }

  Ravel_(index_t shape, index_t stride) : shape{shape}, stride{stride} {}

  IntT at(const index_t& idx) const noexcept {
    return origin + hidx::conv(idx.begin(), stride.begin(), ND, IntT{0});
  }

  template<typename ... Args>
  std::enable_if_t<AllNType<ND, IntT, Args...>::value, IntT>
  /*IntT*/ at(Args ... args) const noexcept {
    std::array<IntT, ND> idx = {static_cast<IntT>(args)...};
    return origin + hidx::conv(idx.begin(), stride.begin(), ND, IntT{0});
  }

  index_t unravel(size_t i) const noexcept {
    index_t idx;
    hidx::unravel<MOrderT>(i, stride.begin(), idx.begin(), ND);
    return idx;
  }

  IntT unraveli(size_t i) const noexcept {
    return hidx::unraveli<MOrderT>(i, stride.begin(), ND);
  }

  // slicing
  // return: [index_start_delta, ravel]
  template<size_t ND_, MOrder MOrderT_ = MOrderT, typename IntT_ = IntT,
           typename std::enable_if_t<(ND_ >= ND), int> = 0>
  std::pair<IntT_, Ravel_<ND_, MOrderT_, IntT_>> view(const NSlice<ND_>& nslice) noexcept {
    Ravel_<ND_, MOrderT_, IntT_> out;
    Index_<IntT, ND> idx;

    for (size_t n_ = 0, n = 0; n_ < ND_; ++n_, ++n) {
      const auto& s_ = nslice[n_];
      if (s_.tag == SlTag::undef) {
        idx[n] = s_.start >= 0 ? s_.start : shape[n] - s_.start;
        assert(s_.stop >= s_.start);
        out.shape[n_] = s_.stop >= s_.start ? static_cast<size_t>(s_.stop - s_.start) : 0;
        out.shape[n_] /= s_.step;
        out.stride[n_] = s_.step * stride[n];
      }
      else if (s_.tag == SlTag::all) {
        assert(s_.start == 0 && s_.stop == 0 && s_.step == 1 && "supposed to be default");
        idx[n] = 0;
        out.shape[n_] = shape[n];
        out.stride[n_] = stride[n];
      }
      else if (s_.tag == SlTag::newaxis) {
        assert(s_.start == 0 && s_.stop == 0 && s_.step == 1 && "supposed to be default");
        // idx[n_] = 0;
        out.shape[n_] = 1;
        out.stride[n_] = 0;
        --n;
      } else {
        assert(false);
      }
    }
    IntT_ delta = at(idx);
    return {delta, out};
  }

  // keep: dims order doesn't matter
  template<size_t Nkeep, typename std::enable_if_t<(Nkeep < ND), int> = 0>
  Ravel_<Nkeep, MOrderT, IntT> keep(const std::array<size_t, Nkeep>& dims) {
    assert(hidx::isunique(dims.begin(), dims.end()) && "dims must be unique-values");

    // hidx::booblesort(dims.begin(), dims.end());
    Index_<IntT, Nkeep> new_shape, new_stride;
    for (size_t n = 0; n < Nkeep; ++n) {
      new_shape[n] = shape[dims[n]];
      new_stride[n] = stride[dims[n]];
    }
    return {new_shape, new_stride};
  }

  // drops: dims order doesn't matter. must be unique
  template<size_t Ndrop, typename std::enable_if_t<(Ndrop < ND), int> = 0>
  Ravel_<ND - Ndrop, MOrderT, IntT> drop(const std::array<size_t, Ndrop>& dims) {
    constexpr size_t Nkeep  = ND - Ndrop;
    std::array<size_t, ND> dkeep;
    std::iota(dkeep.begin(), dkeep.end(), 0);
    auto out0 = hidx::setdiff(dkeep.begin(), dkeep.end(), 
                              dims.begin(), dims.end(), dkeep.begin());
    assert(out0 - dkeep.begin() == Nkeep && "dims must be unique-values");

    std::array<size_t, Nkeep> keep_dims;
    std::copy_n(dkeep.begin(), Nkeep, keep_dims.begin());
    return keep(keep_dims);
  }

  // props
  size_t numel() const noexcept { return shape.numel(); }

  size_t ndcontig() const noexcept {
    return xmat::hidx::ndcontigous<MOrderT>(shape.begin(), stride.begin(), ND);
  }

  IntT leaststride() const noexcept{ return stride[hidx::morderlowi(MOrderT, ND)]; }

  size_t origin = 0;
  index_t shape;
  index_t stride;
};

template<size_t ND> using Ravel = Ravel_<ND, MOrder::C, size_t>;
template<size_t ND> using RavelC = Ravel_<ND, MOrder::C, size_t>;
template<size_t ND> using RavelF = Ravel_<ND, MOrder::F, size_t>;


// View<>
template<typename T, size_t ND> struct NIterator_ { };

template<typename T>
struct NIterator_<T, 1> {
  static const szt ndim = 1;
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using ravel_t = Ravel<1>;

  NIterator_() = default;

  NIterator_(T* ptr, size_t delta) : ptr_{ptr}, delta_{delta} {}

  T& operator*() const { return *ptr_; }
  T* operator->() {return ptr_; };

  NIterator_& operator++() { 
    ptr_ += delta_;
    return *this;
  }
  
  NIterator_ operator++(int) {
    NIterator_ tmp = *this;
    ++(*this);
    return tmp;
  }

  bool operator==(const NIterator_& other) const { return ptr_ == other.ptr_; }
  bool operator!=(const NIterator_& other) const { return ptr_ != other.ptr_; }

  T* ptr_ = nullptr;
  szt delta_ = 0;
};


// flat iterator
template<typename T, size_t ND, MOrder MOrderT, typename IntT = size_t>
struct FIterator_ {
  using increment_t = Increment_<ND, MOrderT, IntT>;
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using ravel_t = Ravel<1>;
  static const szt ndim = ND;

  FIterator_() = default;

  FIterator_(T* ptr, const increment_t& inc) noexcept : ptr_{ptr}, inc_{inc} {}

  // for end iter
  FIterator_(const increment_t& inc, bool end) noexcept : inc_{inc} { inc_.to_end(); }

  T& operator*() const { return *ptr_; }

  T* operator->() {return ptr_; };

  FIterator_& operator++() { ptr_ += inc_.nextd(); return *this; }
  
  FIterator_ operator++(int) {
    FIterator_ tmp = *this;
    ++(*this);
    return tmp;
  }

  bool operator==(const FIterator_& other) const { return inc_.fidx == other.inc_.fidx; }

  bool operator!=(const FIterator_& other) const { return inc_.fidx != other.inc_.fidx; }

 public:
  T* ptr_ = nullptr;
  Increment_<ND, MOrderT, IntT> inc_;
};


// walk iterator
template<typename T, size_t ND, MOrder MOrderT, typename IntT = size_t>
struct WIterator_ {
  using increment_t = Increment_<ND, MOrderT, IntT>;
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using index_t = Index_<IntT, ND>;
  using iterator_t = NIterator_<T, 1>;
  static const szt ndim = ND;

  WIterator_() = default;

  WIterator_(T* ptr, const increment_t& inc) noexcept 
  : ptr_{ptr}, inc_{inc} {
    // if ndcontig = 2
    // morder::C:   shape[x, x, x, x]
    //                          <-->
    // morder::F:   shape[x, x, x, x]
    //                    <-->
    ndcontig_ = xmat::hidx::ndcontigous<MOrderT>(inc_.shape.begin(), inc_.stride.begin(), ND);

    auto sbegin = inc_.shape.begin();
    auto send = inc_.shape.end();
    if (MOrderT == MOrder::C) {
      delta_ = inc_.stride[ND-1];
      sbegin += ND - ndcontig_;
      ndcontig_ = ND - ndcontig_;
    }
    else { 
      delta_ = inc_.stride[0];
      send -= ND - ndcontig_;
    }
    length_ = std::accumulate(sbegin, send, IntT{1}, std::multiplies<IntT>());

    // problem: ndcontig (in a proppper case should have `ndconfig -1` val, but it force stride len+1)
    // so: here inc_ inits in a border value:
    inc_.index[ndcontig_] = inc_.shape[ndcontig_] - 1;
    inc_.fidx = inc_.index[ndcontig_] * inc_.stride[ndcontig_];
  }

  // for end iter
  WIterator_(const increment_t& inc, bool end) noexcept : inc_{inc} { inc_.to_end(); }

  iterator_t begin() noexcept { return iterator_t{ptr_, delta_}; }

  iterator_t end() noexcept { return iterator_t{ptr_ + delta_ * length_, delta_}; }

  WIterator_& operator++() { ptr_ += inc_.nextd(ndcontig_); return *this; }

  WIterator_ operator++(int) {
    WIterator_ tmp = *this;
    ++(*this);
    return tmp;
  }
  
  bool operator==(const WIterator_& other) const { return inc_.fidx == other.inc_.fidx; }
  
  bool operator!=(const WIterator_& other) const { return inc_.fidx != other.inc_.fidx; }

  // specific
  T* data() noexcept { return ptr_; }

  const T* data() const noexcept { return ptr_; }

  IntT delta() const noexcept { return delta_; }

  IntT length() const noexcept { return length_; }

 public:
  T* ptr_ = nullptr;
  Increment_<ND, MOrderT, IntT> inc_;

  IntT delta_ = 0;
  IntT length_ = 0;
  size_t ndcontig_ = 0;
};


template<typename ViewT> struct ViewForNarray {};

template<class Derived, 
         typename T, size_t ND, MOrder MOrderT, typename IntT = size_t>
struct NArrayInterface_ {
  using value_type = T;
  using ravel_t = Ravel_<ND, MOrderT, IntT>;
  using index_t = typename ravel_t::index_t;
  using increment_t = Increment_<ND, MOrderT, IntT>;
  using fiterator_t = FIterator_<T, ND, MOrderT, IntT>;
  using cfiterator_t = FIterator_<const T, ND, MOrderT, IntT>;
  using witerator_t = WIterator_<T, ND, MOrderT, IntT>;
  using cwiterator_t = WIterator_<const T, ND, MOrderT, IntT>;
  
  static const size_t ndim = ND;

  // name using for childes: gist
  // -----------------------------
  // using typename base_t::value_type;
  // using typename base_t::ravel_t;
  // using typename base_t::index_t;
  // using typename base_t::increment_t;
  // using typename base_t::fiterator_t;
  // using typename base_t::witerator_t;
  // using base_t::ndim;

 protected:
  NArrayInterface_() = default;

 public:
  T& at(const index_t& i) noexcept { assert(ptr()); return ptr()[ravel().at(i)]; }

  const T& at(const index_t& i) const noexcept { assert(ptr()); return ptr()[ravel().at(i)]; }

  template<typename ... Args>
  std::enable_if_t<AllNType<ND, IntT, Args...>::value, T&>
  /*T&*/ at(Args ... args) noexcept { assert(ptr()); return ptr()[ravel().at(args...)]; }

  template<typename ... Args>
  std::enable_if_t<AllNType<ND, IntT, Args...>::value, const T&>
  /*const T&*/ at(Args ... args) const noexcept { assert(ptr()); return ptr()[ravel().at(args...)]; }

  // iterators
  fiterator_t fbegin() noexcept { return {ptr(), {ravel().shape, ravel().stride}}; }

  fiterator_t fend() noexcept { return {{ravel().shape, ravel().stride}, true}; }

  witerator_t wbegin() noexcept { return {ptr(), {ravel().shape, ravel().stride}}; }

  witerator_t wend() noexcept { return {{ravel().shape, ravel().stride}, true}; }

  cwiterator_t wbegin() const noexcept { return {ptr(), {ravel().shape, ravel().stride}}; }

  cwiterator_t wend() const noexcept { return {{ravel().shape, ravel().stride}, true}; }

  // 1D-begin
  template<size_t ND_ = ND, typename std::enable_if_t<(ND_ == 1), int> = 0>
  NIterator_<T, 1> begin() noexcept { return {ptr(), ravel().stride[0]}; }

  // 1D-end
  template<size_t ND_ = ND, typename std::enable_if_t<(ND_ == 1), int> = 0>
  NIterator_<T, 1> end() noexcept { 
    return {ptr() + ravel().shape[0] * ravel().stride[0], ravel().stride[0]}; 
  }
  
#if !defined _WIN32 || defined __MINGW32__
  // ND-begin
  template<size_t ND_ = ND, typename std::enable_if_t<(ND_ > 1), int> = 0>
  typename ViewForNarray<Derived>::viterator_t<ND-1, IntT>
  begin() noexcept { return {ptr(), ravel().template drop<1>({0})}; }
  
  // ND-end
  template<size_t ND_ = ND, typename std::enable_if_t<(ND_ > 1), int> = 0>
  typename ViewForNarray<Derived>::viterator_t<ND-1, IntT>
  end() noexcept { return begin().next(ravel().shape[hidx::morderhidhi(MOrderT, ndim)]); }

  // slice, view, keep, drop
  // -----------------------
  template<size_t ND_ = ND, typename IntT_ = IntT, typename std::enable_if_t<(ND_ >= ND), int> = 0>
  typename ViewForNarray<Derived>::view_t<ND_, IntT_> 
  view(const NSlice<ND_>& nslice) noexcept {
    auto rvsl = ravel().template view<ND_, MOrderT, IntT>(nslice);
    return {ptr() + rvsl.first, rvsl.second};
  }

  template<typename ... Args>
  std::enable_if_t<AllNType<ND, Slice, Args...>::value, 
  typename ViewForNarray<Derived>::view_t<sizeof...(Args), IntT>>
  view(Args...args) noexcept {
    NSlice<sizeof...(Args)> nslice{args...};
    auto rvsl = ravel().template view<sizeof...(Args), MOrderT, IntT>(nslice);
    return {ptr() + rvsl.first, rvsl.second};
  }

  template<typename ... Args>
  std::enable_if_t<AllNType<ND, Slice, Args...>::value>
  view_(Args...args) noexcept {
    Slice x[] = {args ...};
  }

  template<size_t Nkeep, typename std::enable_if_t<Nkeep < ND, int> = 0>
  typename ViewForNarray<Derived>::view_t<Nkeep, IntT> 
  keep(const std::array<size_t, Nkeep>& dims) noexcept {
    return {ptr(), ravel().template keep(dims)};
  }

  template<size_t Ndrop, typename std::enable_if_t<Ndrop < ND, int> = 0>
  typename ViewForNarray<Derived>::view_t<ND - Ndrop, IntT> 
  drop(const std::array<size_t, Ndrop>& dims) noexcept {
    return {ptr(), ravel().template drop(dims)};
  }
#endif

  // operations
  void enumerate() noexcept {
    T x{0};
    for (auto it = fbegin(), end = fend(); it != end; ++it, ++x) {
      *it = x;
    }
  }


  // getters
  // -------
  const index_t& shape() const noexcept { return ravel().shape; }

  const index_t& stride() const noexcept { return ravel().stride; }

  size_t numel() const noexcept { return ravel().numel(); }

  // access methods
  // --------------
  T* ptr() noexcept { return static_cast<Derived*>(this)->ptr_; }

  const T* ptr() const noexcept { return static_cast<const Derived*>(this)->ptr_; }

  ravel_t& ravel() noexcept { return static_cast<Derived*>(this)->ravel_; }

  const ravel_t& ravel() const noexcept { return static_cast<const Derived*>(this)->ravel_; }
};


template<typename T, size_t ND, MOrder MOrderT, typename IntT = size_t>
struct View_ : public NArrayInterface_<View_<T, ND, MOrderT, IntT>, T, ND, MOrderT, IntT> {
  using this_t = View_<T, ND, MOrderT, IntT>;
  using base_t = NArrayInterface_<this_t, T, ND, MOrderT, IntT>;
  using typename base_t::value_type;
  using typename base_t::ravel_t;
  using typename base_t::index_t;
  using typename base_t::increment_t;
  using typename base_t::fiterator_t;
  using typename base_t::witerator_t;
  using base_t::ndim;
  
  View_() = default;

  View_(T* ptr, const index_t& shape) noexcept : ptr_{ptr}, ravel_{shape} { }

  View_(T* ptr, const ravel_t& ravel) noexcept : ptr_{ptr}, ravel_{ravel} { }

  T* ptr_ = nullptr;
  ravel_t ravel_;
};

template<typename T, size_t ND> using View = View_<T, ND, MOrder::C, size_t>;
template<typename T, size_t ND> using ViewC = View_<T, ND, MOrder::C, size_t>;
template<typename T, size_t ND> using ViewF = View_<T, ND, MOrder::F, size_t>;


// is used in: NArrayInterface_<>::begin
template<typename T, size_t ND, MOrder MOrderT, typename IntT = size_t>
struct VIterator_ : public View_<T, ND, MOrderT, IntT> {
  using base_t = View_<T, ND, MOrderT, IntT>;
  using typename base_t::value_type;
  using typename base_t::ravel_t;
  using typename base_t::index_t;
  using typename base_t::increment_t;
  using typename base_t::fiterator_t;
  using typename base_t::witerator_t;
  using base_t::ndim;
  using base_t::ptr_;
  using base_t::ravel_;

  using base_t::base_t;
  
  base_t& operator*() { return *this; }

  base_t* operator->() {return this; };

  VIterator_& operator++() noexcept { return next(1); }

  VIterator_ operator++(int) noexcept {
    VIterator_ tmp = *this;
    ++(*this);
    return tmp;
  }

  VIterator_& next(size_t n = 1) noexcept {
    constexpr auto nd = hidx::morderhidhi(MOrderT, ND);
    ptr_ += n * ravel_.shape[nd] * ravel_.stride[nd];
    return *this;
  }

  bool operator==(const VIterator_& other) const { return ptr_ == other.ptr_; }
  
  bool operator!=(const VIterator_& other) const { return ptr_ != other.ptr_; }
};


template<typename T, size_t ND, MOrder MOrderT, typename IntT>
struct ViewForNarray<View_<T, ND, MOrderT, IntT>> {
  template<size_t ND_, typename IntT_> 
  using view_t = View_<T, ND_, MOrderT, IntT_>;

  template<size_t ND_, typename IntT_> 
  using viterator_t = VIterator_<T, ND_, MOrderT, IntT_>;
};


// ------------------------------------------------------------
// Array
// ------------------------------------------------------------
// Data storage for Array<T, ND>
// 
// Examples:
// ---------
// memsource* gmemsrc = &glob_memptr::reset(1 << 10);
// array_storage_ms<T, memallocator> st1{memallocator{gmemsrc}};
//
// MemSourceT examples:
// --------------------
//  std::allocator<T>
//  xmat::glob_memallocator<T, glob_memsource(default)>
//  cmat::memallocator<T>
template<typename T, class MemSourceT>
struct NArrayStorage_ {
  using memory_source_t = MemSourceT;
  
  // See: about conditional default constructor:
  // stackoverflow.com/questions/47321008/constructors-for-templates-that-do-not-have-default-constructors
  static constexpr bool k_has_default_constructor = std::is_default_constructible<MemSourceT>::value; // 

  template<bool HDC = k_has_default_constructor, typename std::enable_if<HDC, int>::type = 0>
  NArrayStorage_() {}

  NArrayStorage_(const memory_source_t& memsrc) : memsource_{memsrc} {}

  ~NArrayStorage_() {
    if(data_) {
      memsource_.deallocate(data_, N_);
    }
  }

  NArrayStorage_(const NArrayStorage_& other) 
    : memsource_{other.memsource_} {
    if (!other.N_) { return; }
    N_ = other.N_;
    data_ = memsource_.allocate(N_);
    std::copy_n(other.data_, N_, data_);
  }

  NArrayStorage_(NArrayStorage_&& other) noexcept
  : memsource_{other.memsource_}  { swap(*this, other); }

  NArrayStorage_& operator=(NArrayStorage_ other) noexcept {
    swap(*this, other);
    return *this;
  }

  friend void swap(NArrayStorage_& lhs, NArrayStorage_& rhs) {
    using std::swap;
    swap(lhs.memsource_, rhs.memsource_);
    swap(lhs.data_, rhs.data_);
    swap(lhs.N_, rhs.N_);
  }

  T* data() noexcept { return data_; }
  const T* data() const noexcept { return data_; }
  size_t size() const noexcept { return N_; }


  void init(std::size_t n) {
    N_ = n;
    data_ = memsource_.allocate(N_); // will throw exception if lack of space
  }
 
 public:
  memory_source_t memsource_;
  T* data_ = nullptr;
  size_t N_ = 0;
};


template<typename T, size_t ND, class MemSourceT, MOrder MOrderT = MOrder::C, typename IntT = size_t>
struct NArray_ : public NArrayInterface_<NArray_<T, ND, MemSourceT, MOrderT, IntT>, T, ND, MOrderT, IntT>
{
  using this_t = NArray_<T, ND, MemSourceT, MOrderT, IntT>;
  using base_t = NArrayInterface_<this_t, T, ND, MOrderT, IntT>;
  using typename base_t::value_type;
  using typename base_t::ravel_t;
  using typename base_t::index_t;
  using typename base_t::increment_t;
  using typename base_t::fiterator_t;
  using typename base_t::witerator_t;
  using base_t::ndim;
  
  using storage_t = NArrayStorage_<T, MemSourceT>;
  using view_t = View_<T, ND, MOrderT, IntT>;

  static constexpr bool k_has_default_constructor = std::is_default_constructible<MemSourceT>::value;

  // ----
  template<bool HDC = k_has_default_constructor, typename std::enable_if<HDC, int>::type = 0>
  NArray_() {}

  NArray_(const MemSourceT& memsrs) : storage_{memsrs} { }

  template<bool HDC = k_has_default_constructor, typename std::enable_if<HDC, int>::type = 0>
  NArray_(index_t shape) : NArray_(shape, MemSourceT{}) { }

  NArray_(index_t shape, const MemSourceT& memsrs) 
    : ravel_{shape}, storage_{memsrs} {
    storage_.init(shape.numel());
    ptr_ = storage_.data();
  }

  NArray_(const NArray_& ) = default;
  NArray_(NArray_&& other) : storage_{storage_} { swap(*this, other); }
  NArray_& operator=(NArray_ other) { swap(*this, other); return *this; }
  
  friend void swap(NArray_& lhs, NArray_& rhs) noexcept {
    using std::swap;
    swap(lhs.ravel_, rhs.ravel_);
    swap(lhs.storage_, rhs.storage_);
  }

 public:
  storage_t storage_;
  T* ptr_ = nullptr;
  ravel_t ravel_;
};

template<typename T, size_t ND, class MemSourceT, MOrder MOrderT, typename IntT>
struct ViewForNarray<NArray_<T, ND, MemSourceT, MOrderT, IntT>> {
  template<size_t ND_, typename IntT_> 
  using view_t = View_<T, ND_, MOrderT, IntT_>;

  template<size_t ND_, typename IntT_> 
  using viterator_t = VIterator_<T, ND_, MOrderT, IntT_>;
};


// NArray_<> aliases for different memory sources
// ----------------------------------------------
// default global allocator: std::allocator
template<typename T, size_t ND>
using NArray = NArray_<T, ND, std::allocator<T>, MOrder::C, size_t>;

template<typename T, size_t ND>
using NArrayxF = NArray_<T, ND, std::allocator<T>, MOrder::F, size_t>;

// global memory source
template<typename T, size_t ND, size_t Aln = alignof(T)>
using NArrayGMS = NArray_<T, ND, AllocatorMSGlobal<T, Aln>, MOrder::C, size_t>;

template<typename T, size_t ND, size_t Aln = alignof(T)>
using NArrayGMSxF = NArray_<T, ND, AllocatorMSGlobal<T, Aln>, MOrder::F, size_t>;

// specific memory source
template<typename T, size_t ND, size_t Aln = alignof(T)>
using NArrayMS = NArray_<T, ND, AllocatorMSRef<T, Aln>, MOrder::C, size_t>;

template<typename T, size_t ND, size_t Aln = alignof(T)>
using NArrayMSxF = NArray_<T, ND, AllocatorMSRef<T, Aln>, MOrder::C, size_t>;


// print
//------
template<class U, typename T, size_t ND, MOrder MOrderT, typename IntT>
std::ostream& print(std::ostream& os, const NArrayInterface_<U, T, ND, MOrderT, IntT>& x, size_t space=4) {
  return os << "NArrayInterface_<" << ND << ">\n";
}


template<class U, typename T, MOrder MOrderT, typename IntT>
std::ostream& print(std::ostream& os, const NArrayInterface_<U, T, 1, MOrderT, IntT>& x, size_t space=4) {
  os << "NArrayInterface_<" << 1 << ">: ";
  for (size_t i = 0, N0 = x.shape()[0]; i < N0; ++i) {
    if (space) os << std::setw(space);
    os << x.at({i}) << ", ";
  }
  return os;
}


template<class U, typename T, MOrder MOrderT, typename IntT>
std::ostream& print(std::ostream& os, const NArrayInterface_<U, T, 2, MOrderT, IntT>& x, size_t space=4) {
  typename NArrayInterface_<U, T, 2, MOrderT, IntT>::index_t idx;
  os << "NArrayInterface_<" << 2 << ">\n";
  for (size_t N0 = x.shape()[0]; idx[0] < N0; ++idx[0], idx[1] = 0) {
    for (size_t N1 = x.shape()[1]; idx[1] < N1; ++idx[1]) {
      if (space) os << std::setw(space);
      os << x.at(idx) << ", ";
    }
    os << '\n';
  }
  return os;
}


template<class U, typename T, MOrder MOrderT, typename IntT>
std::ostream& print(std::ostream& os, const NArrayInterface_<U, T, 3, MOrderT, IntT>& x, size_t space=4) {
  typename NArrayInterface_<U, T, 3, MOrderT, IntT>::index_t idx;
  os << "NArrayInterface_<" << 3 << ">\n";
  for (size_t N0 = x.shape()[0]; idx[0] < N0; ++idx[0], idx[1] = 0) {
    os << "[" << idx[0] << ", :, :]\n";
    for (size_t N1 = x.shape()[1]; idx[1] < N1; ++idx[1], idx[2] = 0) {
      for (size_t N2 = x.shape()[2]; idx[2] < N2; ++idx[2]) {
        if (space) os << std::setw(space);
        os << x.at(idx) << ", ";
      }
      os << '\n';
    }
    os << '\n';
  }
  return os;
}


template<class U, typename T, size_t ND, MOrder MOrderT, typename IntT>
std::ostream& operator<<(std::ostream& os, const NArrayInterface_<U, T, ND, MOrderT, IntT>& x) {
  return print(os, x);
}
} // namespace xmat
