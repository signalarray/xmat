#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>
#include <array>
#include <complex>
#include <algorithm>
#include <iterator>
#include <exception>
#include <utility>
#include <new>

#include "xutil.hpp"


namespace xmat {

using xbyte_t = char;
using xsz_t = std::uint64_t;
using xuint8_t = std::uint8_t;

// constants
const xuint8_t k_xsz_t_size = sizeof(xsz_t);
const xuint8_t k_signature_size = 4;

// Head
static constexpr char k_format_signature[k_signature_size] = "XYZ";
const xuint8_t k_max_block_name_len = 32;
const xuint8_t k_max_type_name_len = 8;
const xuint8_t k_max_ndim_xmat = 8;
const xuint8_t k_head_size = {8 + 2*k_signature_size + 4}; // size in bytes

// Block
static constexpr char k_block_signature_begin[k_signature_size] = "<#>";
static constexpr char k_block_signature_end[k_signature_size] = ">#<";
const xuint8_t k_block_size = 
  {k_max_block_name_len + k_max_type_name_len + 
  (k_max_ndim_xmat + 1) * k_xsz_t_size + (1 + 1)*1 + 2 * k_signature_size};


// supported types registration
// ----------------------------
template<typename T>
struct TypeInfo {
  // static_assert(false, "using not registered data_type");
  static const bool registered = false;
  static const xuint8_t size = 0;
  static constexpr const char* const name = "undef";
};

#define XMAT_ADDTYPE(T, Name) \
template<> struct TypeInfo<T> {                            \
  static const bool registered = true;                     \
  static const xuint8_t size = sizeof(T);                  \
  static constexpr const char* const name = Name;          \
};

XMAT_ADDTYPE(char,                 "qchar");  // char
XMAT_ADDTYPE(std::int8_t,          "ri08");   // int signed
XMAT_ADDTYPE(std::int16_t,         "ri16");
XMAT_ADDTYPE(std::int32_t,         "ri32");
XMAT_ADDTYPE(std::int64_t,         "ri64");
XMAT_ADDTYPE(std::uint8_t,         "ru08");   // int unsigned
XMAT_ADDTYPE(std::uint16_t,        "ru16");
XMAT_ADDTYPE(std::uint32_t,        "ru32");
XMAT_ADDTYPE(std::uint64_t,        "ru64");
XMAT_ADDTYPE(float,                "rf32");   // floating point
XMAT_ADDTYPE(double,               "rf64");
XMAT_ADDTYPE(std::complex<float>,  "cf32");   // complex
XMAT_ADDTYPE(std::complex<double>, "cf64");


// byte buffer
// ----------------------------
class XStreamError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};


// for: xmat::memallocator<char>, xmat::glob_memallocator<char>
template<typename MemSourceT>
struct BBufStorage_ {
  using memory_source_t = MemSourceT;

  //buf_storage_(const memory_source_t& memsrc)

  BBufStorage_() { }
  BBufStorage_(const memory_source_t& memsrc) : memsource_{memsrc} {}

  void size_request(size_t n) { if (n > N_ ) reserve(n); }

  size_t request_all() {
    size_request(memsource_.space()); // does nothing
    return size();
  }

  void reserve(size_t n) { // throw error if unsuccess
    bool out = false;
    size_t nn = 1 << next_pow2(n);

    size_t nout = 0;
    void* ptr = nullptr;
    ptr = memsource_.extend_reserve(data_, n, nn, &nout, std::nothrow);
    
    if(!ptr) {
      ptr = memsource_.reserve(n, nn, &nout, std::nothrow);
      if (ptr) {
        assert((data_ && N_) || (!data_ && !N_));
        std::copy_n(data_, N_, static_cast<char*>(ptr));
      }
      else {
        throw XStreamError("buff_storage_ms:reserve(). exceed memory_source space : " __FILE__);
      }
    }
    data_ = static_cast<char*>(ptr);
    N_ = n;
  }

  xbyte_t* data() noexcept { return data_; }
  const xbyte_t* data() const noexcept { return data_; }
  size_t size() const noexcept { return N_; }
  size_t max_size() const noexcept { return memsource_.space(); }
  bool ready() const noexcept { return true; }

  memory_source_t memsource_;
  xbyte_t* data_ = nullptr;
  size_t N_ = 0;
};


