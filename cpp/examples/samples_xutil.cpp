#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <array>
#include <map>
#include <complex>

#include "common.hpp"
#include "../include/xmat/xutil.hpp"


std::ostream* kOutStream = &std::cout;

int samples_xutil_link_0(); // defined in: samples_xutil_link.cpp
int samples_xutil_link_1(); // defined in: samples_xutil_link.cpp

namespace {

int sample_x() {
  print(__PRETTY_FUNCTION__, 1);
  print("COMMENT", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_1() {
  print(__PRETTY_FUNCTION__, 1);
  print("COMMENT", 0, '-');

  const size_t mem_n = 128;
  std::unique_ptr<char[]> buf{new char[mem_n]};
  xmat::memsource mem_mgr{buf.get(), mem_n};

  print(1, "allocate bytes", 0, '-');
  printv(mem_mgr.used());
  auto p0 = mem_mgr.allocate(1);
  printv(mem_mgr.used());
  auto p1 = mem_mgr.allocate(1);
  printv(mem_mgr.used());
  auto p2 = mem_mgr.allocate(2);
  printv(mem_mgr.used());

  print(1, "allocate bytes aligned", 0, '-');
  auto p3 = mem_mgr.allocate_aln<4>(1);
  printv(xmat::is_aligned(p3, 4));
  printv(xmat::is_aligned(p3, 3));
  printv(mem_mgr.used());

  auto p4 = mem_mgr.allocate_aln<4>(1);
  printv(mem_mgr.used());

  print(1, "allocate T", 0, '-');
  auto p10 = mem_mgr.allocate<char>(1);
  printv(mem_mgr.used());
  auto p11 = mem_mgr.allocate<int>(1);
  printv(mem_mgr.used());
  auto p12 = mem_mgr.allocate<double>(1);
  printv(mem_mgr.used());
  auto p13 = mem_mgr.allocate<std::complex<double>>(1);
  printv(mem_mgr.used());
  auto p14 = mem_mgr.allocate<char, 4>(1);
  printv(mem_mgr.used());
  auto p15 = mem_mgr.allocate<char, 4>(1);
  printv(mem_mgr.used());
  auto p16 = mem_mgr.allocate<int, 32>(4);
  printv(xmat::is_aligned(p16, 32));
  printv(mem_mgr.used());

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_2() {
  print(__PRETTY_FUNCTION__, 1);
  print("COMMENT", 0, '-');

  auto& memsrc = xmat::glob_memsource::reset(128);

  xmat::glob_memallocator<int> alc_int;
  auto p0 = alc_int.allocate(1);
  printv(memsrc.used());
  auto p1 = alc_int.allocate(2);
  printv(memsrc.used());

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_3() {
  print(__PRETTY_FUNCTION__, 1);
  print("sample from other object file: `samples_xutil_link.cpp`", 0, '-');

  print(1, "init membuffer in `samples_xutil`", 0, '-');
  auto& memsrc = xmat::glob_memsource::reset(128);

  print(1, "use allocator in root", 0, '-');
  xmat::glob_memallocator<int> alc_int;
  auto p0 = alc_int.allocate(1);
  printv(memsrc.used());
  auto p1 = alc_int.allocate(2);
  printv(memsrc.used());

  print(1, "call: samples_xutil_link_0()", 0, '-');
  samples_xutil_link_0();

  print(1, "again call in root", 0, '-');
  xmat::glob_memallocator<double> alc_db;
  auto d0 = alc_db.allocate(1);
  printv(memsrc.used());
  auto d1 = alc_db.allocate(2);
  printv(memsrc.used());

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_4() {
  print(__PRETTY_FUNCTION__, 1);
  print("memallocator with std::vector, std::map", 0, '-');

  print(1, "std::vector<int, xmat::memallocator<int>>", 0, '-');
  auto& memsrc = xmat::glob_memsource::reset(128);
  std::vector<int, xmat::glob_memallocator<int>> v0;
  printv(memsrc.used());
  v0.resize(4);
  printv(memsrc.used());
  v0.resize(8);
  printv(memsrc.used());

  std::vector<int, xmat::glob_memallocator<int>> v1 = {11, 22};
  v0[0] = 10;
  printv(v0[0]);
  printv(memsrc.used());
  v0 = v1;
  printv(v0.size());
  printv(v0.capacity());
  printv(v1.size());
  printv(v1.capacity());
  printv(memsrc.used());

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_5() {
  print(__PRETTY_FUNCTION__, 1);
  print("memsourcre", 0, '-');

  char buf[128];
  xmat::memsource ms{buf, sizeof(buf)};
  auto p0 = ms.allocate(3);
  printv(ms.used());

  for (int n = 0; n < 16; ++n ){
    printv(n);
    p0 = ms.extend(p0, 4 + n);
  }



  print(1, "FINISH", 1, '=');
  return 1;
}
} // namespace noname


int main() {
  print("START: " __FILE__, 0, '=');
  sample_5();
  print("END: " __FILE__, 0, '=');
  return EXIT_SUCCESS;
}
