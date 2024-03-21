#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <array>

#include "common.hpp"
#include "../include/xmat/xutil.hpp"


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
  print("COMMENT", 0, '-');

  print(1, "align_up", 0, '-');
  printv(xmat::align_up_(0, 4));
  printv(xmat::align_up_(1, 4));
  printv(xmat::align_up_(2, 4));
  printv(xmat::align_up_(3, 4));
  printv(xmat::align_up_(4, 4));
  printv(xmat::align_up_(5, 4));
  printv(xmat::align_up_(6, 4));
  printv(xmat::align_up_(7, 4));
  printv(xmat::align_up_(8, 4));
  
  xmat::memsource<int> mem_int{128};

  print(1, "mem.data()", 0, '-');
  int* p_data = mem_int.data();
  auto pint_data = reinterpret_cast<uintptr_t>(p_data);
  auto rd = pint_data % 16;
  
  printv(sizeof(int));
  printv(sizeof(size_t));
  printv(pint_data);
  printv(rd);
  printv(alignof(p_data));
  printv(alignof(reinterpret_cast<char*>(pint_data + 1)));

  print(1, "mem.allocate(4)", 0, '-');
  int* p_0 = mem_int.allocate(2);
  printv((char*)p_0 - (char*)p_data);

  print("[1]", 1);
  int* p_1 = mem_int.allocate(4);
  printv((char*)p_1 - (char*)p_data);
  printv((char*)p_1 - (char*)p_0);
  
  print("[2]", 1);
  int* p_2 = mem_int.allocate_aln<4>(3);
  printv((char*)p_2 - (char*)p_1);
  
  print("[3]", 1);
  int* p_3 = mem_int.allocate_aln<4>(3);
  printv((char*)p_3 - (char*)p_2);

  print("[4]", 1);
  int* p_4 = mem_int.allocate_aln<4>(3);
  printv((char*)p_3 - (char*)p_2);

  print("[4]", 1);
  int* p_5 = mem_int.allocate(3);
  printv((char*)p_5 - (char*)p_4);

  print(1, "FINISH", 1, '=');
  return 1;
}
} // namespace noname

int main() {
  print("START: " __FILE__, 0, '=');
  sample_0();
  print("END: " __FILE__, 0, '=');
  return EXIT_SUCCESS;
}
