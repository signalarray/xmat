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

#include <iostream>
#include <exception>

#include "../../include/xmat/xsocket.hpp"
#include "../../include/xmat/xserial.hpp"
#include "../../include/xmat/xprint.hpp"
#include "../common.hpp"


std::ostream* kOutStream = &std::cout;


int main() {
  print(2, __FILE__, 0, '-');
  
  char buf[xmat::k_xsbuf_size + 1] = {};

  try {
    // accept
    xmat::TCP_<1, 1> tcp{};
    auto listener = tcp.listener(xmat::k_xsport);
    tcp.wait(10.0);
    auto socket = tcp.accept(listener);
    printv(socket->remoteaddress());

    // read-write
    size_t n = 0, cnt_job = 0;
    while(true) {
      const int count = tcp.wait(0.0, std::nothrow);
      if (count) {
        socket->recvall(buf, xmat::k_xsbuf_size, 1e-3);
        *kOutStream << "msg num: " << n << ": " << buf << "\n";
        cnt_job = 0;
      } else {
        // do some other job
        xmat::time::sleep(1.0);
        *kOutStream << "other-job: " << cnt_job << "\n";
      }
    }

    print(1, "successfully finish", 1, '=');
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
