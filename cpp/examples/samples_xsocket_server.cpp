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


int sample_0() {
  print(__PRETTY_FUNCTION__, 1);
  print("IPAddress", 0, '-');
  xmat::SocketStartup::init();

  printv("ssss");
  printv(xmat::isle());

  printv(xmat::IPAddress::none());
  printv(xmat::IPAddress::any());
  printv(xmat::IPAddress::localhost());
  printv((xmat::IPAddress{127, 0, 0, 1}));
  printv(xmat::IPAddress{"localhost"});
  printv(xmat::IPAddress::localaddress());
  printv(xmat::IPAddress{""}.is_none());

  print(1, "byte order", 0, '-');
  int x = 1;
  printv(htonl(x));
  printv(htonl(x) + 1);
  printv(htonl(ntohl(x)));
  printv(ntohl(x));
  printv(ntohl(htonl(x)));


  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_1() {
  print(__PRETTY_FUNCTION__, 1);
  print("COMMENT", 0, '-');

  char buf[xmat::k_sock_buf_n] = "";

  xmat::TCPListener server;
  if (server.listen(xmat::k_port) != xmat::SocketState::done) {
    throw std::runtime_error("fail accept"); 
  }
  
  xmat::TCPSocket socket;
  if (server.accept(socket) != xmat::SocketState::done) { 
    throw std::runtime_error("fail accept"); 
  }
  printv(socket.remoteaddress());

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_2() {
  print(__PRETTY_FUNCTION__, 1);
  print("raw socket server", 0, '-');



  print(1, "FINISH", 1, '=');
  return 1;
}

} // namespace


int main() {
  print(4, __FILE__, 0, '-');
  std::setlocale(LC_ALL, "ru-RU");
  
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
