#include <iostream>
#include <exception>

#include "../include/xmat/xsocket.hpp"
#include "common.hpp"


std::ostream* kOutStream = &std::cout;

int main() {
  print("START: " __FILE__, 1);
  print(__PRETTY_FUNCTION__, 1);
  printv(xmat::IP_SERVER);

  const char msg[xmat::kDefaultMsgLen] = "message from server 1234";
  char buff[xmat::kDefaultMsgLen] = "";
  
  char hostname[NI_MAXHOST];
  gethostname(hostname, sizeof(hostname));
  printv(hostname);

  try {
    xmat::Socket server(xmat::IP_SERVER, xmat::PORT, 0);
    printv(server.ip_.begin());
    printv(server.port_.begin());
    
    server.set_options();

    print(1, "bind", 0, '-');
    server.bind();

    print(1, "listen", 0, '-');
    server.listen(4);

    const double timeout = 10;
    print_mv("accept. waiting for sec: ", timeout);
    xmat::Socket connection = server.accept(timeout);
    if (connection.is_open()) {
      print("connection successful:", 0, '-');
      printv(connection.ip_.begin());
      printv(connection.port_.begin());
    } else {
      print("connection failed:", 0, '-');
      return EXIT_FAILURE;
    }

    int out = connection.resv(buff, xmat::kDefaultMsgLen, 2.0);
    print_mv("connection.resv() for iter: ", out);
    if (out == -1) { print(1, "data wasn't accepted", 0, '*'); }
    else { print_mv("received msg: \n", buff); }

    const int out0 = connection.send(msg, xmat::kDefaultMsgLen);
    print_mv("connection.send for iter: ", out0);

  } // END BODY
  catch (std::exception& err) {
    print_mv("caught in main():\n >>", err.what());
  }

  print(1, "finish", 1, '-');
  return EXIT_SUCCESS;
}
