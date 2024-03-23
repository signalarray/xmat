#pragma once

#include <cassert>
#include <cstdint>
#include <fstream>
#include <vector>
#include <array>
#include <algorithm>
#include <exception>
#include <utility>
#include <new>

#include "xutil.hpp"


namespace xmat {

using bbuf_item_t = char;

struct bbuf_storage_dyn {
  bbuf_storage_dyn() = default;

  bool size_request(std::size_t size_request) {
    if (size_request > impl.size()) {
      impl.resize(size_request);
    }
    return true;
  }

  // content acccess
  bbuf_item_t* data() noexcept { return impl.data(); }
  const bbuf_item_t* data() const noexcept { return impl.data(); }
  std::size_t size() const noexcept { return impl.size(); }
  std::size_t max_size() const noexcept { return impl.max_size(); }

 public:
  std::vector<bbuf_item_t> impl;
};


struct bbuf_storage_ms {
  // bbuf_storage_ms() = default;
  bbuf_storage_ms(memsource* msrc) : memsource_{msrc} {}

  bool size_request(size_t n) noexcept { // think about exception instead flag
    // check local mem-menager
    if (n <= N_ ) return true;

    // try to reallocate from memory source
    return reserve(n);
  }

  bool reserve(size_t n) {
    bool out = false;
    size_t nn = 1 << next_pow2(n);

    size_t nout = 0;
    void* ptr = nullptr;
    ptr = memsource_->extend_reserve(data_, n, nn, &nout, std::nothrow);
    if (!ptr) {
      ptr = memsource_->reserve(n, nn, &nout, std::nothrow);
    }
    if (ptr) {
      out = true;
      std::copy_n(data_, N_, static_cast<char*>(ptr));
      data_ = static_cast<char*>(ptr);
      N_ = n;
    }
    else {
      throw std::runtime_error("exceed memory_source space : " __FILE__);
    }
    return out;
  }

  bbuf_item_t* data() noexcept { return data_; }
  const bbuf_item_t* data() const noexcept { return data_; }
  size_t size() const noexcept { return N_; }
  size_t max_size() const noexcept { return memsource_->space(); }

  memsource* memsource_ = nullptr;
  bbuf_item_t* data_ = nullptr;
  size_t N_ = 0;
};


template<class StorageT = bbuf_storage_dyn>
class obbuf {
 public:
  using storage_t = StorageT;

  obbuf() {}
  obbuf(memsource* memsrc) : storage_{memsrc} {}

  obbuf& write(const char* ptr, std::streamsize n) {
    std::size_t cursor_old = cursor_;
    cursor_ += n;
    size_ = std::max(size_, cursor_);
    if (storage_.size_request(size_)) {
      std::copy_n(ptr, n, storage_.data() + cursor_old);
    } else {
      state_ = false;
    }
    return *this;
  }

  std::size_t size() const noexcept { return size_; }

 public:
  storage_t storage_;
  std::size_t size_ = 0;
  std::size_t cursor_ = 0;
  bool state_ = true;
};


class ibbuf {
 public:
  ibbuf() {}

  std::size_t size() const noexcept { return size_; }

 public:
  std::size_t size_;
  std::size_t cursor_ = 0;
};

} // namespace xmat
