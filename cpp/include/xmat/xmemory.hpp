#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <array>
#include <algorithm>
#include <exception>
#include <memory>
#include <type_traits>
#include <new>

#include "xutil.hpp"

namespace xmat {


// allocates none-type bytes
template<typename Derived>
struct MemorySourceBase {
 protected:
  MemorySourceBase() = default;

 public:
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

  template<size_t Aln>
  void* allocate_aln(size_t n) {
    void* out = allocate_aln<Aln>(n, std::nothrow);
    if (!out) throw std::bad_alloc();
    return out;
  }

  void* reserve(size_t nmin, size_t nmax, size_t factor, size_t* nout) {
    void* out = reserve(nmin, nmax, factor, nout, std::nothrow);
    if (!out) throw std::bad_alloc();
    return out;
  }

  void* extend(void* ptr, size_t n) {
    void* out = extend(ptr, n, std::nothrow);
    if (!out) throw std::bad_alloc();
    return out;
  }

  void* extend_reserve(void* ptr, size_t nmin, size_t nmax, size_t factor, size_t* nout) {
    void* out = extend_reserve(ptr, nmin, nmax, factor, nout, std::nothrow);
    if (!out) throw std::bad_alloc();
    return out;
  }

  // nothrow methods
  // ---------------
  void deallocate(void* ptr, size_t n) noexcept { }

  void* allocate(size_t n, std::nothrow_t) noexcept {
    if (n > space()) { return nullptr; }
    return update_(p(), n);
  }
  
  void* allocate(size_t n, size_t aln, std::nothrow_t) noexcept {
    assert((aln & (aln - 1)) == 0 && "Aln must be a pow of 2");
    void* pvoid = static_cast<void*>(p());
    if(std::align(aln, n, pvoid, space())) {
      return update_(static_cast<char*>(pvoid), n);
    }
    return nullptr;
  }

  template<size_t Aln>
  void* allocate_aln(size_t n, std::nothrow_t) noexcept {
    static_assert((Aln & (Aln - 1)) == 0, "Aln must be a pow of 2");
    void* pvoid = static_cast<void*>(p());
    if(std::align(Aln, n, pvoid, space())) {
      return update_(static_cast<char*>(pvoid), n);
    }
    return nullptr;
  }

  // extended methods
  // ----------------
  // Returns:
  //  *nout >= nmin, *nout <= nmax, *nout % factor := 0
  void* reserve(size_t nmin, size_t nmax, size_t factor, 
                size_t* nout, std::nothrow_t) noexcept {
    if (space() < nmin) {
      *nout = 0;
      return nullptr;
    }
    *nout = std::min(nmax, space());
    *nout = (*nout / factor) * factor;
    return allocate(*nout, std::nothrow);
  }

  // like reallocate, don't not copy.
  void* extend(void* ptr, size_t n, std::nothrow_t) noexcept {
    assert(n != 0 && "request for n is `0`. it can be an error");
    if (static_cast<char*>(ptr) != p_prev()) return nullptr;
    assert(p() >= p_prev() && "anyway");
    
    // ok check if space is enough
    const size_t np = p() - p_prev();
    if (n - np > space()) {
      return nullptr;
    }
    p() = p_prev() + n;
    space() = (space() + np) - n;
    assert(ptr == p_prev());
    return p_prev();
  }

  // Returns:
  //  *nout >= nmin, *nout <= nmax, *nout % factor := 0
  void* extend_reserve(void* ptr, size_t nmin, size_t nmax, size_t factor, 
                       size_t* nout, std::nothrow_t) noexcept {
    const size_t np = p() - p_prev();
    if (static_cast<char*>(ptr) != p_prev() || nmin > np + space()) {
      nout = 0;
      return nullptr;
    }
    *nout = std::min(nmax, np + space());
    *nout = (*nout / factor) * factor;
    return extend(ptr, *nout, std::nothrow);
  }

 public:
  void reset() noexcept { 
    p_prev() = p() = buf();
    space() = N();
  }

  void reset(char* buf_p, size_t n_p) noexcept {
    p_prev() = p() = buf() = buf_p;
    space() = N() = n_p;
  }

 protected:
  void* update_(char* p_out, size_t n) noexcept {
    p_prev() = p_out; 
    p() = p_out + n; 
    space() -= n;
    return static_cast<void*>(p_prev());
  }

 public:
  // access methods
  // --------------
  size_t used() const noexcept { return N() - space(); }
  size_t size() const noexcept { return {N()}; }

  // interface methods
  char*& buf() noexcept { return static_cast<Derived*>(this)->buf(); }

  size_t& N() noexcept { return static_cast<Derived*>(this)->N(); }
  size_t N() const noexcept { return static_cast<const Derived*>(this)->N(); }

  size_t& space() noexcept { return static_cast<Derived*>(this)->space(); }
  size_t space() const noexcept { return static_cast<const Derived*>(this)->space(); }

  char*& p() noexcept { return static_cast<Derived*>(this)->p(); }

  char*& p_prev() noexcept {return static_cast<Derived*>(this)->p_prev(); }
};


// typed allocation
template<typename T, size_t Aln, typename MSBDerived, 
  typename std::enable_if_t<
    std::is_base_of<MemorySourceBase<MSBDerived>, MSBDerived>::value, int> = 0>
struct TypedMemorySourceBase : protected MSBDerived {
  using base_t = MSBDerived;
  using base_t::base_t;

  static constexpr size_t alignment = Aln;
  static constexpr size_t sizeoft = sizeof(T);

  T* allocate(size_t n) {
    return static_cast<T*>(base_t::template allocate_aln<Aln>(sizeoft));
  }

