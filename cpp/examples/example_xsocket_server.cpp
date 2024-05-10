/*
- example_0: ordered connection-byte          blocking        scheme-0
- example_1: ordered connection-bugout        blocking        scheme-0
- example_2: unordered connection-byte        non-blocking    scheme-1
- example_3: unordered connection-bugout      non-blocking    scheme-1
- example_4: ordered multi-connection-byte    blocking        scheme-2
- example_5: ordered multi-connection-bugout  blocking        scheme-2

ordered - A->B, B->A, A->B, B->A
  sockets sends massages strictly one after other. like echo

unordered - [A->B OR B->A], [A->B OR B->A], [A->B OR B->A], [A->B OR B->A]
  sockets sends massages in random order.

@startuml
title scheme-3: multi-connection

participant Listener0
participant Listener1
participant Socket0
participant Socket1
participant Client0
participant Client1

note over Listener0, Listener1 : wait()

Client0->Listener0 : connect()
activate Listener0
Listener0->Socket0 ** : accept()
deactivate Listener0

Client1->Listener1 : connect()
activate Listener1
Listener1->Socket1 ** : accept()
deactivate Listener1

note over Listener0, Listener1 : close()
@enduml

** use for diagramms: www.plantuml.com/plantuml/uml/
*/


#include <iostream>
#include <exception>

#include "../include/xmat/xsocket.hpp"
#include "../include/xmat/xserial.hpp"
#include "common.hpp"


std::ostream* kOutStream = &std::cout;

namespace {

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
*/
int example_0() {
  print(__PRETTY_FUNCTION__, 1);
  print("ordered connection-byte: scheme-0", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}

int example_1() {
  print(__PRETTY_FUNCTION__, 1);
  print("ordered connection-bugout: scheme-0", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}

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
int example_2() {
  print(__PRETTY_FUNCTION__, 1);
  print("unordered connection-byte: scheme-1", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}

int example_3() {
  print(__PRETTY_FUNCTION__, 1);
  print("unordered connection-bugout: scheme-1", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}

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
int example_4() {
  print(__PRETTY_FUNCTION__, 1);
  print("unordered connection-byte: scheme-1", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}

int example_5() {
  print(__PRETTY_FUNCTION__, 1);
  print("unordered connection-bugout: scheme-1", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}
}


int main() {
  print(2, __FILE__, 0, '-');
  print("xmat/xsocket.hpp examples: server", 0, '-');
  
  try {
    example_0();
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
