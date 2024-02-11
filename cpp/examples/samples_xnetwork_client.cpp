#include <iostream>
#include <exception>
#include <algorithm>

#include "../include/xmat/xnetwork.hpp"
#include "../include/xmat/xserialization.hpp"
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
  xout.add("x0", x0);
  xout.add("x1", x1);
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
  print("START client for echo xmat-server");
  print(__PRETTY_FUNCTION__);

  int out = 0;

  // make connection
  // ---------------
  print(1, "start connecting", 0, '-');
  xmat::Communication xcom(xmat::IP_LOCALHOST, xmat::PORT, 0);
  xcom.wait_for_connection(10.0);
  print("connected successfully", 0, '-');

  // start sending
  // =============
  // step 1
  // -------------
  print(1, "step 1", 0, '=');
  print(1, "serialize", 0, '-');
  xmat::Output xout_0{xmat::StreamBytes{}};
  xout_0.add("x0", int{8});
  xout_0.add("x1", std::complex<double>{.1, .2});
  xout_0.add("x2", std::vector<int>{4, 3, 2, 1, 0});
  xout_0.add("x3", std::string{"string type var"});
  xout_0.close();
  printv(xout_0.capacity());

  print(1, "deserialize", 0, '-');
  xmat::Input xint_0{xmat::StreamBytes{xmat::StreamMode::in, xout_0.buff(), xout_0.size()}};
  for (std::size_t n = 0, N = xint_0.content_.size(); n < N; ++n) {
    print(1, "Block: ", 0, '*');
    print_x(xint_0.content_[n]);
  }

  xcom.send(xout_0);
  print(1, "finish sending, start waiting for back message", 0, '-');
  xmat::Input xin_0{xmat::StreamBytes{xmat::StreamMode::in}};
  out = xcom.recv(xin_0, 10.0);
  if(out == 0) { throw std::runtime_error("back message wasn't received"); }
  print(1, "received successfully", 0, '-');

  bool res_0 = std::equal(xout_0.buff(), xout_0.buff() + xout_0.size(), xin_0.buff());
  if(!res_0) { throw std::runtime_error("received back data aren't equal"); }
  print(1, "compared successfully", 0, '-');

  // step 2
  // -------------

  xmat::Output xout_end{xmat::StreamBytes{}};
  xout_end.add("command", std::string{"stop"});
  xout_end.close();
  xcom.send(xout_end);

  print(1, "FINISH", 1, '=');
  return 1;
}

}


int main() {
  print("START: client" __FILE__, 1);
  
  try {
    // sample_0();
    sample_1();

  } // END BODY
  catch (std::exception& err) { 
    print_mv("caught in main():\n>>", err.what()); 
    EXIT_FAILURE;
  } 

  return EXIT_SUCCESS;
}
