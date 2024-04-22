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


std::ostream* kOutStream = &std::cout;

namespace {

int sample_x() {
  print(__PRETTY_FUNCTION__, 1);
  print("COMMENT", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_0() {
  print(__PRETTY_FUNCTION__, 1);
  print("Index<ND>", 0, '-');

  print(1, "constructor: Index<ND>()", 0, '-');
  const int k = 8;
  xmat::Index<4> c[k];
  for (int n = 0; n < k; ++n) {
    printv(c[n]);
  }

  print(1, "constructor: Index_(...)", 0, '-');
  xmat::Index<5> i00 = {1, 2};
  xmat::Index<5> i01{4, 9, 1};
  printv(i00);
  i00 = {3, 4};
  printv(i00);
  i00 = i01;
  printv(i00);
  
  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_1() {
  print(__PRETTY_FUNCTION__, 1);
  print("Incrementor_<ND>", 0, '-');

  print(1, "constructor: Increment<ND>()", 0, '-');
  xmat::IncrementC<4> inc0;
  printv(inc0.shape);
  printv(inc0.stride);

  xmat::IncrementC<4> inc1{{1, 2, 3, 4}};
  printv(inc1.shape);
  printv(inc1.stride);
  
  xmat::IncrementF<3> inc1_{{3, 4, 5}};
  printv(inc1_.shape);
  printv(inc1_.stride);

  print(1, "constructor: IncrementC<ND>()", 0, '-');
  xmat::IncrementC<3> inc2{{3, 4, 5}};
  for (int n = 0; n < 16; ++n, inc2.next()) {
    *kOutStream << inc2.index << " : " << inc2.fidx << '\n';
  }

  print(1, "constructor: IncrementF<ND>()", 0, '-');
  xmat::IncrementF<3> inc3{{3, 4, 5}};
  for (int n = 0; n < 30; ++n, inc3.next()) {
    *kOutStream << inc3.index << " : " << inc3.fidx << '\n';
  }

  print(1, "Increment_<> overflow and comparition", 0, '-');
  xmat::IncrementC<2> inc4{{2, 3}};
  for (int n = 0; n < 30; ++n, inc4.next()) {
    *kOutStream << inc4.index << " : " << inc4.fidx << '\n';
  }

  print(1, "FINISH", 1, '=');
  return 1;
}

template<typename ... Args>
bool check_if_index(Args ... args) { return xmat::AllType<int, Args ...>::value; }

template<size_t N, typename ... Args>
bool check_if_index_n(Args ... args) { return xmat::AllNType<N, int, Args...>::value; }

int sample_2() {
  print(__PRETTY_FUNCTION__, 1);
  print("Ravel_<ND>", 0, '-');

  print(1, "constructor: hidx::conv<>", 0, '-');
  const int N0 = 4;
  int a0[N0] = {1, 2, 3, 4};
  int a1[N0] = {1, 2, 3, 4};
  printv(xmat::hidx::conv(a0, a1, N0, 0));
  printv(xmat::hidx::rconv(a0, a1, N0, 0));
  printv(xmat::hidx::conv<N0>(a0, a1, 0));
  printv(xmat::hidx::rconv<N0>(a0, a1, 0));

  print(1, "constructor: Increment<ND>()", 0, '-');
  xmat::RavelF<3> rv0{{3, 4, 5}};
  printv(rv0.shape);
  printv(rv0.stride);

  printv(rv0.at({1, 1, 1}));
  printv(rv0.at({0, 1, 0}));
  printv(rv0.unravel(5));
  printv(rv0.unraveli(10));

  print(1, "VArgs indexing", 0, '-');
  printv(check_if_index(1, 2, 3, 4, 5));
  printv(check_if_index(1, 2, 3, 4, "asd"));
  printv(check_if_index_n<4>(1, 2, 3, 4));
  printv(check_if_index_n<4>(1, 2, 3));
  printv(check_if_index_n<4>(1, 2, 3, 4, 5));

  printv(rv0.at(1, 1, 1));
  printv(rv0.at(0, 1, 0));

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_3() {
  print(__PRETTY_FUNCTION__, 1);
  print("View<ND>, NArray", 0, '-');

  int buf[256] = {};

  print(1, "View", 0, '-');
  xmat::View<int, 2> v0{buf, {4, 6}};
  auto fv = v0.fbegin();
  for (int n = 0; n < 17; ++n, ++fv) {
    *fv = n;
  }

  xmat::print(*kOutStream, v0);
  print(*kOutStream, v0);
  printv(v0);

  int sha[] = {3, 4, 5};
  int sta[] = {20, 5, 1};
  int staf[] = {1, 3, 12};
  auto a = xmat::hidx::ndcontigous<xmat::MOrder::C>(sha, sta, 3);
  auto af = xmat::hidx::ndcontigous<xmat::MOrder::F>(sha, staf, 3);
  printv(a);
  printv(af);

  print(1, "WIterator, wbegin()", 0, '-');
  auto wit = v0.wbegin();
  printv(wit.ndcontig_);
  printv(wit.delta_);
  printv(wit.length_);

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_4() {
  print(__PRETTY_FUNCTION__, 1);
  print("Slice", 0, '-');

  int buf[256] = {};

  print(1, "View", 0, '-');
  xmat::View<int, 2> v0{buf, {4, 6}};
  auto fv = v0.fbegin();
  for (int n = 0; n < 17; ++n, ++fv) { *fv = n; }

  print(1, "Slice", 0, '-');
  xmat::Slice sl0;
  
  using sl = xmat::sl;
  xmat::NSlice<2> sl0__{sl::all, sl::all};

  auto vv0 = v0.view(sl0__);
  //auto vv1 = v0.view<2>({sl::all, sl::all});

  print(1, "isunique", 0, '-');
  std::array<int, 4> u0 = {3,3,1,1};
  printv(xmat::hidx::isunique(u0.begin(), u0.end()));
  xmat::hidx::booblesort(u0.begin(), u0.end());
  printv(u0);

  print(1, "setdiff", 0, '-');
  std::array<int, 8> sd0 = {1, 2, 3, 4, 5, 6, 7};
  std::array<int, 4> sd1 = {4, 5, 10, 11};
  std::array<int, 8> sd2 = {};

  // auto out = xmat::hidx::setdiff(sd0.begin(), sd0.end(), sd1.begin(), sd1.end(), sd2.begin());
  // auto out = xmat::hidx::setdiff(sd1.begin(), sd1.end(), sd0.begin(), sd0.end(), sd2.begin());
  auto out = xmat::hidx::setdiff(sd0.begin(), sd0.end(), sd1.begin(), sd1.end(), sd0.begin());
  printv(out - sd0.begin());
  printv(sd0);
  printv(sd1);
  printv(sd2);

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_5() {
  print(__PRETTY_FUNCTION__, 1);
  print("ArrayAble::view()", 0, '-');

  int buf[256] = {};

  print(1, "View", 0, '-');
  xmat::View<int, 3> v3d0{buf, {3, 4, 5}};
  printv(v3d0);

  print(1, "View::view", 0, '-');
  using xmat::sl;
  using xmat::sl_;
  auto vv2 = v3d0.view({sl::ilen(0, 1), sl::all, sl::all});
  auto vv2_ = v3d0.view(sl_::ilen(0, 1), sl_::all(), sl_::all());
  printv(vv2.shape());
  printv(vv2.stride());

  print(1, "View::keep", 0, '-');
  auto kvv2 = vv2.keep<2>({1, 2});
  printv(kvv2.shape());
  printv(kvv2.stride());
  
  print(1, "View::drop", 0, '-');
  auto dvv2 = vv2.drop<1>({0});
  printv(dvv2.shape());
  printv(dvv2.stride());

  print(1, "flat", 0, '-');
  for (auto it = dvv2.fbegin(), end = dvv2.fend(); it != end; ++it) {
    *it = 1;
  }
  printv(v3d0);

  print(1, "walk", 0, '-');
  for (auto it = dvv2.wbegin(), end = dvv2.wend(); it != end; ++it) {
    for (auto& it_ : it) {
      it_ = 3;
    }
  }
  printv(v3d0);

  print(1, "slice iteration 1d", 0, '-');
  auto ibeg1d = dvv2.begin();
  auto ibeg0d = ibeg1d.begin();
  auto iend0d = ibeg1d.end();
  int kk = 0;
  for (auto it = ibeg1d.begin(), end = ibeg1d.end(); it != end; ++it) {
    *it = -1;
  }
  printv(v3d0);

  print(1, "slice iteration 1d", 0, '-');
  int q = 100;
  for (auto& it0 : v3d0) {
    for (auto& it1 : it0) {
      it1.at({0}) = ++q;
    }
  }
  printv(v3d0);

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_6() {
  print(__PRETTY_FUNCTION__, 1);
  print("NArray", 0, '-');

  const size_t NN = 1 << 8;
  char buf[NN] = {};
  xmat::MemorySource lms{buf, NN};
  auto* gms = &xmat::MemorySourceGlobal::reset(NN);

  xmat::NArray<int, 2> a00;
  xmat::NArrayGMS<int, 2> a01;
  xmat::NArrayMS<int, 2> a02{&lms};

  xmat::NArray<int, 2> a10{{2, 3}};
  xmat::NArrayGMS<int, 2> a11{{2, 3}};
  xmat::NArrayMS<int, 2> a12{{2, 3}, &lms};

  xmat::NArray_<int, 2, xmat::AllocatorMSGlobal<int, 128>> a22{{2, 3}};

  print(1, "FINISH", 1, '=');
  return 1;
}
} // namespace


int main() {
  print("START: " __FILE__, 0, '=');
  sample_6();
  print("END: " __FILE__, 0, '=');
  return EXIT_SUCCESS;
}