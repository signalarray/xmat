#include <cctype>
#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <complex>
#include <type_traits>

#include "../include/xmat/xarr.hpp"
#include "common.hpp"


std::ostream* kOutStream = &std::cout;

namespace {

void print_x(const xmat::Map& cr) {
  printv(char(cr.order));
  printv(cr.ndim);
  printv(cr.numel);
  printv(cr.shape);
  printv(cr.stride);
  // printv(cr.origin);
  // printv(cr.step);
}


template<typename T>
void print_a(const xmat::Array<T>& x, std::size_t space=0) {
  xmat::print(*kOutStream, x, space);
}


int sample_0() {
  print("SAMPLE_0", 1);
  print("using common.hpp>print..(...)", 1, '=');

  print("expressions:", 0, '-');
  printv(3 * (1 + 4));

  xmat::Map idx(4, 4);
  printv(idx.at(1, 1));
  
  print(1, "print_seq: ");
  int x[] = {1, 2, 3};
  print_seq(x);

  print(2, "vector<int>: ");
  std::vector<int> x1{1, 2, 3};
  print_seq(x1);

  print(2, "print std::container<T>", 1);
  printv(x1);

  print(2, "print container<container<T>>", 1);
  std::vector<std::vector<int>> x2{{1, 2, 3}, {4, 5, 6, 7}};
  printv(x2);

  print(2, "finish", 1, '=');
  return 1;
}


int sample_1() {
  print("SAMPLE_1", 1);
  print("using xarr.hpp>xmat::Map", 1, '=');
  
  print("make xmat::Index", 0, '-');
  xmat::Map cr(3, 5);
  printv(char(cr.order));
  printv(cr.ndim);
  printv(cr.numel);
  printv(cr.shape);
  printv(cr.stride);

  xmat::Map cr1(3, 5);
  print_mv("another: ", cr1.shape);

  print(1, "xmat::Index", 0, '-');
  xmat::Index idx = {1, 2};
  printv(idx);
  printv(idx.ndim());
  printv(idx.numel());
  

  print(1, "IndexMap::at()", 0, '-');
  printv(cr.at(1, 1));
  printv(cr.at(0, 1));
  printv(cr.at(1, 0));

  print(1, "IndexMap::set_order()", 0, '-');
  cr.set_order(xmat::Order::F);
  printv(char(cr.order));
  printv(cr.at(1, 1));
  printv(cr.at(0, 1));
  printv(cr.at(1, 0));

  print(1, "finish", 1, '=');
  return 1;
}

int sample_2() {
  print("SAMPLE_2", 1);
  print("using xarr.hpp>xmat::Index2", 1, '=');

  xmat::Index i0 = {1, 2, 3};
  print_mv("i0: ", i0);
  i0 = {0};
  print_mv("i0: ", i0);

  print(1, "xmat::Index2", 0, '-');
  xmat::Index i1 = {1, 2, 3};
  xmat::Index i2{{2, 2}};
  printv(i1);
  printv(i2);

  print(1, "xmat::Index2 - assignment", 0, '-');
  i2 = i1;
  printv(i1);
  printv(i2);

  print(1, "xmat::Map ", 0, '-');
  xmat::Map cr0{{1, 2, 3}};
  xmat::Map cr1{{4, 3, 2, 1}};

  print(1, "cr0: ", 0, '-');
  print_x(cr0);
  print(1, "cr1: ", 0, '-');
  print_x(cr1);

  print(1, "ways to make xmat::Index", 0, '-');
  xmat::IndexBase a0 = {3, 2, 1};
  xmat::Index i00 = {1, 2, 3};
  xmat::Index i01{{1, 2, 3}};
  xmat::Index i02(a0);
  xmat::Index i03 = a0;
  
  print(1, "ways to make xmat::Map", 0, '-');
  xmat::Index i11 = {1, 2, 3};
  xmat::Map cr00{i11};
  xmat::Map cr01{{4, 3, 2, 1}};
  xmat::Map cr02{3, 2};

  print(1, "finish", 1, '=');
  return 1;
}


int sample_3() {
  print("SAMPLE_3", 1);
  print("using Array", 1, '=');

  xmat::Array<int> a0({2, 3});
  printv(a0(0, 0));
  a0(0, 0) = -32;
  printv(a0(0, 0));
  
  print(1, "print Array", 0, '-');
  print_a(a0, 4);

  print(1, "ways to make Array");
  xmat::Array<int> a1{{3}, nullptr};

  print(1, "finish", 1, '=');
  return 1;
}


int sample_4() {
  print("SAMPLE_4", 1);
  print("cast Arrays", 1, '=');

  auto a0 = xmat::cast<int>((double)1.0);
  auto a1 = xmat::cast<int8_t>(1.0);
  auto a2 = xmat::cast<double>('1');

  auto a3 = xmat::cast<std::complex<int>>(2);
  auto a4 = xmat::cast<std::complex<double>>(2);
  auto a5 = xmat::cast<std::complex<int>>(2.0);

  auto b0 = xmat::cast<std::complex<int>>(std::complex<int>{1, 2});
  auto b1 = xmat::cast<std::complex<double>>(std::complex<int>{1, 2});
  auto b2 = xmat::cast<std::complex<int>>(std::complex<double>{1.0, 2.0});

  printv(a0); printv((int)a1); printv(a2);
  printv(a3); printv(a4); printv(a5);
  printv(b0); printv(b1); printv(b2);


  print(1, "xmat::Array<T> cast", 0, '-');
  xmat::Array<int> x0{{2, 3}};
  x0.enumerate();

  auto x1 = x0.cast<double>();
  auto x2 = x1.cast<std::complex<unsigned>>();
  auto x3 = x2.cast<std::complex<float>>();

  print("x0<int>: ", 1);
  xmat::print(*kOutStream, x0, 4);
  print("x1<double>: ", 1);
  xmat::print(*kOutStream, x1, 4);
  print("x2<cx<double>>: ", 1);
  xmat::print(*kOutStream, x2, 4);
  print("x3<cx<float>>: ", 1);
  xmat::print(*kOutStream, x3, 4);
  print(1, "finish", 1, '=');
  return 1;
}
}


int main() {
  print("START: " __FILE__, 0, '=');
  // sample_2();
  // sample_3();
  sample_4();
  print("END: " __FILE__, 0, '=');
  return EXIT_SUCCESS;
}
