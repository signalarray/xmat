#pragma once

#include <cassert>
#include <cstdint>
#include <fstream>
#include <vector>
#include <array>
#include <algorithm>
#include <exception>
#include <utility>


#include "xutil.hpp"


namespace xmat {

using bbuf_item_t = std::int8_t;

enum class bbuf_stor_tag { dyn, stt, vw, cvw};

struct bbuf_stor_dyn {
  static const bbuf_stor_tag tag = bbuf_stor_tag::dyn;

  bbuf_stor_dyn() = default;
  bbuf_stor_dyn(bbuf_item_t* ptr, std::size_t n) : impl(ptr, ptr + n) {}   // copy `ptr:ptr+n` into itself

  // buffer controll
  std::size_t size() const noexcept { return impl.size(); }
  std::size_t max_size() const noexcept { return impl.max_size(); }

  bool reserve(std::size_t size_request) {
    if (size_request > impl.size()) {
      impl.resize(size_request);
    }
    return true;
  }

  // content acccess
  const bbuf_item_t* data() const noexcept { return impl.data(); }
  bbuf_item_t* data() noexcept { return impl.data(); }

 public:
  std::vector<bbuf_item_t> impl;
};


template<std::size_t N>
struct bbuf_stor_static {
  static const bbuf_stor_tag tag = bbuf_stor_tag::stt;

  bbuf_stor_static() = default;
  bbuf_stor_static(bbuf_item_t* ptr, std::size_t n) { 
    assert(n <= N); std::copy_n(ptr, n, impl.begin());
  }

  // buffer control
  static std::size_t size() noexcept { return N; }
  static std::size_t max_size() noexcept { return N; }
  static bool reserve(std::size_t size_request) noexcept { return size_request <= N; }

  // content access
  bbuf_item_t* data() noexcept { return impl.data(); }
  const bbuf_item_t* data() const noexcept { return impl.data(); }

 public:
  std::array<bbuf_item_t, N> impl;
};


struct bbuf_stor_view {
  static const bbuf_stor_tag tag = bbuf_stor_tag::vw;

  bbuf_stor_view() = default;
  bbuf_stor_view(bbuf_item_t* ptr, std::size_t n) : ptr{ptr}, size_{n} {}

  // buffer control
  std::size_t size() const noexcept { return size_; }
  std::size_t max_size() const noexcept { return size_; }
  bool reserve(std::size_t size_request) noexcept { return size_request <= size_; }

  // content access
  bbuf_item_t* data() noexcept { return ptr; }
  const bbuf_item_t* data() const noexcept { return ptr; }

 public:
  bbuf_item_t* ptr = nullptr;
  std::size_t size_ = 0;
};


struct bbuf_stor_cview {
  static const bbuf_stor_tag tag = bbuf_stor_tag::cvw;

  bbuf_stor_cview() = default;
  bbuf_stor_cview(const bbuf_item_t* ptr, std::size_t n) : ptr{ptr}, size_{n} {}

  // buffer control
  std::size_t size() noexcept { return size_; }
  std::size_t max_size() noexcept { return size_; }
  // [cansel] bool reserve(std::size_t size_request) noexcept; 

  // content access
  // [cansel] bbuf_item_t* data() noexcept;
  const bbuf_item_t* data() const noexcept { return ptr; }

 public:
  const bbuf_item_t* ptr = nullptr;
  std::size_t size_ = 0;
};


template<class StorageT = bbuf_stor_dyn>
class obbuf {
 public:
  using storage_t = StorageT;

  obbuf() {}

  obbuf& write(const char* ptr, std::streamsize n) {
    std::size_t cursor_old = cursor_;
    cursor_ += n;
    size_ = std::max(size_, cursor_);
    if (storage_.reserve(size_)) { 
      std::copy_n(ptr, n, storage_.data() + cursor_old);
    } else {
      state_ = false;
    }
    return *this;
  }

  std::size_t size() const noexcept { return size_; }

 public:
  bbuf_stor_dyn storage_;
  std::size_t size_ = 0;
  std::size_t cursor_ = 0;
  bool state_ = 1;
};


template<class StorageT = bbuf_stor_cview>
class ibbuf {
 public:
  using storage_t = StorageT; 

  ibbuf() {}

  std::size_t size() const noexcept { return size_; }

 public:
  storage_t storage_;
  std::size_t size_;
  std::size_t cursor_ = 0;
};

} // namespace xmat
