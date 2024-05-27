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

// print
#include <iostream>
#include <iomanip>

#include "xutil.hpp"
#include "xmemory.hpp"


namespace xmat {
/* -----------------------------------------------------------
  byteorder
--------------------------------------------------------------
Big-endian: char[]      0x'0D'0C'0B'0A
int32                   0x'0D'0C'0B'0A
Little-endian: char[]   0x'0A'0B'0C'0D
*/

// See: stackoverflow.com/questions/105252
template<typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
T xbswap(T n) noexcept {
  constexpr size_t const N2 = sizeof(T) / 2;
  char tmp = 0;
  char* it0 = reinterpret_cast<char*>(&n);
  char* it1 = it0 + sizeof(T) - 1;
  for (size_t n = 0; n != N2; ++n, ++it0, --it1) {
    tmp = *it0;
    *it0 = *it1;
    *it1 = tmp;
  }
  return n;
}

template<bool flag_swap_bytes = false>
struct ByteRepack_ {
  template<typename T>
  static T repack(T x) { return x; }
};

template<>
struct ByteRepack_<true> {
  template<typename T> 
  static T repack(T x) { return xbswap(x); }

  static double repack(double x) { return x; }
  
  static float repack(float x) { return x; }
};

template<Endian endian> struct Pack : ByteRepack_<endian != Endian::native> { };

template<Endian endian> struct Unpack : ByteRepack_<endian != Endian::native> { };

template<typename T> T pack(T x, Endian endian = Endian::big) {
  return endian == Endian::big 
         ? Pack<Endian::big>::repack(x) 
         : Pack<Endian::little>::repack(x);
}

template<typename T> T unpack(T x, Endian endian = Endian::big) {
  return endian == Endian::big 
         ? Unpack<Endian::big>::repack(x) 
         : Unpack<Endian::little>::repack(x);
}


// -----------------------------------------------------------
// byte buffer
// -----------------------------------------------------------
//////////////////////////////////////////////////
class DataStreamError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};


///////////////////////////////////////////////////////////////
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
        throw DataStreamError("buff_storage_ms:reserve(). exceed memory_source space : ");
      }
    }
    data_ = static_cast<char*>(ptr);
    N_ = n;
  }

  char* data() noexcept { return data_; }
  const char* data() const noexcept { return data_; }
  size_t size() const noexcept { return N_; }
  size_t max_size() const noexcept { return memsource_.space(); }
  bool ready() const noexcept { return true; }

  memory_source_t memsource_;
  char* data_ = nullptr;
  size_t N_ = 0;
};


using bbuf_memsource_default = std::allocator<char>;

////////////////////////////////////////////
template<>
struct BBufStorage_<bbuf_memsource_default> {
  using memory_source_t = bbuf_memsource_default;

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
  char* data() noexcept { return impl.data(); }
  const char* data() const noexcept { return impl.data(); }
  size_t size() const noexcept { return impl.size(); }
  size_t max_size() const noexcept { return impl.max_size(); }
  bool ready() const noexcept { return true; }

 public:
  std::vector<char> impl;
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


//////////////////////////////////////////////////////////////
template<typename MemSourceT>
class OBBuf_ {
 public:
  using memory_source_t = MemSourceT;
  using storage_t = BBufStorage_<MemSourceT>;

  virtual ~OBBuf_() = default;
  OBBuf_() {}
  OBBuf_(const memory_source_t& memsrc) : storage_{memsrc} {}

  OBBuf_(const OBBuf_&) = delete;
  OBBuf_& operator=(const OBBuf_&) = delete;
  OBBuf_(OBBuf_&&) = default;
  OBBuf_& operator=(OBBuf_&&) = default;

  std::streampos tellp() const noexcept { return cursor_; }

  OBBuf_& seekp(std::streampos pos) { /*not shure  about exception*/
    assert(pos <= size_);
    cursor_ = pos; 
    return *this;
  }

  OBBuf_& seekp(std::streamoff off, std::ios_base::seekdir way) {  /*not shure  about exception*/
    cursor_ = util_seek(off, way, cursor_, size_);
    if(cursor_ > size_) throw DataStreamError("buff::seek(). index exceed: " __FILE__);
    return *this;
  }

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