using bbuf_memsource_default = std::allocator<xbyte_t>;

template<>
struct BBufStorage_<bbuf_memsource_default> {

  BBufStorage_() = default;

  void size_request(size_t size_request) {
    if (size_request > impl.size()) {
      impl.resize(size_request);
    }
  }

  size_t request_all() {
    size_request(impl.size()); // does nothing
    return impl.size();
  }

  // content acccess
  xbyte_t* data() noexcept { return impl.data(); }
  const xbyte_t* data() const noexcept { return impl.data(); }
  size_t size() const noexcept { return impl.size(); }
  size_t max_size() const noexcept { return impl.max_size(); }
  bool ready() const noexcept { return true; }

 public:
  std::vector<xbyte_t> impl;
};


// util
static size_t util_seek(std::streamoff off, std::ios_base::seekdir way, size_t cursor, size_t size) {
  if (way == std::ios_base::beg) {
    assert(off >= 0 && (size_t)off <= size && "wrong `off` value");
    cursor = off;
  }
  else if (way == std::ios_base::cur) {
    size_t cursor_new = (std::streamoff)cursor + off;
    assert(cursor_new <= size);
    cursor = cursor_new;
  }
  else if (way == std::ios_base::end) {
    assert(off >= 0 && (size_t)off <= size && "wrong `off` value");
    cursor = size - off;
  }
  else { 
    assert(false); 
  }
  assert(cursor <= size);
  return cursor;
}


template<typename MemSourceT>
class OBBuf_ {
 public:
  using memory_source_t = MemSourceT;
  using storage_t = BBufStorage_<MemSourceT>;

  OBBuf_() {}
  OBBuf_(const memory_source_t& memsrc) : storage_{memsrc} {}

  OBBuf_(const OBBuf_&) = delete;
  OBBuf_& operator=(const OBBuf_&) = delete;
  OBBuf_(OBBuf_&&) = default;
  OBBuf_& operator=(OBBuf_&&) = default;

  // like std::ostream
  OBBuf_& write(const char* ptr, std::streamsize n) {
    size_t size_new = std::max(size_, cursor_ + n);
    storage_.size_request(size_new); // throw exception
    // ---- no exeption line
    std::copy_n(ptr, n, storage_.data() + cursor_);
    size_ = size_new;
    cursor_ += n;
    return *this;
  }

  std::streampos tellp() const noexcept { return cursor_; }

  OBBuf_& seekp(std::streampos pos) { /*not shure  about exception*/
    assert(pos <= size_);
    cursor_ = pos; 
    return *this; 
  }

  OBBuf_& seekp(std::streamoff off, std::ios_base::seekdir way) {  /*not shure  about exception*/
    cursor_ = util_seek(off, way, cursor_, size_);
    if(cursor_ > size_) throw XStreamError("buff::seek(). index exceed: " __FILE__);
    return *this;
  }

  bool is_open() const noexcept { return is_open_ && storage_.ready(); }

  void close() noexcept { is_open_ = false; }

  // access
  MemSource get_memsource() noexcept { return MemSource{storage_.data(), storage_.size()}; }

  std::size_t size() const noexcept { return size_; }
  storage_t& storage() { return storage_; }

 public:
  storage_t storage_;
  std::size_t size_ = 0;
  std::size_t cursor_ = 0;
  bool is_open_ = true;
};


template<typename MemSourceT>
class IBBuf_ {
 public:
  using memory_source_t = MemSourceT;
  using storage_t = BBufStorage_<MemSourceT>;

  IBBuf_() {}
  IBBuf_(const memory_source_t& memsrc) : storage_{memsrc} {}
  IBBuf_(const IBBuf_&) = delete;
  IBBuf_& operator=(const IBBuf_&) = delete;
  IBBuf_(IBBuf_&&) = default;
  IBBuf_& operator=(IBBuf_&&) = default;

  IBBuf_& push(const char* ptr, std::streamsize n) {
    size_t size_new = size_ + n;
    storage_.size_request(size_new); // throw exception
    // ---- no exeption line
    std::copy_n(ptr, n, storage_.data() + cursor_);
    size_ = size_new;
    return *this;
  }

