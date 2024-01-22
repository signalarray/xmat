#include <iostream>
#include <fstream>

#include <cctype>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <array>
#include <type_traits>

#include "../include/xmat/xarr.hpp"
#include "../include/xmat/xmat.hpp"
#include "common.hpp"


std::ostream* kOutStream = &std::cout;
std::string folder_data = R"(../../../../data/)"; // error: path is different for different "build" folders. need to fix 

namespace {

int sample_0(){
  print("SAMPLE_0", 1);
  print("xmat::StreamFileOut");

  xmat::StreamFileOut ofile(folder_data + "file.txt");

  const char content[] = "zxcvbnm1234567890";
  ofile.write(content, sizeof(content)-1);

  print(1, "finish", 1, '-');
  return 1;
}


int sample_1(){
  print("SAMPLE_1", 1);
  print("xmat::StreamBytes");

  const char content[] = "zxcvbnm1234567890";
  printv(content);

  print(1, "xmat::StreamBytes.Out", 0, '-');
  xmat::StreamBytes obytes{};
  obytes.write(content, 4);
  printv(obytes.vec_);
  printv(obytes.size_);

  obytes.write(content + 4, 2);
  printv(obytes.vec_);
  printv(obytes.size_);

  print(1, "xmat::StreamBytes.In", 0, '-');
  std::array<xmat::byte_t, 2> buff;
  xmat::StreamBytes ibytes{xmat::StreamMode::in, obytes.buff(), obytes.size()};
  printv(ibytes.size_);
  ibytes.read(buff.data(), buff.size());
  printv(buff);

  ibytes.read(buff.data(), buff.size());
  printv(buff);

  print(1, "finish", 1, '-');
  return 1;
}

} // namespace


int main() {
  print("START: " __FILE__, 0, '=');
  sample_0();
  sample_1();
  print("END: " __FILE__, 0, '=');
  return EXIT_SUCCESS;
}
