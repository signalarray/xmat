#include <iostream>
#include <exception>

#include "../include/xmat/xsocket.hpp"
#include "common.hpp"


std::ostream* kOutStream = &std::cout;

namespace {
int sample_x() {
  print(__PRETTY_FUNCTION__, 1);
  print("COMMENT", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_1() {
  print(4, __PRETTY_FUNCTION__, 1);
  print("TCPSocket - connection only", 0, '-');

  xmat::TCPSocket socket;
  socket.connect(xmat::IPAddress::localhost(), xmat::k_xsport);

  print(1, "FINISH", 1, '=');
  return 1;
}
}


int main() {
  print(4, __FILE__, 0, '-');
  
  try {
    sample_1();

  }
  catch (std::exception& err) {
    print("\n+++++++++++++++++++++\n");
    print_mv("exception im main: message: >>\n", err.what());
    print("\n+++++++++++++++++++++\n");
    return EXIT_FAILURE;  
  }
  print(1, "FINISH" __FILE__, 1, '=');
  return EXIT_SUCCESS;
}
