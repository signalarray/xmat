#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <array>

#include "../include/xmat/xdatastream.hpp"
#include "../include/xmat/xserial.hpp"
#include "../include/xmat/xprint.hpp"
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
  print("COMMENT", 0, '-');

  printv(xmat::Endian::native == xmat::Endian::big);
  printv(xmat::Endian::native == xmat::Endian::little);
  // auto y = xmat::pack(0.0);
  int a  = 1024 >> 5;
  printv(xmat::isle());

  double xd = 1024.0;
  xmat::print_bytes(*kOutStream, *reinterpret_cast<std::int64_t*>(&xd));

  int x = 1024;
  auto y = xmat::xbswap(x);
  printv(x);
  printv(y);
  xmat::print_bytes(*kOutStream, x);
  xmat::print_bytes(*kOutStream, y);

  print(1, "xmat::Pack<Endian>: ", 0, '-');
  xmat::Pack<xmat::Endian::big> to_big{};
  xmat::Pack<xmat::Endian::little> to_little{};
  xmat::Pack<xmat::Endian::native> to_native{};
  printv(to_big.repack(1));
  printv(to_little.repack(1));
  printv(to_native.repack(1));

  printv(to_big.repack(3.14));
  printv(to_little.repack(3.14));

  printv(to_big.repack(3.14f));
  printv(to_little.repack(3.14f));

  print(1, "xmat::Unpack<Endian>: ", 0, '-');
  xmat::Unpack<xmat::Endian::big> from_big{};
  xmat::Unpack<xmat::Endian::little> from_little{};
  xmat::Unpack<xmat::Endian::native> from_native{};
  printv(from_big.repack(to_big.repack(1)));
  printv(from_little.repack(to_little.repack(1)));
  printv(from_native.repack(to_native.repack(1)));

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_1() {
  print(__PRETTY_FUNCTION__, 1);
  print("endian buffers", 0, '-');

  xmat::ODStream<xmat::Endian::native> ods;
  xmat::XHead h;
  xmat::XBlock b;
  h.dump(ods);

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_2() {
  print(__PRETTY_FUNCTION__, 1);
  print("xmat::OMapStream<>", 0, '-');
  using xmat::Endian;

  print(1, "xmat::OMapStream", 0, '-');
  xmat::OMapStream<Endian::native> xout{};
  xout.setitem("a0", 1);
  xout.setitem("a1", -2);
  xout.close();
  printv(xout.head().total());

  print(1, "xmat::IMapStream", 0, '-');
  auto xoutms = xout.stream().get_memsource();
  xmat::IDStreamMS<Endian::native> xin_ibb{&xoutms};
  xin_ibb.push_all();
  xmat::IMapStreamMS<Endian::native> xin{std::move(xin_ibb)};
  printv(xin.head().total());
 
  print(1, "xmat::IMapStream.begin(), end()", 0, '-');
  auto it = xin.begin();
  auto end = xin.end();
  printv(it->pos_);
  printv(end->pos_);
  for (;it != end; ++it) {
    printv(it->pos_);
    printv(it->name());
  }
 
  print(1, "xmat::IMapStream. range loop", 0, '-');
  for(auto& item : xin) { printv(item.name()); }

  print(1, "xmat::IMapStream. element access", 0, '-');
  auto block_b0 = xin.at("b0");
  assert(!block_b0.is_valid());
  printv(block_b0.is_valid());

  auto block_a0 = xin.at("a0");
  printv(block_a0.is_valid());
  if (block_a0){
    printv(block_a0->name());
  }

  print(1, "get<T>", 0, '-');
  printv(block_a0->pos());
  printv(block_a0->nbytes());
  printv(block_a0->data_pos());
  auto a0_back = block_a0.get<int>();
  printv(block_a0.get<int>());

  print(1, "get_to<T>", 0, '-');
  int xx = -1;
  printv(xx);
  printv(block_a0.get_to(xx));
  printv(xin.at("a1").get_to(xx));
  
  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_3() {
  print(__PRETTY_FUNCTION__, 1);
  print("COMMENT", 0, '-');

  print(1, "xmat::OMatStream", 0, '-');
  xmat::OMapStream<xmat::Endian::native> xout{};
  printv(xout.stream().is_open());
  
  print(1, "scalar", 0, '-');
  xout.setitem("a0", 4);
  
  // ----
  print(1, "pointer", 0, '-');
  const size_t N = 8;
  int b0[N] = {1, 2, 3, 4, 5, 6, 7, 8};
  char b1[N] = "absdef";
  xout.setitem_n("b0", b0, N);
  xout.setitem_n("b1", b1, N);

  // ----
  print(1, "std::1d containers", 0, '-');
  xout.setitem("c0", std::vector<int>{11, 22, 33, 44, 55, 66});
  xout.setitem("c1", std::string{"string std string"});
  xout.setitem("c2", std::array<int, N>{1, 2, 3, 4, 5});
  
  // ----
  print(1, "xmat::Array", 0, '-');
  auto d0 = xmat::NArray<int, 1>{{6}};
  auto d1 = xmat::NArray<int, 2>{{2, 6}};
  d0.enumerate();
  d1.enumerate();
  d0.at(2) = -1;
  d1.at(1, 3) = -21;
  printv(d0);
  printv(d1);
  for (auto it = d0.wbegin(), end = d0.wend(); it != end; ++it) {
    for (auto it_ : it) *kOutStream << it_ << ", ";
  }
  *kOutStream << "\n";

  xout.setitem("d0", d0);
  xout.setitem("d1", d1);

  xout.close();
  printv(xout.head().total());

  // ---------------------------
  print(1, "xmat::IMapStream", 0, '-');
  auto xoutms = xout.stream().get_memsource();
  xmat::IDStreamMS<xmat::Endian::native> xin_ibb{&xoutms};
  xin_ibb.push_all();
  xmat::IMapStreamMS<xmat::Endian::native> xin{std::move(xin_ibb)};
  printv(xin.head().total());
  printv(xin);

  // ----
  print(1, "xin. scalar", 0, '-');
  auto a0_ = xin.at("a0").get<int>();
  printv(a0_);

  // ----
  print(1, "xin. pointer", 0, '-');
  std::vector<int> b0_(N);
  xin.at("b0").get_to(b0_.data());
  printv(b0_);
  
  std::string b1_(N, 'x');
  printv(b1_.c_str());
  xin.at("b1").get_to(&b1_[0]);
  printv(b1_.c_str());

  // ----
  print(1, "xin. std", 0, '-');
  auto c0_ = xin.at("c0").get<std::vector<int>>();
  printv(xin.at("c0").get_to(c0_));
  
  auto c1_ = xin.at("c1").get<std::string>();
  printv(xin.at("c1").get_to(c1_));
  
  auto c2_ = xin.at("c2").get<std::array<int, N>>();
  printv(xin.at("c2").get_to(c2_));

  // ----
  print(1, "xin. xmat::Array", 0, '-');
  auto d0_ = xin.at("d0").get<xmat::NArray<int, 1>>();
  printv(d0_);

  auto d1_ = xin.at("d1").get<xmat::NArray<int, 2>>();
  *kOutStream << d1_;
  printv(d1_);
  //
  print(1, "FINISH", 1, '=');
  return 1;
}
}


int main() {
  print("SAMPLES: " __FILE__, 1);

  try {
    sample_3();
  }
  catch (std::exception& err) {
    print(1, "----------------------\n");
    print_mv("caught in main():\n>> ", err.what());
    EXIT_FAILURE;
  } 

  return EXIT_SUCCESS;
}
