#include <cctype>
#include <iostream>
#include <vector>
#include <array>
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

  print(2, "finish", 0, '=');
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

  print(1, "finish", 0, '=');
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

  print(1, "finish", 0, '=');
  return 1;
}


int sample_3() {
  print("SAMPLE_3", 1);
  print("using Index and Map", 1, '=');

  print("Map.origin", 0, '-');
  xmat::Map mp0 = {2, 3};

  print(1, "finish", 0, '=');
  return 1;
}
}


int main() {
  sample_2();
}
