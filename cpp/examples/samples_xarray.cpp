#include <cctype>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <array>
#include <complex>
#include <utility>
#include <type_traits>

#include "../include/xmat/xarray.hpp"
#include "common.hpp"


// just while developint xarray.hpp
// namespace xmat = xmat_new;

std::ostream* kOutStream = &std::cout;

namespace {

// https://www.scs.stanford.edu/~dm/blog/param-pack.html
// https://stackoverflow.com/questions/58598763/how-to-assign-variadic-template-arguments-to-stdarray
// https://itecnote.com/tecnote/c-create-static-array-with-variadic-templates/
// https://www.scs.stanford.edu/~dm/blog/param-pack.html
int sum(int a, int b) { return a + b; }

int foo(const int(&x)[4]) {
  int a = 0;
  for (int it : x) { a+= it; }
  return a;
}

int foo0(std::initializer_list<int> ls) {
  int a = 0;
  for (int it : ls) { a+= it; }
  return a;
}

int sub(int _0) { return 0; }
int sub(int _0, int _1) { return 1; }
int sub(int _0, int _1, int _2) { return 2; }

template<typename ... Args>
std::enable_if_t<(sizeof ... (Args)) <= 3, int>
foo2(Args ... args) {
  return sub(args ...);
}

template<typename ... Args>
int foo3(Args ... args) {
  int a1[] = {args ...};
  return a1[1];
}

// --------------------------------------
// compile time arrays
// --------------------------------------
template <typename T, std::size_t N>
constexpr T cumprod(T const (&a)[N], std::size_t i = 0U) { 
  return i < N ? (a[i] * cumprod(a, i+1U)) : T{1};
}

template<std::size_t ... Args>
struct ShapeC {
  static const std::size_t ndim = sizeof ... (Args);
  static constexpr const std::size_t shape[] = {Args ...};
  static const std::size_t numel = cumprod(shape);
};


template<std::size_t ND>
struct ShapeR {
  static const std::size_t ndim = ND;
  
