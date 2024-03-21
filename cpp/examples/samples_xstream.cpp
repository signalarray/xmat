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

  const int buf_size = 16;

  // constructor default
  xmat::bbuf_stor_dyn bbs0_0{};
  xmat::bbuf_stor_static<buf_size> bbs0_1{};
  xmat::bbuf_stor_view bbs0_2{};
  xmat::bbuf_stor_cview bbs0_3{};

  bbs0_0.reserve(buf_size);
  
  // constructor {ptr, n}
  auto ptr = bbs0_0.data();
  auto n = bbs0_0.size();
  xmat::bbuf_stor_dyn bbs1_0{ptr, n};
  xmat::bbuf_stor_static<buf_size + 4> bbs1_1{ptr, n};
  xmat::bbuf_stor_view bbs1_2{ptr, n};
  xmat::bbuf_stor_cview bbs1_3{ptr, n};

  // assignment

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_1() {
  print(__PRETTY_FUNCTION__, 1);
  print("obbuf, ibbuf: simple", 0, '-');

  xmat::obbuf<> obb;
  

  print(1, "FINISH", 1, '=');
  return 1;
}
}

int main() {
  print("START: " __FILE__, 0, '=');
  sample_0();
  print("END: " __FILE__, 0, '=');
  return EXIT_SUCCESS;
}