  // set storage content as a buffer content
  IBBuf_& push_all() {
    size_ = storage_.request_all();
    return *this;
  }

  // like std:istream
  IBBuf_& read(char* ptr, std::streamsize n) {
    assert(cursor_ + n <= size_);
    std::copy_n(storage_.data() + cursor_, n, ptr);
    cursor_ += n;
    return *this;
  }

  std::streampos tellg() const noexcept { return cursor_; }

  IBBuf_& seekg(std::streampos pos) {
    assert(pos <= size_);
    cursor_ = pos; 
    return *this; 
  }

  IBBuf_& seekg(std::streamoff off, std::ios_base::seekdir way) {
    cursor_ = util_seek(off, way, cursor_, size_);
    if(cursor_ > size_) throw XStreamError("buff::seek(). index exceed: " __FILE__);
    return *this;
  }

  bool is_open() const noexcept { return is_open_ && storage_.ready(); }

  void close() noexcept { is_open_ = false; }

  // access
  size_t size() const noexcept { return size_; }
  storage_t& storage() { return storage_; }

 public:
  storage_t storage_;
  std::size_t size_ = 0;
  std::size_t cursor_ = 0;
  bool is_open_ = true;
};


// format implementation
// ---------------------
template<typename OBuff, typename T>
OBuff& util_write(OBuff& buff, const T& x) {
  return buff.write(reinterpret_cast<const char*>(&x), sizeof(x));
}

template<typename OBuff, typename T>
OBuff& util_write(OBuff& buff, const T* ptr, size_t n) {
  return buff.write(reinterpret_cast<const char*>(ptr), n*sizeof(T));
}

template<typename IBuff, typename T>
IBuff& util_read(IBuff& buff, T& x) {
  buff.read(reinterpret_cast<char*>(&x), sizeof(T));
  return buff;
}

template<typename IBuff, typename T>
IBuff& util_read(IBuff& buff, T* ptr, size_t n) {
  return buff.read(reinterpret_cast<char*>(ptr), n*sizeof(T));
}

struct StreamHead {
 public:
  template<class OBuff>
  OBuff& dump(OBuff& obuf) const {
    util_write(obuf, xsz_t{0});
    util_write(obuf, format_signature_, k_signature_size);
    util_write(obuf, xint_size);
    util_write(obuf, max_block_name_len);
    util_write(obuf, max_type_name_len);
    util_write(obuf, max_ndim);
    util_write(obuf, format_signature_, k_signature_size);
    return obuf;
  }

  template<class IBuff>
  IBuff& load(IBuff& ibuf) {
    util_read(ibuf, total_size);
    util_read(ibuf, format_signature_, k_signature_size);
    util_read(ibuf, xint_size);
    util_read(ibuf, max_block_name_len);
    util_read(ibuf, max_type_name_len);
    util_read(ibuf, max_ndim);
  
    char format_signature_end[k_signature_size] = {};
    util_read(ibuf, format_signature_end, k_signature_size);
    return ibuf;
  }

 public:
  char format_signature_[k_signature_size] = "XYZ";
  xuint8_t xint_size = k_xsz_t_size;
  xuint8_t max_block_name_len = k_max_block_name_len;
  xuint8_t max_type_name_len = k_max_type_name_len;
  xuint8_t max_ndim = k_max_ndim_xmat;

  xsz_t total_size = 0;
};


// streams
// --------------------
template<typename T, typename Enable = void> struct Serializer { 
  // static const bool enabled = /*not enabled*/;
};


class StreamBlock {
 public:
  using shape_t = std::array<xsz_t, k_max_ndim_xmat>;

  StreamBlock() = default;

  template<typename OBuff>
  OBuff& dump(OBuff& obuf) const {
    util_write(obuf, k_block_signature_begin, k_signature_size);
    util_write(obuf, name_.data(), name_.size());
    util_write(obuf, typename_.data(), typename_.size());
    util_write(obuf, shape_.data(), shape_.size());
    util_write(obuf, numel_);
    util_write(obuf, typesize_);
    util_write(obuf, ndim_);
    util_write(obuf, k_block_signature_end, k_signature_size);
    return obuf;
  }

