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
