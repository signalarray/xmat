/* 
@startuml
title scheme-0: ordered connection

note over Socket, Client: connect()
participant Socket
participant Client
... iter0 ...
Client->Socket : msg[0]
Socket->Client : re.msg[0]
... iter1 ...
Client->Socket : msg[1]
Socket->Client : re.msg[1]
... iter2 ...
Client->Socket : msg[2]
Socket->Client : re.msg[2]
... stop ...
Client->Socket : command.stop
note over Socket, Client: close()
@enduml


*use for show diagramm: www.plantuml.com/plantuml/uml
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

    // receive
    size_t n = 0, k = 0;
    while(true) {
      const int count = tcp.wait(0.0, std::nothrow);
      if (count) {
        // receive and process the message if ready
        socket->recvall(buf, xmat::k_xsbuf_size, 1.0);
        cout << "message num: " << n++ << ": " << buf << "\n";
        if (!std::strcmp(buf, "close")) {
          cout << "command `close` received\n";
          break;
        }

        // send msg back
        const char* tail = ".reply";
        std::copy_n(tail, std::strlen(tail), buf + std::strlen(buf));
        socket->sendall(buf, xmat::k_xsbuf_size, 1.0);

        // --
        std::fill_n(buf, xmat::k_xsbuf_size, '\0');
        k = 0;
      } else {
        // do some other job
        xmat::time::sleep(0.125);
        cout << "other-job: " << k++ << "\n";
      }
    }
    xmat::time::sleep(8.0);
    cout << "finish successfully\n";
  }
  catch (std::exception& err) {
    cout << "exception in main: message: >>\n" 
         << err.what();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