  // getters
  // -------
  bool is_open() const noexcept { return is_open_ && storage_.ready(); }

  void close() noexcept { is_open_ = false; }

  MemorySource get_memsource() noexcept { return {data(), size()}; }

  const char* data() const { return storage_.data(); }
  
  char* data() { return storage_.data(); }
  
  std::size_t size() const noexcept { return size_; }
  
  storage_t& storage() { return storage_; }

 public:
  storage_t storage_;
  std::size_t size_ = 0;
  std::size_t cursor_ = 0;
  bool is_open_ = true;
};


//////////////////////////////////////////////////////////////
template<typename MemSourceT>
class IBBuf_ {
 public:
  using memory_source_t = MemSourceT;
  using storage_t = BBufStorage_<MemSourceT>;

  virtual ~IBBuf_() = default;
  IBBuf_() {}
  IBBuf_(const memory_source_t& memsrc) : storage_{memsrc} {}
  IBBuf_(const IBBuf_&) = delete;
  IBBuf_& operator=(const IBBuf_&) = delete;
  IBBuf_(IBBuf_&&) = default;
  IBBuf_& operator=(IBBuf_&&) = default;

  // content make methods
  // --------------------
  // provide space in buffer for write content bytes
  char* push_reserve(std::streamsize n) {
    size_t size_old = size_;
    size_t size_new = size_ + n;
    storage_.size_request(size_new); // throw exception
    // ---- no exception line
    size_ = size_new;
    return storage_.data() + size_old;
  }

  // allocate enougth space in buffer and write ptr to buffer
  IBBuf_& push(const char* ptr, std::streamsize n) {
    std::copy_n(ptr, n, push_reserve(n));
    return *this;
  }

  // set storage content as a buffer content
  IBBuf_& push_all() {
    size_ = storage_.request_all();
    return *this;
  }

  // like std:istream
  // ----------------
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
    if(cursor_ > size_) throw DataStreamError("buff::seek(). index exceed: " __FILE__);
    return *this;
  }

  // getters
  // -------
  bool is_open() const noexcept { return is_open_ && size() > 0; }

  void close() noexcept { is_open_ = false; }

  MemorySource get_memsource() noexcept { return {data(), size()}; }
  
  const char* data() const { return storage_.data(); }

  char* data() { return storage_.data(); }
  
  std::size_t size() const noexcept { return size_; }
  
  storage_t& storage() { return storage_; }

 public:
  storage_t storage_;
  std::size_t size_ = 0;
  std::size_t cursor_ = 0;
  bool is_open_ = true;
};


///////////////////////////////////////////
template<typename OBBuffT, Endian endian_tag> 
struct ODStream_ : public OBBuffT {
  static_assert(std::has_virtual_destructor<OBBuffT>::value, "asd");
  using base_t = OBBuffT;
  using repack_t = Pack<endian_tag>;

  static const Endian endian = endian_tag;

  template<typename T, 
    Endian endian_tag_ = endian_tag, 
    std::enable_if_t<endian_tag_ != Endian::native && (sizeof(T) > 1), int> = 0>
  ODStream_& write(const T* data, size_t n) {
    T tmp;
    for (; n != 0; --n, ++data) {
      tmp = repack_t::repack(*data);
      base_t::write(reinterpret_cast<const char*>(&tmp), sizeof(T));
    }
    return *this;
  };

  template<typename T, 
    Endian endian_tag_ = endian_tag, 
    std::enable_if_t<endian_tag_ == Endian::native || (sizeof(T) <= 1), int> = 0>
  ODStream_& write(const T* data, size_t n) {
    base_t::write(reinterpret_cast<const char*>(data), n*sizeof(T));
    return *this;
  };

  template<typename T>
  ODStream_& write(T x) { return write(&x, 1); }

  template<typename T>
  ODStream_& operator<<(T x) { return write(&x, 1); }
};


/// \tparam IBBuff      input buffer like: std::ifstream, xmat::OBBuf_<>
/// \tparam endian_tag  xmat::Endian
///////////////////////////////////////////
template<typename IBBuffT, Endian endian_tag>
struct IDStream_ : public IBBuffT {
  static_assert(std::has_virtual_destructor<IBBuffT>::value, "asd");

  using base_t = IBBuffT;
  using repack_t = Unpack<endian_tag>;

