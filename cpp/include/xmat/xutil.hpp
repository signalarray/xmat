#pragma once

#include <cstddef>
#include <cstring>
#include <array>
#include <algorithm>
#include <exception>
#include <memory>
#include <new>


namespace xmat {

// using uint = std::size_t;
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
struct Strv {
  Strv(const char* s) : ptr{s}, size_{std::strlen(s)} {}
  Strv(const std::string& s) : ptr{s.c_str()}, size_{s.size()} {}
  operator const char*() { return ptr; }

  size_t size() { return size_; }
  
  const char* ptr = nullptr;
  size_t size_ = 0;
};


inline size_t next_pow2(size_t n) noexcept {
  size_t pow = 1, k = 0;
  while (pow < n) {
    pow *= 2;
    ++k;
  }
  return k;
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

// Example:
// --------
// xmat::memsource<void> msint{128};
// int* ptr0 = msing.allocate<int, 16>(4);   // with alignment
// float* ptr1 = msing.allocate<float>(8);   // without alignment
class MemSource {
 public:
  using value_type = void;

  MemSource() = default;
  MemSource(char* buf, size_t n) { init(buf, n); }
  // memsource(const memsource&) = default;
  // memsource& operator=(const memsource&) = default;
  MemSource(MemSource&&) = default;
  MemSource& operator=(MemSource&&) = default;

  // reserve
  void* reserve(size_t nmin, size_t nmax, size_t* nout, std::nothrow_t) noexcept {
    if (space_ < nmin) {
      *nout = 0;
      return nullptr;
    }
    *nout = std::min(nmax, space_);
    return allocate(*nout, std::nothrow);
  }


  // allocate
  void* allocate(size_t n, std::nothrow_t) noexcept {
    if (n > space_) { return nullptr; }
    return update_(p_, n);
  }


  void* allocate(size_t n, size_t aln, std::nothrow_t) noexcept {
    assert((aln & (aln - 1)) == 0 && "Aln must be a pow of 2");
    void* pvoid = static_cast<void*>(p_);
    if(std::align(aln, n, pvoid, space_)) {
      return update_(static_cast<char*>(pvoid), n);
    }
    return nullptr;
  }


  template<typename T>
  T* allocate(size_t n, std::nothrow_t) noexcept {
    return static_cast<T*>(allocate_aln<alignof(T)>(n * sizeof(T)));
  }


  template<typename T, size_t Aln>
  T* allocate(size_t n, std::nothrow_t) noexcept {
    // as many checks as possible: because not clear when will it be used.
    // for ariерhetic types for calculations it should be like bellow.
    static_assert(Aln >= alignof(T), "Aln less than alignof(T)");
    static_assert(Aln % alignof(T) == 0, "Aln isn't multiple to alignof(T)");
    static_assert(Aln % sizeof(T) == 0, "Aln isn't multiple to sizeof(T)");
    return static_cast<T*>(allocate_aln<Aln>(n * sizeof(T)));
  }


  template<size_t Aln>
  void* allocate_aln(size_t n, std::nothrow_t) noexcept {
    static_assert((Aln & (Aln - 1)) == 0, "Aln must be a pow of 2");
    void* pvoid = static_cast<void*>(p_);
    if(std::align(Aln, n, pvoid, space_)) {
      return update_(static_cast<char*>(pvoid), n);
    }
    return nullptr;
  }


  // like reallocate, but not copy.
  void* extend(void* ptr, size_t n, std::nothrow_t) noexcept {
    assert(n != 0 && "request for n is `0`. it can be an error");
    if (static_cast<char*>(ptr) != p_prev_) return nullptr;
    assert(p_ >= p_prev_ && "anyway");
    
    // ok check if space is enough
    const size_t np = p_ - p_prev_;
    if (n - np > space_) {
      return nullptr;
    }
    p_ = p_prev_ + n;
    space_ = (space_ + np) - n;
    assert(ptr == p_prev_);
    return p_prev_;
  }


  void* extend_reserve(void* ptr, size_t nmin, size_t nmax, size_t* nout, std::nothrow_t) noexcept {
    const size_t np = p_ - p_prev_;
    if (static_cast<char*>(ptr) != p_prev_ || nmin > np + space_) {
      nout = 0;
      return nullptr;
    }
    *nout = std::min(nmax, np + space_);
    return extend(ptr, *nout, std::nothrow);
  }

  // not noexcept functions ----------------------------
  void* reserve(size_t nmin, size_t nmax, size_t* nout) {
    void* out = reserve(nmin, nmax, nout, std::nothrow);
    if (!out) throw std::bad_alloc();
    return out;
  }

  void* allocate(size_t n) {
    void* out = allocate(n, std::nothrow);
    if (!out) throw std::bad_alloc();
    return out;
  }

  void* allocate(size_t n, size_t aln) {
    void* out = allocate(n, aln, std::nothrow);
    if (!out) throw std::bad_alloc();
    return out;
  } 

  template<typename T>
  T* allocate(size_t n) {
    T* out = allocate<T>(n, std::nothrow);
    if (!out) throw std::bad_alloc();
    return out;
  }

  template<typename T, size_t Aln>
  T* allocate(size_t n) {
    T* out = allocate<T, Aln>(n, std::nothrow);
    if (!out) throw std::bad_alloc();
    return out;
  }

  template<size_t Aln>
  void* allocate_aln(size_t n) {
    void* out = allocate_aln<Aln>(n, std::nothrow);
    if (!out) throw std::bad_alloc();
    return out;
  }

  void* extend(void* ptr, size_t n) {
    void* out = extend(ptr, n, std::nothrow);
    if (!out) throw std::bad_alloc();
    return out;
  }

  void* extend_reserve(void* ptr, size_t nmin, size_t nmax, size_t* nout) {
    void* out = extend_reserve(ptr, nmin, nmax, nout, std::nothrow);
    if (!out) throw std::bad_alloc();
    return out;
  }
  // end not-noexcept functions ----

  // deallocate
  void deallocate(void* ptr, size_t n) noexcept { }

  template<typename T>
  void deallocate(T* ptr, size_t n) noexcept { }

  // storage access
  // --------------
  void init(void* buf, size_t n) {
    buf_ = static_cast<char*>(buf);
    N_ = n;
    reset();
  }

  void reset() noexcept {
    p_prev_ = p_ = buf_;
    space_ = N_;
  }

  // state
  char* data() noexcept { return buf_; }
  const char* data() const noexcept { return buf_; }
  size_t size() const noexcept { return N_; }
  size_t space() const noexcept { return space_; }
  size_t used() const noexcept { return N_ - space_; }

  bool pointer_in_buffer(void* p) const noexcept {
    return (static_cast<char*>(p) >= buf_) && (static_cast<char*>(p) <= buf_ + N_);
  }

 private:
  void* update_(char* p_out, size_t n) {
    p_prev_ = p_out; 
    p_ = p_prev_ + n; 
    space_ -= n;
    return static_cast<void*>(p_prev_);
  }

 public:
  char* buf_;
  size_t N_ = 0;
  size_t space_ = 0;
  char* p_ = nullptr;   // pointer for next section
  char* p_prev_ = nullptr;  // pointer for last section
};

template<typename T, typename Enable = void> struct MemSourceAlloc { };

// for T = 8 bit size type
template<typename T>
struct MemSourceAlloc <T, std::enable_if_t<sizeof(T) == sizeof(char)>> {
  using source_t = MemSource;
  MemSourceAlloc(MemSource& memsrc) : impl{&memsrc} {}
  MemSourceAlloc(MemSource* memsrc) : impl{memsrc} {}
  T* allocate(size_t n) { return impl->template allocate<T>(n); }
  void deallocate(T* p, size_t n) noexcept { impl->deallocate(p, n); }

  // additional
  size_t space() const noexcept { return impl->space(); }

  void* extend_reserve(void* ptr, size_t nmin, size_t nmax, size_t* nout, std::nothrow_t nth) noexcept {
    return impl->extend_reserve(ptr, nmin, nmax, nout, nth);
  }

  void* reserve(size_t nmin, size_t nmax, size_t* nout, std::nothrow_t nth) noexcept {
    return impl->reserve(nmin, nmax, nout, nth);
  }

  mutable MemSource* impl = nullptr;
};

// for T = 8+ bit size type
template<typename T>
struct MemSourceAlloc <T, std::enable_if_t<(sizeof(T) > sizeof(char))>> {
  using source_t = MemSource;
  MemSourceAlloc(MemSource& memsrc) : impl{&memsrc} {}
  MemSourceAlloc(MemSource* memsrc) : impl{memsrc} {}
  T* allocate(size_t n) { return impl->template allocate<T>(n); }
  void deallocate(T* p, size_t n) noexcept { impl->deallocate(p, n); }

  mutable MemSource* impl = nullptr;
};

// for global allocator
// --------------------
struct GlobalMemSource {
  using memsource_t = MemSource;
  static MemSource& get() {
    static MemSource s;
    return s;
  }

  static MemSource& reset(size_t n) {
    static std::unique_ptr<char[]> uptr;
    uptr.reset(new char[n]);
    get().init(uptr.get(), n);
    return get();
  }
};


// empty memsource will no allocate anything. can be used as default initializer.
struct GlobalMemSourceNull {
  using memsource_t = MemSource;
  static MemSource& get() {
    static MemSource s;
    return s;
  }
};


template<typename T, class Arena = GlobalMemSource>
class GlobalMemAllocator {
 public:
  using value_type = T;
  using arena_t = Arena;
  using source_t = typename Arena::memsource_t;

  GlobalMemAllocator() = default;
  
  template<typename U> 
  constexpr GlobalMemAllocator(const GlobalMemAllocator<U>&) noexcept {}
  
  T* allocate(size_t n) { return source().template allocate<T>(n); }

  template<size_t Aln>
  T* allocate(size_t n) { return source().template allocate_aln<T, Aln>(n); }

  void deallocate(T* p, size_t n) noexcept {}

  static source_t& source() { return GlobalMemSource::get(); }  // the same for all instancess
};


template<class T, class U, class A>
bool operator==(const GlobalMemAllocator<T, A>&, const GlobalMemAllocator<U, A>&) { return true; }
 
template<class T, class U, class A>
bool operator!=(const GlobalMemAllocator<T, A>&, const GlobalMemAllocator<U, A>&) { return false; }
} // namespace xmat