  template<typename IBuff>
  IBuff& load(IBuff& ibuf) {
    char sig_0[k_signature_size] = {};
    char sig_1[k_signature_size] = {};

    pos_ = ibuf.tellg();
    util_read(ibuf, sig_0, k_signature_size);
    assert(std::strcmp(sig_0, k_block_signature_begin) == 0 && "wrong block signature in buffer");

    util_read(ibuf, name_.data(), name_.size());
    util_read(ibuf, typename_.data(), typename_.size());
    util_read(ibuf, shape_.data(), shape_.size());
    util_read(ibuf, numel_);
    util_read(ibuf, typesize_);
    util_read(ibuf, ndim_);
    util_read(ibuf, sig_1, k_signature_size);

    assert(std::strcmp(sig_1, k_block_signature_end) == 0 && "wrong block signature in buffer");
    return ibuf;
  }

  // --------------------
  bool empty() const noexcept { return pos_ == 0; }
  operator bool()const noexcept { return !empty(); }

  template<typename T>
  bool check_element() const {
    bool out = 
      std::strcmp(TypeInfo<T>::name, typename_.cbegin()) == 0 && 
      TypeInfo<T>::size == typesize_;
    return out;
  }

 public:
  std::array<char, k_max_block_name_len> name_ = {};
  std::array<char, k_max_type_name_len> typename_ = {};
  shape_t shape_ = {0};
  xsz_t numel_ = 0;
  xuint8_t typesize_ = 0;
  xuint8_t ndim_ = 0;

  const char* ptr_ = nullptr;
  xuint8_t pos_ = 0;
};


template<typename OBuff>
struct BugOut_ {
  using buff_t = OBuff;
  
  BugOut_() {
    // for bbuf_storage std 
    if(obuf_.is_open()) head_.dump(obuf_);
  }

  BugOut_(OBuff&& buf) : obuf_{std::move(buf)} {
    head_.dump(obuf_);
  }

  ~BugOut_() { close(); }

  template<typename T, bool Enabled = Serializer<T>::enabled>
  void setitem(Strv name, const T& x) {
    StreamBlock block = Serializer<T>::dump(x);
    write_block(name, block);
  }

  template<typename T>
  void setitem_n(Strv name, const T* x, size_t n) {
    StreamBlock block = Serializer<T*>::dump_n(x, n);
    write_block(name, block);
  }

  StreamBlock write_block(Strv name, StreamBlock& block) {
    if(!obuf_.is_open()) {
      throw XStreamError("xmat::bugout.write_block. attempt to write to a closed stream");
    }
    assign(block.name_, name.ptr);
    block.dump(obuf_);
    obuf_.write(block.ptr_, block.typesize_*block.numel_);
    return block;
  }

  void close() noexcept {
    if (!obuf_.is_open()) { return; }
    // save total size
    obuf_.seekp(0, std::ios::end);
    head_.total_size = obuf_.tellp();
    obuf_.seekp(0);
    util_write(obuf_, static_cast<xsz_t>(head_.total_size));
    obuf_.close(); 
  }

  // access
  StreamHead& head() { return head_; }
  buff_t& buf() { return obuf_; }

 public:
  StreamHead head_;
  buff_t obuf_;
};


// ----------------------------
template<typename IBuff>
struct BugIn_ {
  using buff_t = IBuff;
  
  BugIn_() = default;

  BugIn_(IBuff&& buf) : ibuf_{std::move(buf)} { 
    if (ibuf_.size() >= k_head_size){
      scan_head(); 
    } 
  }

  ~BugIn_() { close(); }

  struct Iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = StreamBlock;
    using pointer           = StreamBlock*;
    using reference         = StreamBlock&;

    Iterator(size_t endpos) : endpos{endpos} {}

    Iterator(IBuff* ibuf, size_t endpos) : ibuf{ibuf}, endpos{endpos} {
      ibuf->seekg(k_head_size);
      ++(*this);
    }

    const StreamBlock& operator*() const { return block; }
    const StreamBlock* operator->() const { return &block; }

    Iterator& operator++() {
      if (ibuf->tellg() >= endpos) { 
        block = StreamBlock{};
      } 
      else {
        block.load(*ibuf);
        ibuf->seekg(block.typesize_ * block.numel_, std::ios::cur);
      }
      return *this;
    }

    Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