  static const Endian endian = endian_tag;

  using base_t::base_t;

  template<typename T, 
    Endian endian_tag_ = endian_tag, 
    std::enable_if_t<endian_tag_ != Endian::native, int> = 0>
  IDStream_& read(T* data, size_t n) {
    T tmp;
    for (; n != 0; --n, ++data) {
      base_t::read(reinterpret_cast<const char*>(&tmp), sizeof(T));
      *data = repack_t::repack(tmp);
    }
    return *this;
  };

  template<typename T,
    Endian endian_tag_ = endian_tag, 
    std::enable_if_t<endian_tag_ == Endian::native, int> = 0>
  IDStream_& read(T* data, size_t n) {
    base_t::read(reinterpret_cast<char*>(data), n*sizeof(T));
    return *this;
  };

  template<typename T>
  IDStream_& read(T& x) { return read(&x, 1); }

  template<typename T>
  IDStream_& operator>>(T& x) { return read(&x, 1); }
};


//////////////////////////////////////////////////////////////
// file format
//////////////////////////////////////////////////////////////
namespace sf { // file serialized format
using xsize_t  = std::uint64_t;
using xuint8_t = std::uint8_t;

// header
const size_t k_sizeof_xsize_t = sizeof(xsize_t);
const size_t k_sign_size = 4;
const size_t k_bom = 0x01;

// block
const xuint8_t k_max_ndim = 8;
const xuint8_t k_max_name = 32;
} // namespace sf

// register types for format
// -------------------------
template<typename T>
struct DataStreamType {
  static const bool enabled = false;
  static constexpr const char* const label = "undef";
};


