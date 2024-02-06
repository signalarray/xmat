#include <iostream>
#include <exception>

#include "../include/xmat/xsocket.hpp"
#include "common.hpp"


std::ostream* kOutStream = &std::cout;

int main() {
  print("START: " __FILE__, 1);
  print("echo server C++ started: comand `stop` to close the server", 1, '=');
  print_mv("max message len is: ", xmat::kDefaultMsgLen-1);
  printv(xmat::IP_SERVER);

  std::array<char, xmat::kDefaultMsgLen> buff{};
  int msg_len = 0, out = 0;
  
  char hostname[NI_MAXHOST];
  gethostname(hostname, sizeof(hostname));
  printv(hostname);

  try {
    xmat::Socket server(xmat::IP_SERVER, xmat::PORT, 0);
    printv(server.ip_.begin());
    printv(server.port_.begin());
    
    // init server
    // -----------
    server.set_options();

    print(1, "bind", 0, '-');
    server.bind();

    print(1, "listen", 0, '-');
    server.listen(4);


    // wait for client
    // ---------------
    const double timeout = 10;
    print_mv("accept. waiting for sec: ", timeout);
    xmat::Socket connection = server.accept(timeout);
    if (connection.is_open()) {
      print("connection successful:", 0, '-');
      printv(connection.ip_.begin());
      printv(connection.port_.begin());
    } else {
      print(1, "connection failed:", 0, '*');
      return EXIT_FAILURE;
    }

    print(1, "main start loop", 0, '=');
    int num = 0;
    while(true) { // main loop
      int n0 = connection.num_bytes_available();
      print_mv("\nnum_bytes_available: ", n0);
      std::this_thread::sleep_for(std::chrono::milliseconds{100});

      num = connection.recv_all(buff.begin(), buff.size()-1, 1.0);
      if (num > 0) {
        buff[num] = '\0';
              // if command `stop`
        if (std::strcmp(buff.begin(), "stop") == 0) {
          print(1, "[command]: `stop`: server closed");
          break;
        }
        
        print_mv("[data]:", buff.begin());
        msg_len = std::strlen(buff.begin());
        out = connection.send(buff.begin(), msg_len);
        print_mv("connection.send() for iter: ", out);
      }
    }

  } // END BODY
  catch (std::exception& err) {
    print_mv("caught in main():\n>>", err.what());
  }

  print(1, "finish", 1, '-');
  return EXIT_SUCCESS;
}