  const std::size_t shape[ndim] = {};
  const std::size_t numel = 0;
};

// samples
// --------------------
int sample_x() {
  print(__PRETTY_FUNCTION__, 1);
  print("COMMENT", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_00() {
  print(__PRETTY_FUNCTION__, 1);
  print("COMMENT", 0, '-');

  ShapeC<2, 3, 4> sh0;
  printv(sh0.ndim);
  printv(sh0.shape[0]);
  printv(sh0.shape[1]);
  printv(sh0.shape[2]);
  printv(sh0.numel);

  print(1, "FINISH", 1, '=');
  return 1;
}

int sample_0() {
  print(__PRETTY_FUNCTION__, 1);
  print("some trash: ", 0, '-');
  
  int x0 = 2, x1 = 3, x2 = 4;
  int x3 = sum(x0, 1), x4 = sum(x1, 1), x5 = sum(x0, 1);

  std::array<int, 3> a0 = {1, 2};
  xmat::Index<3> a1{{1, 2}};
  xmat::Index<3> a2{{11, 12, 13}};

  int b0 = foo({1, 2, 3});
  printv(b0);

  int b1 = foo0({1, 2, 3, 10});
  printv(b1);

  int b2 = foo2(1);
  printv(b2);
  printv(foo2(1));
  printv(foo2(1, 1));
  printv(foo2(1, 1, 2));

  int b3 = foo3(1, 2, 3);
  printv(b3);
  printv(foo3(1, 10, 3));
  printv(foo3(1, 11, 3, 11));

  print(1, "FINISH", 1, '=');
  return 1;
}

int sample_1() {
  print(__PRETTY_FUNCTION__, 1);
  print("xmat::Index<ND> ", 0, '-');

  print("index create", 0, '-');
  xmat::Index<3> ii0 = {1, 2, 3};
  xmat::Index<3> ii1 = ii0;
  printv(ii0);
  printv(ii1);
  ii1[1] = 11;
  ii0 = ii1;
  printv(ii0);
  printv(ii1);
  ii0 = {3, 2, 1};
  ii1 = {13, 12};
  printv(ii0);
  printv(ii1);
  ii0.fill(0);
  printv(ii1);

  print("index make of std::array", 0, '-');
  std::array<xmat::szt, 3> a0 = {33, 22, 11};
  xmat::Index<3> ii1_0{a0};
  xmat::Index<3> ii1_1 = a0;

  print("index props", 0, '-');
  xmat::Index<4> ii2 = {2, 3, 4, 5};
  xmat::Index<4> ii3 = {2, 3, 0, 0};
  xmat::Index<4> ii4 = {2, 3, 0, 5};

  printv(ii2.numel());
  printv(ii3.numel());
  printv(ii4.numel());

  printv(ii2.top());
  printv(ii3.top());
  printv(ii4.top());

  printv(ii2.topnumel());
  printv(ii3.topnumel());
  printv(ii4.topnumel());

  printv(ii2.check());
  printv(ii3.check());
  printv(ii4.check());

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_2() {
  print(__PRETTY_FUNCTION__, 1);
  print("xmat.Ravel", 0, '-');

  xmat::Ravel<4> rv{{2, 3, 4, 5}};
  printv(rv.shape);
  printv(rv.stride);
  rv.set_order(xmat::Order::F);
  printv(rv.stride);

  print("indexing order F", 0, '-');
  printv(rv.at({1, 0, 0, 0}));
  printv(rv.at({0, 1, 0, 0}));
  printv(rv.at({0, 0, 1, 0}));
  printv(rv.at({0, 0, 0, 1}));
  printv(rv.at({0, 0, 0, 2}));

  print("indexing order C", 0, '-');
  rv.set_order(xmat::Order::C);
  printv(rv.at({1, 0, 0, 0}));
  printv(rv.at({0, 1, 0, 0}));
  printv(rv.at({0, 0, 1, 0}));
  printv(rv.at({0, 0, 0, 1}));
  printv(rv.at({0, 0, 0, 2}));

  print(1, "indexing with args...", 0, '-');
  printv(rv.at(1, 0, 0, 0));
  printv(rv.at(0, 1, 0, 0));

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_5() {
  print(__PRETTY_FUNCTION__, 1);
  print("Array indexing:", 0, '-');

  print(1, "xmat::conv:", 0, '-');
  std::array<unsigned, 4> a1 = {10, 20, 30, 40};
  printv(a1);
  printv(xmat::conv(a1, 1, 0, 0));
  printv(xmat::conv(a1, 0, 1, 0));
  printv(xmat::conv(a1, 1, 0, 1));
  printv(xmat::conv(a1, 2, 0, 0));

  print(1, "indexing", 0, '-');
  xmat::NArray<int, 4> ar1{{2, 3, 4, 5}};
  printv(ar1.at({1, 2, 3}));
  printv(ar1.at({1, 0, 0}));
  ar1.at({1, 2, 3}) = 1;
  ar1.at({1, 0, 0}) = 2;
  printv(ar1.at(1, 2, 3, 0));
  printv(ar1.at(1, 0, 0, 0));

  // xmat::print(*kOutStream, ar0, 4);
  
  print(1, "FINISH", 1, '=');
  return 1;
}

int sample_6() {
  // https://www.learncpp.com/cpp-tutorial/stdarray-of-class-types-and-brace-elision/
  print(__PRETTY_FUNCTION__, 1);
  print("Slice", 0, '-');

  print(1, "Array", 0, '-');
  xmat::NArray<int, 2> a0{{4, 5}};
  a0.fill(0, 1);
  a0.at({0, 1}) = -1;
  a0.at(2, 1) = -2;
  xmat::print(*kOutStream, a0, 4);

  print(1, "Slice[]", 0, '-');
  std::array<xmat::Slice, 2> sl_0{{{2, 3, 1}, {1, 4, 1}}};
  printv(sl_0);

  print(1, "Ravel", 0, '-');
  auto& rv0 = a0.ravel_;
  xmat::Ravel<2> rv1 = rv0.slice(sl_0);
  printv(rv1.origin);
  printv(rv1.stride);
  printv(rv1.at({0, 0}));
  printv(rv1.at(0, 0));
  printv(rv1.at(0, 1));
  printv(rv1.at(0, 2));

  print(1, "slice_2", 0, '-');
  std::array<xmat::Slice, 2> sl_1{{{1, 4}, {3, 4}}};
  printv(sl_1);
  xmat::Ravel<2> rv2 = rv0.slice(sl_1);
  printv(rv2.at(0, 0));
  printv(rv2.at(1, 0));
  printv(rv2.at(2, 0));

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_7() {
  print(__PRETTY_FUNCTION__, 1);
  print("View:", 0, '-');

  print(1, "Array", 0, '-');
  xmat::NArray<int, 2> a0{{4, 5}};
  a0.fill(0, 1);
  printv(a0);

  print(1, "Array.view", 0, '-');
  auto v0 = a0.view();
  printv(v0);
  
  print(1, "View.slice", 0, '-');
  using xmat::Slice;
  auto s0 = v0.slice({Slice{2, 3, 1}, Slice{1, 4, 1}});
  printv(s0);

  printv(v0.slice({{{1, 4}, {3, 4}}}));

  print(1, "Ravel.reshape", 0, '-');
  auto& rv0 = a0.ravel_;
  // auto rv1 = rv0.reshape<1>({2, 10}); //??? why 2 elements for array1 length
  auto rv1 = rv0.reshape<2>({2, 10});
  printv(rv1.shape);
  printv(rv1.stride);

  print(1, "View.reshape", 0, '-');
  auto v1 = v0.reshape<2>({2, 10});
  printv(v1);

  print(1, "View/Slice.squeese", 0, '-');
  auto s0_1d = s0.squeese<1>();
  printv(s0_1d);

  auto s1 = v0.slice({{{1, 4}, {3, 4}}});
  auto s1_1d = s1.squeese<1>();
  printv(s1_1d);

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_8() {
  print(__PRETTY_FUNCTION__, 1);
  print("Iterators and assignment", 0, '-');

  print(1, "Array", 0, '-');
  xmat::NArray<int, 2> a0{{4, 5}};
  a0.fill(0, 1);
  auto v0 = a0.view();
  printv(v0);

  auto s0 = v0.slice({{{1, 3}, {3, 4}}});
  auto s0_1d = s0.squeese<1>();
  printv(s0_1d);

  print(1, "test iterator<1D>", 0, '-');
  auto it0 = s0_1d.begin();
  auto end0 = s0_1d.end();
  printv(s0_1d.numel());
  printv(*end0);
  for (int n = 0, N = s0_1d.numel(); n < N; ++n, ++it0) {
    printv(*it0);
    printv(it0 == end0);
  }
  printv(it0 == end0);
  
  print(1, "print", 0, '-');
  printv(s0_1d);

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_9() {
  print(__PRETTY_FUNCTION__, 1);
  print("array_storage_ms<std::allocator>: ctor, copy, move, assignment", 0, '-');
  
  auto gmemsrs = &xmat::GlobalMemSource::reset(1 << 10);

  // using use_allocator = std::allocator<int>;
  using use_allocator = xmat::GlobalMemAllocator<int>;

  print(1, "std::allocator<> or xmat::global_memallocator<>", 0, '-');
  print(1, "constructor", 0, '-');
  xmat::NArrayStorageMS<int, use_allocator> as00{};
  as00.init(16);
  as00.data_[0] = -1;
  printv(as00.N_);
  printv(as00.data_[0]);

  print(1, "copy ctr", 0, '-');
  xmat::NArrayStorageMS<int, use_allocator> as01{as00};
  printv(as01.data_[0]);
  as01.data_[0] = 11;
  printv(as01.data_[0]);
  printv(as01.N_);
  
  print(1, "assign", 0, '-');
  as00 = as01;
  printv(as00.N_);
  printv(as00.data_[0]);

  print(1, "move ctr", 0, '-');
  xmat::NArrayStorageMS<int, use_allocator> as02{std::move(as00)};
  printv(as02.data_[0]);
  printv(as02.N_);
  printv(as00.N_);
  
  print(1, "move assign", 0, '-');
  as01.data_[0] = 222;
  printv(as01.data_[0]);
  printv(as02.data_[0]);
  as01 = std::move(as02);
  printv(as01.N_);
  printv(as02.N_);
  printv(as01.data_[0]);

  print(1, "FINISH", 1, '=');
  return 1;
}

int sample_10() {
  print(__PRETTY_FUNCTION__, 1);
  print("array_storage_ms<std::allocator>: ctor, copy, move, assignment", 0, '-');
  
  auto gmemsrs = &xmat::GlobalMemSource::reset(1 << 10);
  using use_allocator = xmat::MemSourceAlloc<int>;

  print(1, "std::allocator<>", 0, '-');
  print(1, "constructor", 0, '-');
  xmat::NArrayStorageMS<int, use_allocator> as00{gmemsrs};
  as00.init(16);
  as00.data_[0] = -1;
  printv(as00.N_);
  printv(as00.data_[0]);

  print(1, "copy ctr", 0, '-');
  xmat::NArrayStorageMS<int, use_allocator> as01{as00};
  printv(as01.data_[0]);
  as01.data_[0] = 11;
  printv(as01.data_[0]);
  printv(as01.N_);
  
  print(1, "assign", 0, '-');
  as00 = as01;
  printv(as00.N_);
  printv(as00.data_[0]);

  print(1, "move ctr", 0, '-');
  xmat::NArrayStorageMS<int, use_allocator> as02{std::move(as00)};
  printv(as02.data_[0]);
  printv(as02.N_);
  printv(as00.N_);
  
  print(1, "move assign", 0, '-');
  as01.data_[0] = 222;
  printv(as01.data_[0]);
  printv(as02.data_[0]);
  as01 = std::move(as02);
  printv(as01.N_);
  printv(as02.N_);
  printv(as01.data_[0]);

  print(1, "FINISH", 1, '=');
  return 1;
}

int sample_11() {
  print(__PRETTY_FUNCTION__, 1);
  print("ArrayMS", 0, '-');

  auto gmemsrc = &xmat::GlobalMemSource::reset(1 << 10);

  print(1, "xmat::array_storage_ms (with explicit storage argument)", 0, '-');
  xmat::NArrayStorageMS<int, std::allocator<int>> 
  stor0{std::allocator<int>{}};

  xmat::NArrayStorageMS<int, xmat::GlobalMemAllocator<int>> 
  stor1{xmat::GlobalMemAllocator<int>{}};

  xmat::NArrayStorageMS<int, xmat::MemSourceAlloc<int>> 
  stor2{xmat::MemSourceAlloc<int>{gmemsrc}};

  print(1, "xmat::array_storage (with default storage constructor)", 0, '-');
  xmat::NArrayStorageMS<int, std::allocator<int>> stor10{};
  xmat::NArrayStorageMS<int, xmat::GlobalMemAllocator<int>> stor11{};

  print(1, "xmat::ArrayMS (with explicit storage constructor)", 0, '-');
  xmat::NArray_<int, 2, std::allocator<int>> a0{std::allocator<int>{}};
  xmat::NArray_<int, 2, xmat::GlobalMemAllocator<int>> a1{xmat::GlobalMemAllocator<int>{}};
  xmat::NArray_<int, 2, xmat::MemSourceAlloc<int>> a2{xmat::MemSourceAlloc<int>{gmemsrc}};
  xmat::NArray_<int, 2, xmat::MemSourceAlloc<int>> a3{gmemsrc};
  
  print(1, "xmat::Array (with default storage constructor)", 0, '-');
  xmat::NArray_<int, 2, std::allocator<int>> b0{};
  xmat::NArray_<int, 2, xmat::GlobalMemAllocator<int>> b1{};

  print(1, "FINISH", 1, '=');
  return 1;
}

int sample_12() {
  print(__PRETTY_FUNCTION__, 1);
  print("View manipulations", 0, '-');

  xmat::NArray<int, 2> a0{{2, 4}};
  auto va0 = a0.view();
  size_t n = va0.contigous();
  printv(n);

  print(1, "FINISH", 1, '=');
  return 1;
}
}


int main() {
  print("START: " __FILE__, 0, '=');
  sample_12();
  print("END: " __FILE__, 0, '=');
  return EXIT_SUCCESS;
}
