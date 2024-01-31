#include <iostream>
#include <exception>

#include "../include/xmat/xsocket.hpp"
#include "common.hpp"


std::ostream* kOutStream = &std::cout;


int main() {
  print("START: " __FILE__, 1);
  print(__PRETTY_FUNCTION__, 1);  
  
  std::array<char, xmat::kDefaultMsgLen> buff{};
  int msg_len = 0, out = 0;

  try {
    xmat::Socket client(xmat::IP_LOCALHOST, xmat::PORT, 0);
    printv(client.ip_.begin());
    printv(client.port_.begin());
    
    print(1, "connect", 0, '-');
    client.connect(2.0);
    
    // -------------------
    xmat::impl::assign(buff, "first message from C++");
    msg_len = std::strlen(buff.begin());

    print(1, "send", 0, '-');
    out = client.send(buff.begin(), msg_len);
    print_mv("client.send() for iter: ", out);

    print(1, "receive back", 0, '-');
    out = client.resv(buff.begin(), msg_len, 2.0);
    print_mv("client.resv() for iter: ", out);
    if (out == -1) { print(1, "data wasn't accepted", 0, '*'); }
    else { print_mv("[msg received]: \n", buff.begin()); }


    // -------------------
    xmat::impl::assign(buff, "second message from C++");
    msg_len = std::strlen(buff.begin());

    print(1, "send", 0, '-');
    out = client.send(buff.begin(), msg_len);
    print_mv("client.send() for iter: ", out);

    print(1, "receive back", 0, '-');
    out = client.resv(buff.begin(), msg_len, 2.0);
    print_mv("client.resv() for iter: ", out);
    if (out == -1) { print(1, "data wasn't accepted", 0, '*'); }
    else { print_mv("[msg received]: \n", buff.begin()); }

    // --------------------
    xmat::impl::assign(buff, "stop");
    msg_len = std::strlen(buff.begin());
    print(1, "send command `stop`", 0, '-');
    out = client.send(buff.begin(), msg_len);
    print_mv("client.send() for iter: ", out);

  } // END BODY
  catch (std::exception& err) {
    print_mv("caught in main():\n >>", err.what());
  }

  return EXIT_SUCCESS;
}
