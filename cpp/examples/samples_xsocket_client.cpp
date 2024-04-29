#include <iostream>
#include <string>
#include <exception>

#include "../include/xmat/xserial.hpp"
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
  // mock
  return 1; 
}


int sample_2() {
  print(__PRETTY_FUNCTION__, 1);
  print("TCPSocket - connection only", 0, '-');


  xmat::TCPSocket socket;
  socket.connect(xmat::IPAddress::localhost(), xmat::k_xsport);

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_3() {
  print(__PRETTY_FUNCTION__, 1);
  print("TCPSocket - send, receive", 0, '-');

  char buf[xmat::k_xsbuf_size] = "";

  print(1, "connect", 0, '-');
  xmat::TCPSocket socket;
  socket.connect(xmat::IPAddress::localhost(), xmat::k_xsport);

  print(1, "send", 0, '-');
  std::string msg_send = "message from a client";
  socket.send(msg_send.c_str(), msg_send.size() + 1);

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_4() {
  print(__PRETTY_FUNCTION__, 1);
  print("TCPSocket - send, receive", 0, '-');

  print(1, "make BugOut", 0, '-');
  xmat::BugOut xout{};
  xout.setitem("a0", 4);
  xout.setitem("a1", 3.14);
  xout.close();
  printv(xout);

  print(1, "connect", 0, '-');
  xmat::TCPSocket socket;
  socket.connect(xmat::IPAddress::localhost(), xmat::k_xsport);

  print(1, "send BugOut", 0, '-');
  socket.send(xout);

  print(1, "FINISH", 1, '=');
  return 1;
}
}


int main() {
  print(2, __FILE__, 0, '-');
  
  try {
    sample_4();

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
