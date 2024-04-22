#include <iostream>
#include <exception>

#include "../include/xmat/xnetwork.hpp"
#include "../include/xmat/xserial.hpp"
#include "common.hpp"


std::ostream* kOutStream = &std::cout;

namespace {
int sample_x() {
  print(__PRETTY_FUNCTION__, 1);
  print("COMMENT", 0, '-');

  print(1, "FINISH", 1, '=');
  return 1;
}

int sample_0() {
  print(__PRETTY_FUNCTION__, 1);
  print("start: xmat::TCPConnection/Client. for echo-server");

  print(1, "make client", 0, '-');
  xmat::TCPConnection client{xmat::IP_LOCALHOST,xmat::PORT, false};
  client.wait_for_connection(4.0);
  print(1, "client connected: ok", 0, '-');

  print(1, "start sending", 0, '-');
  print(1, "step 0", 0, '-');
  xmat::BugOut xout;
  xout.setitem("a0", int{1});
  xout.setitem("a2", int{2});
  xout.close();
  printv(xout.head().total_size);
  client.send(xout);
  print(1, "step 0: data sent", 0, '-');

  xmat::BugIn xin;
  if(!client.resv(xin, 10.0)) {
    print(1, "receiving back failed");
    throw std::runtime_error("error");
  }
  print(1, "receiving back: ok", 0, '-');

  print(1, "send command stop:", 0, '-');
  xmat::BugOut xout_end;
  xout_end.setitem("command", std::string("stop"));
  xout_end.close();
  client.send(xout_end);

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
