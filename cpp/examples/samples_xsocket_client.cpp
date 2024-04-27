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
  print("COMMENT", 0, '-');

  char buf[xmat::k_sock_buf_n] = "";

  xmat::TCPSocket socket;
  // auto resout = socket.connect(xmat::IPAddress{127, 0, 0, 1}, xmat::k_port);
  auto resout = socket.connect(xmat::IPAddress::localhost(), xmat::k_port);
  if(resout != xmat::SocketState::done) { throw std::runtime_error("socket.connect"); }
  

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
    print(1, "exception in main()", 1, '+');
    print_mv("exception message: >>", err.what());
    return EXIT_FAILURE;  
  }
  print(1, "FINISH" __FILE__, 1, '=');
  return EXIT_SUCCESS;
}