  T* reserve(size_t nmin, size_t nmax, size_t* nout) {
    return static_cast<T*>(base_t::reserve(nmin*sizeoft, nmax*sizeoft, sizeoft, nout));
  }

  T* extend(T* ptr, size_t n) {
    return static_cast<T*>(base_t::extend(ptr, sizeoft*n));
  }

  T* extend_reserve(T* ptr, size_t nmin, size_t nmax, size_t* nout) {
    return static_cast<T*>(base_t::extend_reserve(ptr, nmin*sizeoft, nmax*sizeoft, sizeoft, nout));
  }

  // nothrow-methods
  // ---------------
  void deallocate(T* ptr, size_t n) noexcept {
    base_t::deallocate(ptr, sizeoft*n);
  }

  T* allocate(size_t n, std::nothrow_t) noexcept {
    return static_cast<T*>(base_t::template allocate_aln<Aln>(sizeoft, std::nothrow));
  }

  T* reserve(size_t nmin, size_t nmax, size_t* nout, std::nothrow_t) noexcept {
    T* ptr = static_cast<T*>(base_t::reserve(nmin*sizeoft, nmax*sizeoft, sizeoft, nout, std::nothrow));
    *nout /= sizeoft;
    return ptr;
  }

  T* extend(T* ptr, size_t n, std::nothrow_t) noexcept {
    return static_cast<T*>(base_t::extend(ptr, sizeoft*n, std::nothrow));
  }

  T* extend_reserve(T* ptr, size_t nmin, size_t nmax, size_t* nout, std::nothrow_t) noexcept {
    T* optr = static_cast<T*>(base_t::extend_reserve(ptr, nmin*sizeoft, nmax*sizeoft, sizeoft, nout, std::nothrow));
    *nout /= sizeoft;
    return optr;
  }

  base_t* base() noexcept { return this; }

  size_t  used() const noexcept { return ((base_t::N() - base_t::space()) / sizeoft) * sizeoft; }

  size_t N() const noexcept { return base_t::N() / sizeoft; }

  size_t space() const noexcept { return base_t::space() / sizeoft; }
};


struct MemorySource : public MemorySourceBase<MemorySource> {
 public:
  using value_type = void;

  MemorySource() = default;

  MemorySource(char* buf, size_t n) { reset(buf, n); }

  MemorySource(MemorySource&&) = default;

  MemorySource& operator=(MemorySource&&) = default;

  // access
  // ------
  char*&  buf() noexcept { return buf_; }
  size_t& N() noexcept { return N_; }
  size_t N() const noexcept { return N_; }
  size_t& space() noexcept { return space_; }
  size_t space() const noexcept { return space_; }
  char*&  p() noexcept { return p_; }
  char*&  p_prev() noexcept {return p_prev_; }

 public:
  char* buf_;
  size_t N_ = 0;
  size_t space_ = 0;
  char* p_ = nullptr;   // pointer for next section
  char* p_prev_ = nullptr;  // pointer for last section
};


struct MemorySourceRef : public MemorySourceBase<MemorySourceRef> {
  
  MemorySourceRef() = default;

  MemorySourceRef(MemorySource* memsource) : memsource_{memsource} {}

  // access
  // ------
  char*&  buf() noexcept { assert(memsource_); return memsource_->buf(); }
  size_t& N() noexcept { assert(memsource_); return memsource_->N(); }
  size_t N() const noexcept { assert(memsource_); return memsource_->N(); }
  size_t& space() noexcept { assert(memsource_); return memsource_->space(); }
  size_t space() const noexcept { assert(memsource_); return memsource_->space(); }
  char*&  p() noexcept { assert(memsource_); return memsource_->p(); }
  char*&  p_prev() noexcept { assert(memsource_); return memsource_->p_prev(); }

  MemorySource* memsource_ = nullptr;
};


// global no-thread-save memory source
// -----------------------------------
struct MemorySourceGlobal : public MemorySourceBase<MemorySourceGlobal> {
  using memsource_t = MemorySource;

  MemorySourceGlobal() = default;

  // access
  // ------
  static char*&  buf() noexcept { return get().buf(); }
  static size_t& N() noexcept { return get().N(); }
  static size_t& space() noexcept { return get().space(); }
  static char*&  p() noexcept { return get().p(); }
  static char*&  p_prev() noexcept {  return get().p_prev(); }

  static memsource_t& get() {
    static memsource_t s;
    return s;
  }

  static MemorySource& reset(size_t n) {
    static std::unique_ptr<char[]> uptr;
    uptr.reset(new char[n]);
    get().reset(uptr.get(), n);
    return get();
  }
};


// allocators based on: MemorySourceRef, MemorySourceGlobal
// --------------------------------------------------------
template<typename T, size_t Aln = alignof(T)>
using AllocatorMS = TypedMemorySourceBase<T, Aln, MemorySource>;

template<typename T, size_t Aln = alignof(T)>
using AllocatorMSRef = TypedMemorySourceBase<T, Aln, MemorySourceRef>;

template<typename T, size_t Aln = alignof(T)>
using AllocatorMSGlobal = TypedMemorySourceBase<T, Aln, MemorySourceGlobal>;


// So, there are classes for memory:
// -------------------------------------------
// memory source            allocator based on
// -------------------------------------------
// MemorySource        ->   AllocatorMS<T, Aln>   (**not used)
// MemorySourceRef     ->   AllocatorMSRef<T, Aln>
// MemorySourceGlobal  ->   AllocatorMSGlobal<T, Aln>
//        the same as  ->   std::allocator<T>

} // namespace xmat
