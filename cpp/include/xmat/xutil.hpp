#pragma once

#include <cstddef>
#include <cstring>
#include <array>
#include <algorithm>
#include <exception>
#include <memory>
#include <new>

namespace xmat
{

using uint = std::size_t;
using szt = std::size_t;
using ptdt = std::ptrdiff_t;


template<uint N>
uint assign(std::array<char, N>& dest, const char* src) {
  assert(true);
  auto n = std::strlen(src) + 1;
  if (n > N) { throw std::overflow_error("cxx::assing()"); }
  std::copy_n(src, n, dest.begin());
  return n;
}


// ----------------------------
// string view
// ----------------------------
struct sview {
  sview(const char* s) : ptr{s}, size_{std::strlen(s)} {}
  sview(const std::string& s) : ptr{s.c_str()}, size_{s.size()} {}
  operator const char*() { return ptr; }

  std::size_t size() { return size_; }
  
  const char* ptr = nullptr;
  std::size_t size_ = 0;
};



// ----------------------------
// allocator
// ----------------------------
size_t align_up(size_t n, size_t alignment) noexcept {
  return (n + (alignment - 1)) & ~(alignment - 1);
}

inline size_t align_up_(size_t n, size_t alignment) noexcept {
  auto d = n % alignment;
  return n += d == 0 ? 0 : (alignment - d);
}


// Example:
// --------
// xmat::memsource<int> msint{128};
// int* ptr0 = msing.allocate<16>(4);   // with alignment
// int* ptr1 = msing.allocate(4);       // without alignment
template<typename T>
class memsource {
 public:
  using value_type = T;

  memsource() = default;
  memsource(size_t n) { reset(n); }
  memsource(const memsource&) = delete;
  memsource& operator=(const memsource&) = delete;

  T* allocate(size_t n) {
    if (n > space_) { return nullptr; }
    T* out = p_;
    space_ -= n;
    p_ += n;
    return out;
  }

  template<size_t Aln>
  T* allocate_aln(size_t n) {
    static_assert((Aln & (Aln - 1)) == 0, "alignment must be a pow of 2");

    auto space_tmp = space_ * sizeof(T);
    void* p_tmp = reinterpret_cast<void*>(p_);
    void* out_v = std::align(Aln*sizeof(T), n*sizeof(T), p_tmp, space_tmp);
    if (!out_v) {
      throw std::bad_alloc();
      return nullptr;
    }
    T* out = reinterpret_cast<T*>(out_v);
    assert(space_tmp % sizeof(T) == 0);
    space_ = space_tmp / sizeof(T) - n;
    p_ = out + n;
    return out;
  }

  void deallocate(T* p, size_t n) noexcept { }

  void reset() noexcept {
    p_ = buf_.get();
    space_ = N_;
  }

  void reset(size_t n) {
    buf_.reset(new T[n]);
    N_ = n;
    reset();
  }

  // state
  T* data() noexcept { return buf_.get(); }
  size_t size() const noexcept { return N_; }
  size_t space() const noexcept { return space_; }
  size_t used() const noexcept { return N_ - space_; }

  bool pointer_in_buffer(T* p) const noexcept {
    return (p >= data()) && (p <= data() + N_); 
  }

 public:
  std::unique_ptr<T[]> buf_;
  size_t N_ = 0;
  size_t space_ = 0;
  T* p_ = nullptr;
};


// Example:
// --------
// xmat::memsource<void> msint{128};
// int* ptr0 = msing.allocate<int, 16>(4);   // with alignment
// float* ptr1 = msing.allocate<float>(8);   // without alignment
template <>
class memsource<void> {

};

} // namespace xmat
