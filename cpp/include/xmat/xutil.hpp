#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <array>
#include <algorithm>
#include <exception>
#include <memory>
#include <new>


namespace xmat {

using szt = std::size_t;
using ptdt = std::ptrdiff_t;


// TODO - change this. looks not good.
template<size_t N>
size_t assign(std::array<char, N>& dest, const char* src) {
  assert(true);
  auto n = std::strlen(src) + 1;
  if (n > N) { throw std::overflow_error("cxx::assing()"); }
  std::copy_n(src, n, dest.begin());
  return n;
}


// ----------------------------
// string view
// ----------------------------
struct VString {
  VString(const char* s) : ptr{s} {}
  VString(const std::string& s) : ptr{s.c_str()} {}
  operator const char*() { return ptr; }

  const char* data() const noexcept { return ptr; }
  bool empty() const noexcept { return !size(); }
  size_t size() const noexcept { return std::strlen(ptr); }
  
  bool operator==(const VString& other) const noexcept { return std::strcmp(ptr, other.ptr); }
  bool operator==(const char* other) const noexcept { return std::strcmp(ptr, other); }
  bool operator==(const std::string& other) const noexcept { return other == ptr; }
  friend bool operator==(const char* lhs, const VString& rhs) noexcept { return rhs == lhs; }
  friend bool operator==(const std::string& lhs, const VString& rhs) noexcept { return rhs == lhs; }

  const char* ptr = nullptr;
};

inline size_t next_pow2(size_t n) noexcept {
  size_t pow = 1, k = 0;
  while (pow < n) {
    pow *= 2;
    ++k;
  }
  return k;
}

// See also: align_up
template<typename Int1, typename Int2>
Int1 next_multiple_to(Int1 n, Int2 devider) noexcept {
  return n + (devider - n % devider);
}

// ----------------------------
// allocator
// ----------------------------
inline size_t align_up(size_t n, size_t alignment) noexcept {
  return (n + (alignment - 1)) & ~(alignment - 1);
}

inline size_t align_up_(size_t n, size_t alignment) noexcept {
  auto d = n % alignment;
  return n += d == 0 ? 0 : (alignment - d);
}

inline bool is_aligned(const void* ptr, uintptr_t alignment) noexcept {
  return !(reinterpret_cast<uintptr_t>(ptr) % alignment);
}

template<size_t Aln>
inline bool is_aligned(const void* ptr) noexcept {
  return !(reinterpret_cast<uintptr_t>(ptr) % Aln);
}

// is little endian
inline bool isle() {
  size_t x = 1;
  return *((char *) &x);
}

// template<typename T, class Arena = GlobalMemSource>
// class GlobalMemAllocator {
//  public:
//   using value_type = T;
//   using arena_t = Arena;
//   using source_t = typename Arena::memsource_t;
// 
//   GlobalMemAllocator() = default;
//   
//   template<typename U> 
//   constexpr GlobalMemAllocator(const GlobalMemAllocator<U>&) noexcept {}
//   
//   T* allocate(size_t n) { return source().template allocate<T>(n); }
// 
//   template<size_t Aln>
//   T* allocate(size_t n) { return source().template allocate_aln<T, Aln>(n); }
// 
//   void deallocate(T* p, size_t n) noexcept {}
// 
//   static source_t& source() { return GlobalMemSource::get(); }  // the same for all instancess
// };
// 
// 
// template<class T, class U, class A>
// bool operator==(const GlobalMemAllocator<T, A>&, const GlobalMemAllocator<U, A>&) { return true; }
//  
// template<class T, class U, class A>
// bool operator!=(const GlobalMemAllocator<T, A>&, const GlobalMemAllocator<U, A>&) { return false; }
} // namespace xmat
