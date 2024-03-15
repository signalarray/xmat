#pragma once

#include <cstring>
#include <string>


namespace xmat {

struct sview {
  sview(const char* s) : ptr{s}, size_{std::strlen(s)} {}
  sview(const std::string& s) : ptr{s.c_str()}, size_{s.size()} {}
  operator const char*() { return ptr; }

  std::size_t size() { return size_; }
  
  const char* ptr = nullptr;
  std::size_t size_ = 0;
};
} // namespace xmat
