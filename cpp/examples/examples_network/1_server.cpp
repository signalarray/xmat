/*
@startuml
title scheme-1: unordered connection

participant Socket
participant Client

Client->Socket : msg[0]
Socket->Client : re.msg[0]

group Socket busy(8 sec)
 Client->Socket : msg[1]
 note over Client: wait(for reply, 1 sec)
 activate Client

 Client->Socket : msg[2]
 note over Client: wait(for reply, 1 sec)
 Client->Socket : msg[3]
 note over Client: wait(for reply, inf)
end

Socket->Client : re.msg[1]
Socket->Client : re.msg[2]
Socket->Client : re.msg[3]
deactivate Client
@enduml
*/

#include <cstring>
#include <iostream>
#include <algorithm>
#include <exception>

#include "../../include/xmat/xsocket.hpp"
#include "../../include/xmat/xserial.hpp"
#include "../../include/xmat/xprint.hpp"


int main() {
  using std::cout;
  char buf[xmat::k_xsbuf_size + 1] = {};

  try {
    // accept connection
    xmat::TCP tcp{};
    auto listener = tcp.listener(xmat::k_xsport);
    tcp.wait(2.0);
    auto socket = tcp.accept(listener);
    tcp.remove(listener); // not necessary
    cout << "connection success\n";
    cout << socket->remoteaddress() << "\n";

    // socket busy
    // ----------------------------
    cout << "sleeps for a while\n";
    xmat::time::sleep(2.0);

    // receive
    cout << "receive all messages\n";
    size_t n = 0;
    while(true) {
      const int count = tcp.wait(1.0, std::nothrow);
      if (!count) {
        break;
      }
      xmat::IMapStream<> xin{};
      socket->recv(xin, 1.0);
      if (xin.at("stop") != xin.end()) {
        cout << "\n**command `stop` reveiced\n";
        break;
      }

      auto field_n = xin.at("n").get<std::uint64_t>();
      auto field_msg = xin.at("msg").get<std::string>();
      cout << "\n" 
           << "xin.at(`n`).get<std::uint64_t>() " << field_n << "\n"
           << "xin.at(`msg`).get<std::string>() " << field_msg << "\n";

      xmat::OMapStream<> xout{};
      xout.setitem("n", field_n);
      xout.setitem("msg", field_msg + ".reply");
      xout.close();
      socket->send(xout, 1.0);
    }

    cout << "finish successfully\n";
  }
  catch (std::exception& err) {
    cout << "\nexception in main: message: >>\n" 
         << err.what();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}