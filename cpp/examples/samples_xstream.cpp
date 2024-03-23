#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <array>

#include "common.hpp"
#include "../include/xmat/xstream.hpp"

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
  print("bbuf_stor_..", 0, '-');

  print(1, "bbuf_storage_dyn", 0, '-');
  xmat::bbuf_storage_dyn bbdyn;
  for (int n = 0; n < 8; ++n) {
    print_mv("n :", n);
    bbdyn.size_request(n);
    printv(bbdyn.size());
  }

  print(1, "bbuf_storage_memsource", 0, '-');
  auto gmemsrc = &xmat::glob_memsource::reset(5);
  xmat::bbuf_storage_ms bbms{gmemsrc};
  for (int n = 0; n < 8; ++n) {
    print_mv("n :", n);
    printv(bbms.size_request(n));
    printv(bbms.size());
  }

  // print(1, "bbuf_storage_memsource: local", 0, '-');
  // xmat::memsource loc_memsrc{bbdyn.data(), bbdyn.size()};
  // xmat::bbuf_storage_ms bbms1{&loc_memsrc};
  // for (int n = 0; n < 8; ++n) {
  //   print_mv("n :", n);
  //   printv(bbms1.size_request(n));
  //   printv(bbms1.size());
  // }

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_1() {
  print(__PRETTY_FUNCTION__, 1);
  print("obbuf", 0, '-');

  print(1, "bbuf_storage_dyn", 0, '-');
  // xmat::bbuf_storage_dyn bbdyn;
  xmat::obbuf<> obb;
  const char line1[] = "line 1. ";
  const char line2[] = "line 2";

  obb.write(line1, sizeof(line1)-1);
  obb.write(line2, sizeof(line2));
  printv(obb.storage_.data())

  print(1, "FINISH", 1, '=');
  return 1;
}
}

int main() {
  print("START: " __FILE__, 0, '=');
  sample_1();
  print("END: " __FILE__, 0, '=');
  return EXIT_SUCCESS;
}
