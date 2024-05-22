#include <iostream>
#include <exception>

#include "../include/xmat/xsocket.hpp"
#include "../include/xmat/xserial.hpp"
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
  print("xmat::SocketError::make_msg()", 0, '-');

  printvl(xmat::SocketError::make_msg(xmat::xsstate::good));
  printvl(xmat::SocketError::make_msg(xmat::xsstate::good, "comment message"));
  printvl(xmat::SocketError::make_msg(xmat::xsstate::fail, "fail message"));
  printvl(xmat::SocketError::make_msg(xmat::xsstate::error, "fail message"));

  print(1, "print errors with code", 0, '-');
#ifdef XMAT_USE_WINSOCKET
  std::array<int, 3> errcodes = {WSAEMFILE, WSAENETDOWN, WSAEINVAL};
  for (int n = 0; n < errcodes.size(); ++n) {
    printvl(xmat::SocketError::make_msg(xmat::xsstate::error, "fail message", errcodes[n]));
  }
#endif
 
  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_2() {
  print(__PRETTY_FUNCTION__, 1);
  print("TCPListener, TCPSocket - connection only", 0, '-');

  xmat::TCPListener server;
  server.listen(xmat::k_xsport);
  
  xmat::TCPSocket socket;
  server.accept(socket);
  printv(socket.remoteaddress());

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_3() {
  print(__PRETTY_FUNCTION__, 1);
  print("TCPListener, TCPSocket - send, receive", 0, '-');

  char buf[xmat::k_xsbuf_size] = "";

  print(1, "make a listener", 0, '-');
  xmat::TCPListener server;
  server.listen(xmat::k_xsport);
  
  print(1, "connect", 0, '-');
  xmat::TCPSocket socket;
  server.accept(socket);
  printv(socket.remoteaddress());

  print(1, "receive a message from the client", 0, '-');
  size_t received = socket.recv(buf, sizeof(buf));
  printv(buf);

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_4() {
  print(__PRETTY_FUNCTION__, 1);
  print("receive xmat::BugIn", 0, '-');

  print(1, "make a listener", 0, '-');
  xmat::TCPListener server;
  server.listen(xmat::k_xsport);
  
  print(1, "connect", 0, '-');
  xmat::TCPSocket socket;
  server.accept(socket);
  printv(socket.remoteaddress());

  print(1, "receive in BugIn", 0, '-');
  xmat::IMapStream<> xin;
  socket.recv(xin, 0.0);
  
  printv(xin);

  print(1, "FINISH", 1, '=');
  return 1;
}

// after add xmat::Selector, xmat::TCP and non-blocking mode
// ----------------------------------------------------------
// ----------------------------------------------------------
int sample_5() {
  print(__PRETTY_FUNCTION__, 1);
  print("TCPGroup: listener. accept", 0, '-');

  print(1, "test std::chrono::duration<double>: ", 0, '-');
  std::chrono::duration<double> dt{2.125};
  auto dt2 = std::chrono::duration_cast<std::chrono::milliseconds>(dt);
  auto dt3 = std::chrono::duration_cast<std::chrono::microseconds>(dt);
  printv(dt.count());   //  2.125 
  printv(dt2.count());  //  2125
  printv(dt3.count());  //  2125000
 
  print(1, "make listener: ", 0, '-');
  xmat::TCPListener listener;
  listener.listen(xmat::k_xsport);

  xmat::TCPSocket socket;
  listener.accept(socket);
  printv(socket.remoteaddress());

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_6() {
  print(__PRETTY_FUNCTION__, 1);
  print("TCPGroup: listener. accept", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}
} // namespace


int main() {
  print(2, __FILE__, 0, '-');
  
  try {
    sample_6();

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

