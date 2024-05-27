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
