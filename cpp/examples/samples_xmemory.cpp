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
#include "../include/xmat/xmemory.hpp"


std::ostream* kOutStream = &std::cout;

int samples_xmemory_link_0(); // defined in: samples_xutil_link.cpp
int samples_xmemory_link_1(); // defined in: samples_xutil_link.cpp

namespace {

int sample_x() {
  print(__PRETTY_FUNCTION__, 1);
  print("COMMENT", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_0() {
  print(__PRETTY_FUNCTION__, 1);
  print("MemorySource::make", 0, '-');

  const size_t N = 1 << 8;
  char buf[N] = {};

  print(1, "xmat::MemorySource", 0, '-');
  xmat::MemorySource ms(buf, N);
  printv(ms.space());
  ms.allocate(16, std::nothrow);
  printv(ms.space());

  xmat::MemorySourceRef msr(&ms);
  msr.allocate(8, std::nothrow);
  printv(ms.space());
  printv(msr.space());

  print(1, "xmat::TypedMemorySource", 0, '-');
  xmat::AllocatorMSRef<int> tms{&ms};
  tms.allocate(2, std::nothrow);
  printv(msr.space());

  print(1, "call: samples_xmemory_link_0()", 0, '-');
  auto& memsrc = xmat::MemorySourceGlobal::reset(1<<10);
  samples_xmemory_link_0();
  xmat::AllocatorMSGlobal<int> alc_int;
  printv(alc_int.used());
  alc_int.allocate(8);
  printv(alc_int.used());

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_1() {
  print(__PRETTY_FUNCTION__, 1);
  print("MemorySource::methods", 0, '-');

  const size_t N = 1 << 8;
  char buf[N] = {};

  print(1, "xmat::MemorySource", 0, '-');
  xmat::MemorySource ms(buf, N);
  xmat::MemorySourceRef msr(&ms);
  printv(msr.space());
  void* p0 = nullptr;
  
  print(1, "msr.allocate(n, std::nothrow)", 0, '-');
  p0 = msr.allocate(8, std::nothrow);
  printv(msr.space());
  printv(alignof(std::max_align_t));
  printv(xmat::is_aligned(p0, 16));
  printv(xmat::is_aligned(p0, 32));
  printv(xmat::is_aligned(p0, 64));

  print(1, "msr.allocate(n, aln, std::nothrow)", 0, '-');
  p0 = msr.allocate(1, 64, std::nothrow);
  printv(msr.space());
  printv(xmat::is_aligned(p0, 64));

  print(1, "msr.allocate_aln<Aln>(n, std::nothrow)", 0, '-');
  p0 = msr.allocate_aln<32>(1, std::nothrow);
  printv(msr.space());
  printv(xmat::is_aligned(p0, 32));

  print(1, "msr.reserve(nmin, nmax, factor, nout, std::nothrow)", 0, '-');
  size_t nout = 0;
  p0 = msr.reserve(32, 64, 16, &nout, std::nothrow);
  printv(p0 != nullptr);
  printv(msr.space());

  print(1, "msr.extend(ptr, n, std::nothrow)", 0, '-');
  p0 = msr.extend(p0, 128, std::nothrow);
  printv(p0 != nullptr);
  printv(msr.space());

  print(1, "msr.extend_reserve(ptr, nmin, nmax, factor, nout, std::nothrow)", 0, '-');
  p0 = msr.extend_reserve(p0, 128 + 32, 256, 32, &nout, std::nothrow);
  printv(nout);
  printv(msr.space());

  print(1, "msr.deallocate(ptr, n)", 0, '-');
  msr.deallocate(p0, 1);

  print(1, "xmat::AllocatorMSRef<int> alc{&ms}", 0, '=');
  // ----------------------------------------------------
  ms.reset();
  printv(msr.space());
  int* p1 = nullptr;
  xmat::AllocatorMSRef<int> alc{&ms};
  constexpr size_t reqaln = alc.alignment;
  print(1, "alc.allocate()", 0, '-');
  p1 = alc.allocate(4);
  printv(alc.space());
  printv(xmat::is_aligned(p1, reqaln));

  print(1, "alc.reserve()", 0, '-');
  p1 = alc.reserve(8, 16, &nout, std::nothrow);
  printv(nout);
  printv(alc.space());

  print(1, "alc.extend()", 0, '-');
  p1 = alc.extend(p1, 32, std::nothrow);
  printv(p1);
  printv(alc.space());

  print(1, "alc.extend_reserve()", 0, '-');
  p1 = alc.extend_reserve(p1, 32+8, 64, &nout, std::nothrow);
  printv(nout);
  printv(p1);
  printv(alc.space());
  printv(alc.base()->space());

  print(1, "alc.deallocate()", 0, '-');
  alc.deallocate(p1, 1);

  print(1, "FINISH", 1, '=');
  return 1;
}
}


int main() {
  print("START: memory\n" __FILE__, 1);
  
  try {
    sample_1();
  }
  catch (std::exception& err) {
    print(1, "----------------------\n");
    print_mv("caught in main():\n>> ", err.what()); 
    EXIT_FAILURE;
  } 

  return EXIT_SUCCESS;
}
