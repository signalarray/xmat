#include <cassert>
#include <iostream>
#include <fstream>

#include <cctype>
#include <iomanip>
#include <string>
#include <vector>
#include <array>
#include <complex>
#include <algorithm>
#include <type_traits>

#include "../include/xmat/xarr.hpp"
#include "../include/xmat/xserialization.hpp"

#include "common.hpp"


std::ostream* kOutStream = &std::cout;
std::string folder_data = R"(../../../../data/)"; // error: path is different for different "build" folders. need to fix

namespace {

void print_x(const xmat::Block& b) {
  printv(b.name_);
  printv(b.typename_);
  printv(b.shape_);
  printv(b.numel_);
  printv(int{b.typesize_});
  printv(int{b.ndim_});
}


int sample_0(){
  print("SAMPLE_0", 1);
  print("xmat::StreamFileOut");

  xmat::StreamFileOut ofile(folder_data + "file.txt");

  const char content[] = "zxcvbnm1234567890";
  ofile.write__(content, sizeof(content)-1);

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
  obytes.write__(content, 4);
  printv(obytes.vec_);
  printv(obytes.size_);

  obytes.write__(content + 4, 2);
  printv(obytes.vec_);
  printv(obytes.size_);

  print(1, "xmat::StreamBytes.In", 0, '-');
  std::array<xmat::byte_t, 2> buff;
  xmat::StreamBytes ibytes{xmat::StreamMode::in, obytes.buff(), obytes.size()};
  printv(ibytes.size_);
  ibytes.read__(buff.data(), buff.size());
  printv(buff);

  ibytes.read__(buff.data(), buff.size());
  printv(buff);

  print(1, "finish", 1, '-');
  return 1;
}


int sample_2(){
  print("SAMPLE_2", 1);
  print("xmat::StreamBase::write for different types");

  const char content[] = "zxcvbnm1234567890";
  printv(content);

  print(1, "xmat::StreamBytes.Out", 0, '-');
  xmat::StreamBytes obytes{};
  obytes.write__(content, 4);
  printv(obytes);

  obytes.write('X');
  obytes.write('Y');
  obytes.write('I');
  obytes.write('|');
  obytes.write(content, 8);
  printv(obytes);

  print(1, "xmat::Output", 0, '-');
  xmat::Output out{xmat::StreamBytes{}};

  print(1, "finish", 1, '-');
  return 1;
}


int sample_3(){
  print(__PRETTY_FUNCTION__, 1);
  print("xmat::Head, xmat::Block");
  
  print(1, "xmat::Head.dump()", 0, '-');
  xmat::StreamBytes osb{};
  xmat::Head h{};
  h.dump(osb);
  printv(osb);

  print(1, "xmat::Block.dump()", 0, '-');
  xmat::Block b0{};
  xmat::assign(b0.name_, "block_name");
  xmat::assign(b0.typename_, "default");
  b0.dump(osb);
  printv(osb);
  
  print(1, "xmat::Head.load()", 0, '-');
  xmat::StreamBytes isb{xmat::StreamMode::in, osb.begin(), osb.size()};
  xmat::Head h1{};
  h1.xint_size = 0;
  h1.max_block_name_len = 0;
  h1.max_type_name_len = 0;
  h1.load(isb);
  printv(h1.xint_size);
  printv(h1.max_block_name_len);
  printv(h1.max_type_name_len);
  
  print(1, "xmat::Block.load()", 0, '-');
  xmat::assign(b0.name_, "");
  xmat::assign(b0.typename_, "");
  b0.load(isb);
  printv(b0.name_.begin());
  printv(b0.typename_.begin());

  print(1, "finish", 1, '-');
  return 1;
}


int sample_4(){
  print(__PRETTY_FUNCTION__, 1);
  print("xmat::Head, xmat::Block, xmat::Output", 1);
  
  print(1, "Block default", 0, '-');
  xmat::Block b0{};
  print_x(b0);

  print(1, "Block{int}", 0, '-');
  const int x1 = 1;
  xmat::Block b1{x1};
  print_x(b1);

  print(1, "make xmat::Output dump:", 0, '=');
  xmat::Output dump{xmat::StreamBytes{}};
  print("end", 0, '-');
  printv(std::addressof(*dump.stream_));
  printv(std::addressof(*dump.stream_obj_.stream_));
  printv(std::addressof(dump.stream_obj_.stream_bytes_));

  printv((int)dump.stream_obj_.stream_->endian_);
  printv((char)dump.stream_obj_.stream_->mode_);

  print(1, "---------------", 1);
  xmat::StreamBytes osb0{};
  xmat::StreamBase* os_base = &osb0;
  printv(std::addressof(osb0));
  printv(std::addressof(*os_base));

  print(1, "xmat::Output::add(...)", 0, '-');
  xmat::Block b2{x1};
  xmat::Block b3 = dump.add_impl("a", b2);
  print_x(b2);

  print(1, "buffer content", 0, '-');
  printv(dump.stream_obj_.stream_bytes_);

  print(1, "finish", 1, '-');
  return 1;
}


int sample_5() {
  print(__PRETTY_FUNCTION__, 1);
  print("Output.add(...) and xmat::xblock(...)", 1);
  
  // scalars
  const int a0 = 9;
  std::complex<double> a1 = {0.25, 1.33};

  // arrays
  std::vector<int> x0 = {4, 3, 2, 1};
  std::vector<std::complex<double>> x1 = {{2, 1}, {4, 3}};
  xmat::Array<int> x2({2, 3});
  x2(0, 0) = 11; x2(0, 2) = 111;
  
  print(1, "VALUES: ", 0, '-');
  print(0, "scalars: ", 0, '-');
  printv(a0);
  printv(a1);

  print(1, "arrays: ", 0, '-');
  printv(x0);
  printv(x1);
  print("x2: ", 1);
  xmat::print(*kOutStream, x2, 4);

  print(1, "serialize", 0, '-');
  xmat::Output xout{xmat::StreamBytes{}};
  printvl(xout.stream_obj_.stream_bytes_);
  xout.add_impl("a0", xmat::XSerial<std::decay_t<decltype(a0)>>::dump(a0));
  xout.add_impl("a1", xmat::XSerial<std::decay_t<decltype(a1)>>::dump(a1));
  xout.add_impl("x0", xmat::XSerial<std::decay_t<decltype(x0)>>::dump(x0));
  xout.add_impl("x1", xmat::XSerial<std::decay_t<decltype(x1)>>::dump(x1));
  xout.add_impl("x2", xmat::XSerial<std::decay_t<decltype(x2)>>::dump(x2));
  xout.close();
  printv(xout.header_.total_size);
  printvl(xout.stream_obj_.stream_bytes_);

  print(1, "deserialize", 0, '-');
  xmat::Input xin{xmat::StreamBytes{xmat::StreamMode::in, 
                                    xout.stream_obj_.stream_bytes_.buff(),
                                    xout.stream_obj_.stream_bytes_.size()}};
  
  for (std::size_t n = 0, N = xin.content_.size(); n < N; ++n) {
    print(1, "Block: ", 0, '*');
    print_x(xin.content_[n]);
  }

  print(1, "finish", 1, '-');
  return 1;
}


int sample_6() {
  print(__PRETTY_FUNCTION__, 1);
  print("Input.at(...). test for general types", 1);
  
  // scalars
  const int a0 = 9;
  std::complex<double> a1 = {0.25, 1.33};
  char a2 = 'w';

  // std
  std::vector<int> a3 = {4, 3, 2, 1};
  std::vector<std::complex<double>> a4 = {{2, 1}, {4, 3}};
  std::array<float, 5> a5 = {-.2, -.1, 0.0, .1, .2};
  std::string a6 = "std::string C++";

  // array
  xmat::Array<int> a7({2, 3});
  a7(0, 0) = 11; a7(0, 2) = 111;
  
  print(1, "VALUES: ", 0, '-');
  print(0, "scalars: ", 0, '-');
  printv(a0);
  printv(a1);
  printv(a2);
  print(1, "std: ", 0, '-');
  printv(a3);
  printv(a4);
  printv(a5);
  printv(a6.c_str());
  print("a7: ", 1);
  xmat::print(*kOutStream, a7, 4);

  print(1, "serialize", 0, '-');
  xmat::Output xout{xmat::StreamBytes{}};
  printvl(xout.stream_obj_.stream_bytes_);
  xout.add("a0", a0);
  xout.add("a1", a1);
  xout.add("a2", a2);
  xout.add("a3", a3);
  xout.add("a4", a4);
  xout.add("a5", a5);
  xout.add("a6", a6);
  xout.add("a7", a7);
  xout.add("b0", 1.25 + 4.5);
  xout.add("b1", (4-5));
  xout.close();
  printv(xout.header_.total_size);
  printvl(xout.stream_obj_.stream_bytes_);

  print(1, "deserialize", 0, '-');
  xmat::Input xin{xmat::StreamBytes{xmat::StreamMode::in, xout.buff(), xout.size()}};
  
  for (std::size_t n = 0, N = xin.content_.size(); n < N; ++n) {
    print(1, "Block: ", 0, '*');
    print_x(xin.content_[n]);
  }

  print(1, "get values from xInput", 0, '-');
  assert(xin.is<int>("a0"));
  assert(xin.is<std::vector<int>>("a0"));
  assert((xin.is<std::array<int, 1>>("a0")));
  auto A0 = xin.at<int>("a0");
  // -------------------------
  assert(xin.is<std::complex<double>>("a1"));
  assert(xin.is<std::vector<std::complex<double>>>("a1"));
  assert((xin.is<std::array<std::complex<double>, 1>>("a1")));  
  auto A1 = xin.at<std::complex<double>>("a1");
  // ------------------------------------------
  auto A2 = xin.at<char>("a2");
  auto A3 = xin.at<std::vector<int>>("a3");
  auto A4 = xin.at<std::vector<std::complex<double>>>("a4");
  auto A5 = xin.at<std::array<float, 5>>("a5");
  auto A6 = xin.at<std::string>("a6");
  auto A7 = xin.at<xmat::Array<int>>("a7");
  auto B0 = xin.at<double>("b0");
  auto B1 = xin.at<int>("b1");

// 
  printv(A0);
  printv(A1);
  printv(A2);
  printv(A3);
  printv(A4);
  printv(A5);
  printv(A6.c_str());
  print("A7: ", 1);
  xmat::print(*kOutStream, A7, 4);
  printv(B0);
  printv(B1);

  print(1, "load one more time to current var", 0, '-');
  xin.at("a0", A0);
  xin.at("a1", A1);
  xin.at("a2", A2);
  xin.at("a3", A3);
  xin.at("a4", A4);
  
  printv(A0);
  printv(A1);

  print(1, "finish", 1, '-');
  return 1;
}
} // namespace


int main() {
  try {
  print("START: " __FILE__, 0, '=');
  // sample_0();
  // sample_1();
  // sample_2();
  // sample_3();
  // sample_4();
  // sample_5();
  sample_6();
  } 
  catch (std::exception& err) {
    print("exception in main():\n");
    print(err.what(), 1);
  }
  print("END: " __FILE__, 0, '=');
  return EXIT_SUCCESS;
}
