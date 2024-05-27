/*
@startuml
title scheme-2: ordered multi-connection

participant Socket0
participant Socket1
participant Client0
participant Client1

... ordered ...
note over Socket0, Socket1 : wait()
Client0->Socket0 : msg[0]
activate Client0
Socket0->Client0 : re.msg[0]
deactivate Client0

note over Socket0, Socket1 : wait()
Client1->Socket1 : msg[1]
activate Client1
Socket1->Client1 : re.msg[1]
deactivate Client1

... mixed order ...
group Sockets sleep(timeout)
 Client0->Socket0 : msg[2]
 activate Client0
 Client1->Socket1 : msg[3]
 activate Client1
end

note over Socket0, Socket1 : ...\nrecv()
/ note over Client0, Client1 : wait()
Socket0->Client0  : re.msg[2]
deactivate Client0
Socket1->Client1  : re.msg[3]
deactivate Client1
@enduml
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
  
  try {
    

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
