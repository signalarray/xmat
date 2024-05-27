#include <iostream>
#include <string>
#include <exception>

#include "../include/xmat/xserial.hpp"
#include "../include/xmat/xsocket.hpp"
#include "../include/xmat/xprint.hpp"
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
  socket.connect(xmat::IPAddress::localhost(), xmat::k_xsport, 0.0);

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_3() {
  print(__PRETTY_FUNCTION__, 1);
  print("TCPSocket - send, receive", 0, '-');

  char buf[xmat::k_xsbuf_size] = "";

  print(1, "connect", 0, '-');
  xmat::TCPSocket socket;
  socket.connect(xmat::IPAddress::localhost(), xmat::k_xsport, 0.0);

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
  xmat::OMapStream<> xout{};
  xout.setitem("a0", 4);
  xout.setitem("a1", 3.14);
  xout.close();
  printv(xout);

  print(1, "connect", 0, '-');
  xmat::TCPSocket socket;
  socket.connect(xmat::IPAddress::localhost(), xmat::k_xsport, 0.0);

  print(1, "send BugOut", 0, '-');
  socket.send(xout, 0.0);

  print(1, "FINISH", 1, '=');
  return 1;
}

// after add xmat::Selector, xmat::TCP and non-blocking mode
// ----------------------------------------------------------
// ----------------------------------------------------------
int sample_5() {
  print(__PRETTY_FUNCTION__, 1);
  print("TCPGroup: client", 0, '-');

  print(1, "make client", 0, '-');
  xmat::TCPSocket socket;
  // socket.connect(xmat::IPAddress::localhost(), xmat::k_xsport, xmat::time::inf());
  // socket.connect(xmat::IPAddress::localhost(), xmat::k_xsport, 0.0);
  socket.connect(xmat::IPAddress::localhost(), xmat::k_xsport, 4.0);

  print(1, "FINISH", 1, '=');
  return 1;
}

int sample_6() {
  print(__PRETTY_FUNCTION__, 1);
  print("TCP_<>: listener. accept", 0, '-');

  const size_t N = 8;
  int b0[N] = {1, 2, 3, 4, 5, 6, 7, 8};
  char b1[N] = "absdef";

  xmat::TCP_<1, 1> tcp{};
  auto socket = tcp.client(xmat::IPAddress::localhost(), xmat::k_xsport, 4.0);
  
  xmat::OMapStream<> xout{};
  xout.setitem("a0", 4);
  xout.setitem_n("b0", b0, N);
  xout.setitem_n("b1", b1, N);
  xout.close();
  printv(xout);

  socket->send(xout, 1.0);

  print(1, "FINISH", 1, '=');
  return 1;
}

}


int main() {
  print(2, __FILE__, 0, '-');
  
  try {
    sample_6();
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