    friend bool operator== (const Iterator& a, const Iterator& b) { return a.block.pos_ == b.block.pos_; }
    friend bool operator!= (const Iterator& a, const Iterator& b) { return a.block.pos_ != b.block.pos_; }

    // data conversion
    template<typename T> T get() {
      assert(!empty());
      if(empty()) throw XStreamError{"bugin.iterator.get<T>: access to empty block"};
      return Serializer<T>::load(block, *ibuf);
    }

    template<typename T, typename Allocator> T get(Allocator&& alloc) {
      assert(!empty());
      if(empty()) throw XStreamError{"bugin.iterator.get<T>: access to empty block"};
      return Serializer<T>::load_with_allocator(block, *ibuf, std::forward<Allocator>(alloc));
    }

    template<typename T> T& get_to(T& y) {
      assert(!empty());
      if(empty()) throw XStreamError{"bugin.iterator.get<T>: access to empty block"};
      return Serializer<T>::load_to(block, *ibuf, y);
    }

    template<typename T> T* get_to(T* y) {
      assert(!empty());
      if(empty()) throw XStreamError{"bugin.iterator.get<T>: access to empty block"};
      return Serializer<T*>::load_to(block, *ibuf, y);
    }

    // access
    bool empty() const noexcept { return block.empty(); }
    operator bool()const noexcept { return !empty(); }

    const char* name()  const noexcept { return block.name_.data(); }
    const char* type_name() const noexcept { return block.typename_.data(); }
    StreamBlock::shape_t shape() const noexcept { return block.shape_; }
    xsz_t shape(size_t nd) const noexcept { assert(nd < block.ndim_); return block.shape_[nd]; }
    xsz_t numel() const noexcept { return block.numel_; }
    xuint8_t type_size() const noexcept { return block.typesize_; }
    xuint8_t ndim() const noexcept { return block.ndim_; }
    const char* ptr() const noexcept { return block.ptr_; }
    size_t  pos() const noexcept { return block.pos_; }

   public:
    IBuff* ibuf = nullptr;
    StreamBlock block;
    size_t endpos = 0;
  };

  Iterator begin() { return Iterator{&ibuf_, head_.total_size}; }

  Iterator end() { return Iterator{head_.total_size}; }

  Iterator at(Strv name) {
    auto it = std::find_if(
      begin(),
      end(),
      [name](const StreamBlock& block){
        return std::strcmp(name.ptr, block.name_.cbegin()) == 0;
      }
    );
    return it;
  }

  // parse content
  // -------------
  void close() noexcept { ibuf_.close(); }

  void scan_head() {
    assert(head_.total_size == 0);
    if(ibuf_.size() < k_head_size) {
      throw XStreamError("xmat::bugin.scan_head(). stream buffer isn't filled");
    }
    head_.load(ibuf_);
  }

  // access
  StreamHead& head() { return head_; }
  buff_t& buf() { return ibuf_; }

 public:
  StreamHead head_;
  buff_t ibuf_;
};


// aliases
// -----------------------------------
using OBBuf       = OBBuf_<bbuf_memsource_default>;      // default_constructable
using OBBufGMS    = OBBuf_<GlobalMemAllocator<xbyte_t>>;  // default_constructable
using OBBufMS     = OBBuf_<MemSourceAlloc<xbyte_t>>;       // non_default_constructable

using IBBuf       = IBBuf_<bbuf_memsource_default>;      // default_constructable
using IBBufGMS    = IBBuf_<GlobalMemAllocator<xbyte_t>>;  // default_constructable
using IBBufMS     = IBBuf_<MemSourceAlloc<xbyte_t>>;       // non_default_constructable


using BugOutFile  = BugOut_<std::ofstream>;              // non_default_constructable
using BugOut      = BugOut_<OBBuf>;                      // default_constructable
using BugOutGMS   = BugOut_<OBBufGMS>;                   // default_constructable
using BugOutMS    = BugOut_<OBBufMS>;                    // non_default_constructable

using BugInFile   = BugIn_<std::ifstream>;               // non_default_constructable
using BugIn       = BugIn_<IBBuf>;                       // default_constructable
using BugInGMS    = BugIn_<IBBufGMS>;                    // default_constructable
using BugInMS     = BugIn_<IBBufMS>;                     // non_default_constructable

} // namespace xmat
