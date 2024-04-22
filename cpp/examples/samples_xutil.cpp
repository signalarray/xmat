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

namespace {

int sample_x() {
  print(__PRETTY_FUNCTION__, 1);
  print("COMMENT", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}

} // namespace noname


int main() {
  print("START: " __FILE__, 0, '=');
  sample_x();
  print("END: " __FILE__, 0, '=');
  return EXIT_SUCCESS;
}
