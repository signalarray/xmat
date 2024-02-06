#include <iostream>
#include <exception>

#include "../include/xmat/xnetwork.hpp"
#include "common.hpp"


std::ostream* kOutStream = &std::cout;

namespace {

void print_x(const xmat::Block& b) {
  printv(b.name_);
  printv(b.typename_);
  printv(b.shape_);
  printv(b.numel_);
  printv(int{b.typesize_});
  printv(int{b.ndim_});
}


int sample_0() {
  print("START: ");
  print(__PRETTY_FUNCTION__, 1);
  
  print(1, "start connecting", 0, '-');
  xmat::Communication xcom(xmat::IP_SERVER, xmat::PORT, 0);
  bool out = xcom.wait_for_connection(10.0, true);
  print("connected successfully", 0, '-');
  printv(xcom.connection_->ip());
  printv(xcom.connection_->port());

  xmat::Input xin{xmat::StreamBytes{xmat::StreamMode::in}};
  xcom.recv(xin, 10.0);
  print("received", 0, '-');
  printv(xin.header_.total_size);
  printvl(xin.stream_obj_.stream_bytes_);
  printv(xin.capacity());

  for (std::size_t n = 0, N = xin.content_.size(); n < N; ++n) {
    print(1, "Block: ", 0, '*');
    print_x(xin.content_[n]);
  }
  
  print(1, "FINISH", 1, '=');
  return 1;
}

int sample_1() {
  return 1;
}

}


int main() {
  print("START: server\n" __FILE__, 1);
  
  try {
    sample_0();

  } // END BODY
  catch (std::exception& err) { 
    print_mv("caught in main():\n>>", err.what()); 
    EXIT_FAILURE;
  } 

  return EXIT_SUCCESS;
}
