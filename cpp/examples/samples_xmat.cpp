#include <iostream>
#include <fstream>

#include <cctype>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <array>
#include <complex>
#include <algorithm>
#include <type_traits>

#include "../include/xmat/xarr.hpp"
#include "../include/xmat/xmat.hpp"
#include "common.hpp"


std::ostream* kOutStream = &std::cout;
std::string folder_data = R"(../../../../data/)"; // error: path is different for different "build" folders. need to fix

namespace {

template<typename T>
xmat::Block xblock(const T& X) {
  xmat::Block block;
  xmat::TypeInfo<T> info;
  xmat::assign(block.typename_, info.name);
  block.shape_ = {0};
  block.numel_ = 1;
  block.typesize_ = info.size;
  block.ndim_ = 0;
  block.ptr_ = reinterpret_cast<const char*>(&X);
  return block;
}


template<typename T, typename A>
xmat::Block xblock(const std::vector<T, A>& x) {
  xmat::Block block;
  xmat::TypeInfo<T> info;
  xmat::assign(block.typename_, info.name);
  block.shape_ = {x.size()};
  block.numel_ = 1;
  block.typesize_ = info.size;
  block.ndim_ = 1;
  block.ptr_ = reinterpret_cast<const char*>(x.data());
  return block;
}


template<typename T>
xmat::Block xblock(const xmat::Array<T>& x) {
  xmat::Block block;
  xmat::TypeInfo<T> info;
  xmat::assign(block.typename_, info.name);
  assert(x.map.ndim <= block.shape_.size());
  std::copy_n(x.map.shape.begin(), x.map.ndim, block.shape_.begin());
  block.numel_ = x.map.numel;
  block.typesize_ = info.size;
  block.ndim_ = x.map.ndim;
  block.ptr_ = reinterpret_cast<const char*>(x.data);
  return block;
}


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
  xmat::Block b3 = dump.add("a", b2);
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
  xout.add("a0", xblock(a0));
  xout.add("a1", xblock(a1));
  xout.add("x0", xblock(x0));
  xout.add("x1", xblock(x1));
  xout.add("x2", xblock(x2));
  xout.close();
  printv(xout.header_.total_size);
  printvl(xout.stream_obj_.stream_bytes_);

  print(1, "deseirialize", 0, '-');
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
} // namespace


int main() {
  print("START: " __FILE__, 0, '=');
  // sample_0();
  // sample_1();
  // sample_2();
  // sample_3();
  // sample_4();
  sample_5();
  print("END: " __FILE__, 0, '=');
  return EXIT_SUCCESS;
}