#define XMAT_REGTYPE(Id, Size, Label, Type)                           \
template<> struct DataStreamType<Type> {                              \
  static const bool enabled = true;                                   \
  static_assert(sizeof(Type) == Size, "sizeof(" #Type ") !=" #Size);  \
  static const sf::xuint8_t id = Id;                                  \
  static const size_t size = Size;                              \
  static constexpr const char* const label = Label;                   \
}; \

// 
struct xvoid { };

XMAT_REGTYPE(0x00,    1,    "v",    xvoid);
XMAT_REGTYPE(0x01,    1,    "c",    char);
// XMAT_REGTYPE(0x02,    1,    "?",    bool);   // not shure how to implement

XMAT_REGTYPE(0x10,    1,    "i0",   std::int8_t);
XMAT_REGTYPE(0x11,    2,    "i1",   std::int16_t);
XMAT_REGTYPE(0x12,    4,    "i2",   std::int32_t);
XMAT_REGTYPE(0x13,    8,    "i3",   std::int64_t);

XMAT_REGTYPE(0x20,    2,    "i0",   std::complex<std::int8_t>);
XMAT_REGTYPE(0x21,    4,    "i1",   std::complex<std::int16_t>);
XMAT_REGTYPE(0x22,    8,    "i2",   std::complex<std::int32_t>);
XMAT_REGTYPE(0x23,    16,   "i3",   std::complex<std::int64_t>);

XMAT_REGTYPE(0x30,    1,    "u0",   std::uint8_t);
XMAT_REGTYPE(0x31,    2,    "u1",   std::uint16_t);
XMAT_REGTYPE(0x32,    4,    "u2",   std::uint32_t);
XMAT_REGTYPE(0x33,    8,    "u3",   std::uint64_t);

XMAT_REGTYPE(0x40,    2,    "u0",   std::complex<std::uint8_t>);
XMAT_REGTYPE(0x41,    4,    "u1",   std::complex<std::uint16_t>);
XMAT_REGTYPE(0x42,    8,    "u2",   std::complex<std::uint32_t>);
XMAT_REGTYPE(0x43,    16,   "u3",   std::complex<std::uint64_t>);

XMAT_REGTYPE(0x52,    4,    "f2",   float);
XMAT_REGTYPE(0x53,    8,    "f3",   double);

XMAT_REGTYPE(0x62,    8,    "F2",   std::complex<float>);
XMAT_REGTYPE(0x63,    16,   "F3",   std::complex<double>);


inline constexpr size_t sizeof_data_stream_type(sf::xuint8_t id) noexcept {
  using std::complex;

  switch (id) {
  case DataStreamType<  xvoid                   >::id:  return DataStreamType<xvoid>::size;
  case DataStreamType<  char                    >::id:  return DataStreamType<char>::size;
  // case DataStreamType<  bool                    >::id:  return DataStreamType<bool>::size;

  case DataStreamType<  std::int8_t             >::id:  return DataStreamType<std::int8_t>::size;
  case DataStreamType<  std::int16_t            >::id:  return DataStreamType<std::int16_t>::size;
  case DataStreamType<  std::int32_t            >::id:  return DataStreamType<std::int32_t>::size;
  case DataStreamType<  std::int64_t            >::id:  return DataStreamType<std::int64_t>::size;

  case DataStreamType<  complex<std::int8_t>    >::id:  return DataStreamType<complex<std::int8_t>>::size;
  case DataStreamType<  complex<std::int16_t>   >::id:  return DataStreamType<complex<std::int16_t>>::size;
  case DataStreamType<  complex<std::int32_t>   >::id:  return DataStreamType<complex<std::int32_t>>::size;
  case DataStreamType<  complex<std::int64_t>   >::id:  return DataStreamType<complex<std::int64_t>>::size;

  case DataStreamType<  std::uint8_t            >::id:  return DataStreamType<std::uint8_t>::size;
  case DataStreamType<  std::uint16_t           >::id:  return DataStreamType<std::uint16_t>::size;
  case DataStreamType<  std::uint32_t           >::id:  return DataStreamType<std::uint32_t>::size;
  case DataStreamType<  std::uint64_t           >::id:  return DataStreamType<std::uint64_t>::size;

  case DataStreamType<  complex<std::uint8_t>   >::id:  return DataStreamType<complex<std::uint8_t>>::size;
  case DataStreamType<  complex<std::uint16_t>  >::id:  return DataStreamType<complex<std::uint16_t>>::size;
  case DataStreamType<  complex<std::uint32_t>  >::id:  return DataStreamType<complex<std::uint32_t>>::size;
  case DataStreamType<  complex<std::uint64_t>  >::id:  return DataStreamType<complex<std::uint64_t>>::size;

  case DataStreamType<  float                   >::id:  return DataStreamType<float>::size;
  case DataStreamType<  double                  >::id:  return DataStreamType<double>::size;

  case DataStreamType<  complex<float>          >::id:  return DataStreamType<complex<float>>::size;
  case DataStreamType<  complex<double>         >::id:  return DataStreamType<complex<double>>::size;
  default: return 0;
  }
}


//////////////////////////////
struct XHead {
  // ODStreamT = ODStream_<>
  template<typename ODStreamT>
  ODStreamT& dump(ODStreamT& os) const {
    os.write(sign_, sf::k_sign_size);
    os.write(bom_);
    os.write(total_size_);
    os.write(i_);
    os.write(s_);
    os.write(b_);
    return os;
  }

  // IDStreamT = IDStream_<>
  template<typename IDStreamT>
  IDStreamT& load(IDStreamT& is) {
    is.read(sign_, sf::k_sign_size);
    is.read(bom_);
    is.read(total_size_);
    is.read(i_);
    is.read(s_);
    is.read(b_);
    return is;
  }

  bool check() const noexcept { return true; }

  // getters
  // -------
  size_t total() const noexcept { return total_size_; }

  size_t sizeof_int() const noexcept { return i_; }

  size_t maxndim() const noexcept { return s_; }

  size_t maxname() const noexcept { return b_; }

  static constexpr size_t nbytes() noexcept { 
    constexpr size_t n = sf::k_sign_size + 2 + sf::k_sizeof_xsize_t + 3;
    return n;
  }

 public:
  char          sign_[sf::k_sign_size + 1] = "xmat";      // Sign
  std::uint16_t bom_        = sf::k_bom;                  // BOM
  sf::xsize_t   total_size_ = 0;                          // Total Size
  sf::xuint8_t  i_          = sf::k_sizeof_xsize_t;       // I
  sf::xuint8_t  s_          = sf::k_max_ndim;             // S
  sf::xuint8_t  b_          = sf::k_max_name;             // B
};


//////////////////////////////
struct XBlock {
  using shape_t = std::array<sf::xsize_t, sf::k_max_ndim>;

  /// \tparam ODStreamT = output stream like: ODStream_<>
  template<typename ODStreamT>
  ODStreamT& dump(ODStreamT& os) const {
    os.write(o_);
    os.write(t_);
    os.write(s_);
    os.write(b_);
    for (int n = 0; n < 4; ++n) os.write(char{0});
    os.write(shape_.data(), s_);
    assert(std::strlen(name_.data()) == b_);
    os.write(name_.data(), b_);
    return os;
  }

  /// \tparam IDStreamT = input stream like: IDStream_<>
  template<typename IDStreamT>
  IDStreamT& load(IDStreamT& is) {
    // reset
    shape_.fill(0);
    name_.fill(0);

    pos_ = is.tellg();
    is.read(o_);
    is.read(t_);
    is.read(s_);
    is.read(b_);
    char zero; // just check for zero and skip
    for (int n = 0; n < 4; ++n) { 
      is.read(zero); 
      assert(zero == '\0'); 
    }
    is.read(shape_.data(), s_);
    is.read(name_.data(), b_);
    return is;
  }

  bool check() const noexcept {
    if (o_ != 'C' || o_ != 'F') {  return false; }
    if (sizeof_data_stream_type(t_) == 0) { return false; }
    if (s_ > sf::k_max_ndim ) { return false; }
    if (!std::all_of(shape_.begin() + s_, shape_.end(), 
                     [](sf::xsize_t i){return i == 0; })) {
      return false;
    }
    if (b_ > sf::k_max_name ) { return false; }
    return true; 
  }

  // getters
  // -------
  bool is_valid() const noexcept { return pos_ != 0; }

  char morder() const noexcept { return o_; }

  sf::xuint8_t tid() const noexcept { return t_; }

  size_t typesize() const noexcept { return sizeof_data_stream_type(tid()); }

  size_t ndim() const noexcept { return s_; }

  shape_t&       shape() noexcept { return shape_; }
  const shape_t& shape() const noexcept { return shape_; }

  size_t namelen() const noexcept { return b_; }

  char*       name() noexcept { return name_.data(); }
  const char* name() const noexcept { return name_.data(); }

  size_t nbytes() const noexcept { return 8 + sf::k_sizeof_xsize_t * ndim() + namelen(); }

  size_t pos() const noexcept { return pos_; }

  size_t data_pos() const noexcept { return pos_ + nbytes(); }

  std::size_t data_nbytes() const noexcept { return numel() * typesize(); }

  size_t numel() const noexcept {
    sf::xuint8_t N = 1;
    for (auto it = shape_.begin(), end = shape_.begin() + s_; it != end; ++it) {
      N *= *it;
    }
    return N;
  }

 public:
  char          o_ = 'C';
  char          t_ =  DataStreamType<xvoid>::id;
  sf::xuint8_t  s_ = 0;
  sf::xuint8_t  b_ = 0;
  shape_t       shape_ = {};
  std::array<char, sf::k_max_name + 1>  name_  = {};

  char* ptr_ = nullptr;
  const char* cptr_ = nullptr;
  size_t pos_ = 0;
};


//////////////////////////////////////////////////////////////////
template<typename T, typename Enable = void> struct Serializer { };

namespace serial {
template<typename T, typename Enable = void>
struct Dump { 
  static const bool enabled = false;

  template<typename ODStreamT>
  static void dump(XBlock&, ODStreamT&, const T&) { }
};

template<typename T, typename Enable = void>
struct LoadTo { 
  static const bool enabled = false;

  template<typename IDStreamT>
  static void load(const XBlock&, IDStreamT&, T&) { }
};

template<typename T, typename Enable = void>
struct Load { 
  static const bool enabled = false;

  template<typename IDStreamT>
  static T load(const XBlock&, IDStreamT&) { return T{}; }
};

template<typename T, typename Enable = void>
struct DumpPtr {
  static const bool enabled = false;

  template<typename ODStreamT>
  static void dump(XBlock&, ODStreamT&, const T*, size_t) { }
};

template<typename T, typename Enable = void>
struct LoadPtr {
  static const bool enabled = false;

  template<typename IDStreamT>
  static void load(const XBlock&, IDStreamT&, T*) { }
};

template<typename T, typename Enable = void>
struct LoadArgs { 
  static const bool enabled = false;

  template<typename IDStreamT, typename ... Args>
  static T load(const XBlock&, IDStreamT&, Args&&... args) {
    // T{std::forward<Args>(args)...};
    return T{};
  }
};
} // namespace serial


/// \tparam ODStreamT output data-stream like:
///   ODStream_<std::ofstream,                  endian>
///   ODStream_<OBBuf_<bbuf_memsource_default>, endian>
////////////////////////////
template<typename ODStreamT>
class OMapStream_ {
 public:
  using odstream_t = ODStreamT;

  virtual ~OMapStream_() { close(); };

  OMapStream_() {
    if(!ods_.is_open()) { // for case of xmat::OBBuf_
      return;
    }
    head_.dump(ods_);
  }

  OMapStream_(odstream_t&& ods) : ods_{std::move(ods)} {
    assert(ods.is_open());
    head_.dump(ods_);
  }

  void close() noexcept {
    if(!ods_.is_open()) { return; }
    // save total size
    ods_.seekp(0, std::ios::end);
    head_.total_size_ = ods_.tellp();
    ods_.seekp(6);
    ods_.write(head_.total_size_);
    ods_.close();
  }  

  template<typename T, typename std::enable_if_t<serial::Dump<T>::enabled, int> = 0>
  XBlock setitem(VString name, const T& x) {
    XBlock block;
    assign(block.name_, name.ptr);
    block.b_ = std::strlen(name.ptr);
    serial::Dump<T>::dump(block, ods_, x);
    return block;
  }

  // not welcome for using
  template<typename T>
  XBlock setitem_n(VString name, const T* x, size_t n) {
    XBlock block;
    assign(block.name_, name.ptr);
    block.b_ = std::strlen(name.ptr);
    serial::DumpPtr<T>::dump(block, ods_, x, n);
    return block;
  }
  
  // getters
  // -------
  XHead& head() { return head_; }

  const XHead& head() const { return head_; }

  odstream_t& stream() { return ods_; }

  const odstream_t& stream() const { return ods_; }

 private:
  XHead head_;
  odstream_t ods_;
};


/// \tparam IDS input data-stream like:
///   IDStream_<std::ifstream,                  endian>
///   IDStream_<IBBuf_<bbuf_memsource_default>, endian>
////////////////////////////
template<typename IDStreamT>
class IMapStream_ {
 public:
  using idstream_t = IDStreamT;

  virtual ~IMapStream_() { close(); }

  IMapStream_() = default;

  IMapStream_(idstream_t&& ids, bool flag_scan_head = true) : ids_{std::move(ids)} {
    if (flag_scan_head && ids_.is_open()) { 
      scan_head(); 
    }
  }

  void close() noexcept { ids_.close(); }

  ////////////////
  class Iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = XBlock;
    using pointer           = XBlock*;
    using reference         = XBlock&;

    // used for end-iterator. it'll have block_.pos_ = 0
    Iterator(size_t endpos) : endpos_{endpos} {}

    Iterator(idstream_t* ids, size_t endpos) : ids_{ids}, endpos_{endpos} {
      ids_->seekg(XHead::nbytes());
      ++(*this);
    }

    const XBlock& operator*() const { return block_; }

    const XBlock* operator->() const { return &block_; }

    Iterator& operator++() {
      if (ids_->tellg() >= endpos_) {
        block_ = XBlock{};
      }
      else {
        block_.load(*ids_);
        ids_->seekg(block_.numel() * block_.typesize(), std::ios::cur);
      }
      return *this;
    }

    Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

    friend bool operator== (const Iterator& a, const Iterator& b) { return a.block_.pos_ == b.block_.pos_; }

    friend bool operator!= (const Iterator& a, const Iterator& b) { return !(a == b); }

    // load data
    // ---------
    template<typename T, typename std::enable_if_t<serial::Load<T>::enabled, int> = 0> 
    T get() {
      get_precond();
      return serial::Load<T>::load(block_, *ids_);
    }

    template<typename T, typename Allocator, typename std::enable_if_t<serial::LoadArgs<T>::enabled, int> = 0> 
    T get(Allocator&& alloc) {
      get_precond();
      return serial::LoadArgs<T>::load(block_, *ids_, std::forward<Allocator>(alloc));
    }

    template<typename T, typename std::enable_if_t<serial::LoadTo<T>::enabled, int> = 0> 
    T& get_to(T& y) {
      get_precond();
      serial::LoadTo<T>::load(block_, *ids_, y);
      return y;
    }

    template<typename T, typename std::enable_if_t<serial::LoadPtr<T>::enabled, int> = 0> 
    T* get_to(T* y) {
      get_precond();
      serial::LoadPtr<T>::load(block_, *ids_, y);
      return y;
    }

    // getters
    // -------
    void get_precond() {
      if(!is_valid()) {
        throw DataStreamError{"bugin.iterator.get<T>: access to empty block"};
      }
      ids_->seekg(block_.data_pos(), std::ios_base::beg);
    }

    bool is_valid() const noexcept { return block_.is_valid(); }

    operator bool()const noexcept { return is_valid(); }

   public:
    idstream_t* ids_ = nullptr;
    XBlock block_;
    size_t endpos_ = 0;
  }; // Iterator

  // element access
  // --------------
  Iterator begin() { return Iterator{&ids_, head_.total_size_}; }

  Iterator end() { return Iterator{head_.total_size_}; }

  Iterator at(VString name) {
    auto it = std::find_if(
      begin(),
      end(),
      [name](const XBlock& block){
        return std::strcmp(name.ptr, block.name()) == 0;
      }
    );
    return it;
  }

  void scan_head() {
    assert(head_.total() == 0);
    if(ids_.size() < XHead::nbytes()) {
      throw DataStreamError("xmat::bugin.scan_head(). stream buffer isn't filled");
    }
    head_.load(ids_);
  }

  // getters
  // -------
  bool empty() const noexcept { return !head_.total_size_; }

  XHead& head() { return head_; }

  const XHead& head() const { return head_; }
  
  idstream_t& stream() { return ids_; }

  const idstream_t& stream() const { return ids_; }

 private:
  XHead head_;
  idstream_t ids_;
};


