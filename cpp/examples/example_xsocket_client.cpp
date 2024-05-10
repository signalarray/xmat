#include <iostream>
#include <exception>

#include "../include/xmat/xsocket.hpp"
#include "../include/xmat/xserial.hpp"
#include "common.hpp"


std::ostream* kOutStream = &std::cout;

namespace {
// See: ./example_xsocket_server.cpp::example_()
int example_0() {
  print(__PRETTY_FUNCTION__, 1);
  print("TCPGroup: listener. accept", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}
}


int main() {
  print(2, __FILE__, 0, '-');
  print("xmat/xsocket.hpp examples: server", 0, '-');
  
  try {
    example_0();
  }
  catch (std::exception& err) {
    print("\n+++++++++++++++++++++\n");
    print_mv("exception in main: message: >>\n", err.what());
    print("\n+++++++++++++++++++++\n");
    return EXIT_FAILURE;  
  }
  print(1, "FINISH" __FILE__, 1, '=');
  return EXIT_SUCCESS;
}
