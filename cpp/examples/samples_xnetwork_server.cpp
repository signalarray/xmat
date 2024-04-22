#include <iostream>
#include <string>
#include <exception>

#include "../include/xmat/xnetwork.hpp"
#include "../include/xmat/xserial.hpp"
#include "common.hpp"


std::ostream* kOutStream = &std::cout;

namespace {

void print_x(const xmat::StreamBlock& b) {
  printv(b.name_);
  printv(b.typename_);
  printv(b.shape_);
  printv(b.numel_);
  printv(int{b.typesize_});
  printv(int{b.ndim_});
}


int sample_x() {
  print(__PRETTY_FUNCTION__, 1);
  print("COMMENT", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}

int sample_0() {
  print(__PRETTY_FUNCTION__, 1);
  print("start: xmat::TCPServer. echo");

  print(1, "make server", 0, '-');
  xmat::TCPServer server{xmat::PORT, false};
  printv(server.socket_.ip());
  printv(server.socket_.port());

  xmat::TCPConnection xcon = server.wait_for_connection(10.0);
  print(1, "connection accepted:", 0, '-');
  printv(xcon.socket_.ip());
  printv(xcon.socket_.port());
  
  while (true) {
    xmat::BugIn xin;
    if(!xcon.resv(xin, 4.0)) continue;

    auto command = xin.at("command");
    if (command) {
      auto str = command.get<std::string>();
      print(1, "command received:", 0, '-');
      printv(str.c_str());

      if(str == "stop") {
        print("server was closed by [command]: 'stop'");
        break;
      }
    }

    print(1, "received content:", 0, '-');
    for (auto& block : xin) print_x(block);

    print(1, "send back:", 0, '-');
    xcon.send(xin.buf().data(), xin.buf().size());
  }

  print(1, "FINISH", 1, '=');
  return 1;
}
}


int main() {
  print("START: server\n" __FILE__, 1);
  
  try {
    sample_0();

  }
  catch (std::exception& err) { 
    print(1, "----------------------\n");
    print_mv("caught in main():\n>> ", err.what()); 
    EXIT_FAILURE;
  } 

  return EXIT_SUCCESS;
}