//////////////////////////////
// aliases
// ---------------------------
using OBBuf       = OBBuf_<bbuf_memsource_default>;   // default_constructable
using OBBufGMS    = OBBuf_<AllocatorMSGlobal<char>>;  // default_constructable
using OBBufMS     = OBBuf_<AllocatorMSRef<char>>;     // non_default_constructable

using IBBuf       = IBBuf_<bbuf_memsource_default>;   // default_constructable
using IBBufGMS    = IBBuf_<AllocatorMSGlobal<char>>;  // default_constructable
using IBBufMS     = IBBuf_<AllocatorMSRef<char>>;     // non_default_constructable

template<Endian endian> using ODStreamFile  = ODStream_<std::ofstream,  endian>;
template<Endian endian> using ODStream      = ODStream_<OBBuf,          endian>;
template<Endian endian> using ODStreamGMS   = ODStream_<OBBufGMS,       endian>;
template<Endian endian> using ODStreamMS    = ODStream_<OBBufMS,        endian>;

template<Endian endian> using IDStreamFile  = IDStream_<std::ifstream,  endian>;
template<Endian endian> using IDStream      = IDStream_<IBBuf,          endian>;
template<Endian endian> using IDStreamGMS   = IDStream_<IBBufGMS,       endian>;
template<Endian endian> using IDStreamMS    = IDStream_<IBBufMS,        endian>;

template<Endian endian = Endian::native> using OMapStreamFile  = OMapStream_<ODStreamFile<endian>>;
template<Endian endian = Endian::native> using OMapStream      = OMapStream_<ODStream<endian>>;
template<Endian endian = Endian::native> using OMapStreamGMS   = OMapStream_<ODStreamGMS<endian>>;
template<Endian endian = Endian::native> using OMapStreamMS    = OMapStream_<ODStreamMS<endian>>;

template<Endian endian = Endian::native> using IMapStreamFile  = IMapStream_<IDStreamFile<endian>>;
template<Endian endian = Endian::native> using IMapStream      = IMapStream_<IDStream<endian>>;
template<Endian endian = Endian::native> using IMapStreamGMS   = IMapStream_<IDStreamGMS<endian>>;
template<Endian endian = Endian::native> using IMapStreamMS    = IMapStream_<IDStreamMS<endian>>;
} // namespace xmat
