#include "common.hpp"
#include "../include/xmat/xmemory.hpp"


int samples_xmemory_link_0() {
  print("from file:" __FILE__, 1);

  print(1, "use allocator in object file", 0, '-');
  auto& memsrc = xmat::MemorySourceGlobal::get();
  xmat::AllocatorMSGlobal<int> alc_int;
  auto p0 = alc_int.allocate(1);
  printv(memsrc.used());
  auto p1 = alc_int.allocate(2);
  printv(memsrc.used());

  return 1;
}


int samples_xmemory_link_1() {
  print("from file:" __FILE__);
  return 1;
}
