#include <iostream>
#include <exception>

#include "../include/xmat/xnetwork.hpp"
#include "../include/xmat/xformat_spec.hpp"
#include "common.hpp"


std::ostream* kOutStream = &std::cout;

void print_x(const xmat::Block& b) {
  printv(b.name_);
  printv(b.typename_);
  printv(b.shape_);
  printv(b.numel_);
  printv(int{b.typesize_});
  printv(int{b.ndim_});
}


namespace {

int sample_0() {
  print("START: ");
  print(__PRETTY_FUNCTION__);
  
  std::vector<int> x0 = {1, 2, 3};
  std::string x1 = "string message";

  print(1, "serialize", 0, '-');
  xmat::Output xout{xmat::StreamBytes{}};
  xout.add("x0", xmat::xblock(x0));
  xout.add("x1", xmat::xblock(x1));
  xout.close();
  printv(xout.header_.total_size);
  printvl(xout.stream_obj_.stream_bytes_);
  printv(xout.capacity());

  print(1, "deserialize", 0, '-');
  xmat::Input xin{xmat::StreamBytes{xmat::StreamMode::in, xout.buff(), xout.size()}};

  for (std::size_t n = 0, N = xin.content_.size(); n < N; ++n) {
    print(1, "Block: ", 0, '*');
    print_x(xin.content_[n]);
  }

  print(1, "start connecting", 0, '-');
  xmat::Communication xcom(xmat::IP_LOCALHOST, xmat::PORT, 0);
  bool out = xcom.wait_for_connection(10.0);
  print("connected successfully", 0, '-');
  xcom.send(xout);

  print(1, "FINISH", 1, '=');
  return 1;
}

int sample_1() {
  return 1;
}

}


int main() {
  print("START: client" __FILE__, 1);
  
  try {
    sample_0();

  } // END BODY
  catch (std::exception& err) { 
    print_mv("caught in main():\n>>", err.what()); 
    EXIT_FAILURE;
  } 

  return EXIT_SUCCESS;
}
