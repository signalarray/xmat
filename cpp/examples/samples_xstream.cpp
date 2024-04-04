#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <array>

#include "common.hpp"
#include "../include/xmat/xstream.hpp"
#include "../include/xmat/xserial.hpp"

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
  print("bbuf_stor_..", 0, '-');

  print(1, "bbuf_storage_dyn", 0, '-');
  xmat::BBufStorage_<xmat::bbuf_memsource_default> bbdyn;
  for (int n = 0; n < 8; ++n) {
    print_mv("n :", n);
    bbdyn.size_request(n);
    printv(bbdyn.size());
  }

  print(1, "bbuf_storage_memsource", 0, '-');
  auto gmemsrc = &xmat::GlobalMemSource::reset(5);
  xmat::BBufStorage_<xmat::MemSourceAlloc<xmat::xbyte_t>> bbms{gmemsrc};
  for (int n = 0; n < 8; ++n) {
    print_mv("n :", n);
    bbms.size_request(n);
    printv(bbms.size());
  }

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_1() {
  print(__PRETTY_FUNCTION__, 1);
  print("obbuf, ibbuf", 0, '-');

  print(1, "obbuf<bbuf_storage_std>", 0, '-');
  xmat::OBBuf obb;
  const char line1[] = "line 1. ";
  const char line2[] = "line 2";

  obb.write(line1, sizeof(line1) - 1);
  obb.write(line2, sizeof(line2));
  printv(obb.storage_.data())

  print(1, "obbuf<bbuf_storage_ms>", 0, '-');
  auto gmemsrc = &xmat::GlobalMemSource::reset(1 << 10);
  xmat::OBBufMS obbms{gmemsrc};

  obbms.write(line1, sizeof(line1) - 1);
  obbms.write(line2, sizeof(line2));
  printv(obbms.storage_.data())

  print(1, "ibbuf<bbuf_storage_std>", 0, '-');
  const char iline[] = "1234567890zxcvbnm,../asdfghjkl";
  char buf[8] = "asd";
  xmat::IBBuf ibb;
  ibb.push(iline, sizeof(iline));
  printv(ibb.size());
  printv(ibb.storage().data());

  ibb.read(buf, 4);
  printv(buf);

  ibb.read(buf, 4);
  printv(buf);

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_2() {
  print(__PRETTY_FUNCTION__, 1);
  print("xmat::Head", 0, '-');

  print(1, "xmat::obbuf<>", 0, '-');
  xmat::StreamHead h0;
  h0.total_size = -1;
  h0.max_ndim = 16;
  xmat::OBBuf obb;
  h0.dump(obb);

  print(1, "xmat::ibbuf<memsource>", 0, '-');
  xmat::MemSource obb_smem = obb.get_memsource();
  printv(obb_smem.size());
  xmat::IBBufMS ibb{&obb_smem};
  printv(ibb.size());
  ibb.push_all();
  printv(ibb.size());
  printv(ibb.tellg());
  printv(ibb.cursor_);
  xmat::StreamHead h1;
  printv((int)h1.max_ndim);
  h1.load(ibb);
  printv((int)h1.max_ndim);

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_3() {
  print(__PRETTY_FUNCTION__, 1);
  print("xmat::bugout<>", 0, '-');

  print(1, "xmat::bugout_ms xout{gmemsrc}", 0, '-');
  auto gmemsrc = &xmat::GlobalMemSource::reset(1 << 10);
  // xmat::bugout_ms xout{gmemsrc};
  // xmat::bugout_ms xout{{gmemsrc}};
  xmat::BugOutMS xout{xmat::OBBufMS{gmemsrc}};
  xout.setitem("a0", 1);
  xout.setitem("a1", 2);
  xout.close();
  printv(xout.head_.total_size);

  print(1, "xmat::bugin_ms xin{std::move(xin_ibb)};", 0, '-');
  auto xout_ms = xout.buf().get_memsource();
  xmat::IBBufMS xin_ibb{&xout_ms};
  xmat::BugInMS xin{std::move(xin_ibb)};
  xin.buf().push_all();
  xin.scan_head();
  printv(xin.head_.total_size);

  print(1, "xmat::bugin_.begin(), end()", 0, '-');
  auto it = xin.begin();
  auto end = xin.end();
  printv(end->pos_);
  for (;it != end; ++it) {
    printv(it->pos_);
    printv(it->name_);
  }

  print(1, "xmat::bugin_xx. range loop", 0, '-');
  for(auto& item : xin) { printv(item.name_); }

  print(1, "xmat::bugin_xx. element access", 0, '-');
  auto block_b0 = xin.at("b0");
  printv(block_b0.empty());

  auto block_a0 = xin.at("a0");
  printv(block_a0.empty());
  if (block_a0){
    printv(block_a0.name());
    printv(block_a0.numel());
  }

  print(1, "get<T>", 0, '-');
  auto a0_back = block_a0.get<int>();
  printv(block_a0.get<int>());

  print(1, "get_to<T>", 0, '-');
  int xx = -1;
  printv(xx);
  printv(block_a0.get_to(xx));

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_4() {
  print(__PRETTY_FUNCTION__, 1);
  print("xmat:serial. types", 0, '-');

  print(1, "write output", 0, '-');
  xmat::BugOut xout{};
  printv(xout.buf().is_open());
  
  print(1, "scalar", 0, '-');
  xout.setitem("a0", 4);

  print(1, "pointer", 0, '-');
  const size_t N = 8;
  int b0[N] = {1, 2, 3, 4, 5, 6, 7, 8};
  char b1[N] = "absdef";
  xout.setitem_n("b0", b0, N);
  xout.setitem_n("b1", b1, N);

  print(1, "std::1d containers", 0, '-');
  xout.setitem("c0", std::vector<int>{11, 22, 33, 44, 55, 66});
  xout.setitem("c1", std::string{"string std string"});
  xout.setitem("c2", std::array<int, N>{1, 2, 3, 4, 5});

  print(1, "xmat Array", 0, '-');
  auto d0 = xmat::NArray<int, 1>{{6}}.enumerate();
  auto d1 = xmat::NArray<int, 2>{{2, 6}}.enumerate();

  d0.enumerate();
  d1.enumerate();
  xout.setitem("d0", d0);
  xout.setitem("d1", d1);

  xout.close();
  printv(xout.head_.total_size);

  // ---------------------------
  print(1, "read input", 0, '-');
  auto xout_ms = xout.buf().get_memsource();
  xmat::BugInMS xin{std::move(xmat::IBBufMS{&xout_ms}.push_all())};
  printv(xin.head_.total_size);
  for(auto& item : xin) { printv(item.name_); }

  print(1, "xin.get", 0, '-');
  print(1, "xin. scalar", 0, '-');
  auto a0_ = xin.at("a0").get<int>();
  printv(a0_);

  print(1, "xin. pointer", 0, '-');
  std::vector<int> b0_(N);
  xin.at("b0").get_to(b0_.data());
  printv(b0_);

  std::string b1_(N, 'x');
  xin.at("b1").get_to(&b1_[0]);
  printv(b1_.c_str());

  print(1, "xin. std", 0, '-');
  auto c0_ = xin.at("c0").get<std::vector<int>>();
  printv(c0_);

  auto c1_ = xin.at("c1").get<std::string>();
  printv(c1_.c_str());

  auto c2_ = xin.at("c2").get<std::array<int, N>>();
  printv(c2_);

  print(1, "xin. xmat::Array", 0, '-');
  auto d0_ = xin.at("d0").get<xmat::NArray<int, 1>>();
  // auto d0_ = xin.at("d0").get<xmat::Array<int, 1, std::allocator<int>>>();
  printv(d0_);

  // auto d1_ = xin.at("d1").get<xmat::array<int, 2>>();
  // // auto d1_ = xin.at("d0").get<xmat::Array<int, 2, std::allocator<int>>>();
  // printv(d1_);

  print(1, "FINISH", 1, '=');
  return 1;
}


int sample_5() {
  print(__PRETTY_FUNCTION__, 1);
  print("xmat::bugin. using memsource", 0, '-');

  print(1, "write output", 0, '-');
  xmat::BugOut xout{};

  print(1, "xmat Array", 0, '-');
  auto d0 = xmat::NArray<int, 1>{{6}};
  auto d1 = xmat::NArray<int, 2>{{2, 6}};
  d0.enumerate();
  d1.enumerate();
  xout.setitem("d0", d0);
  xout.setitem("d1", d1);
  xout.close();

  // ---------------------------
  print(1, "read input", 0, '-');
  auto xout_ms = xout.buf().get_memsource();
  xmat::BugInMS xin{std::move(xmat::IBBufMS{&xout_ms}.push_all())};
  printv(xin.head_.total_size);
  for(auto& item : xin) { printv(item.name_.data()); }

  print(1, "xin. xmat::Array<std::allocator>", 0, '-');
  auto d0_ = xin.at("d0").get<xmat::NArray<int, 1>>();
  printv(d0_);

  auto d1_ = xin.at("d1").get<xmat::NArray<int, 2>>();
  printv(d1_);
  
  print(1, "xin. xmat::Array<xmat::memsource> ", 0, '-');
  auto gmems = &xmat::GlobalMemSource::reset(1 << 10);
  auto d0_ms_ = xin.at("d0").get<xmat::NArrayMS<int, 1>>(gmems);
  printv(d0_ms_);

  print(1, "FINISH", 1, '=');
  return 1;
}
}


int main() {
  print("START: " __FILE__, 0, '=');
  sample_5();
  print("END: " __FILE__, 0, '=');
  return EXIT_SUCCESS;
}
