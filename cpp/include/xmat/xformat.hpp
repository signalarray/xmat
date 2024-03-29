#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <complex>
#include <iterator>
#include <memory>
#include <exception>

#include "xutil.hpp"


namespace xmat {

using uint = std::size_t;
using xint_t = std::uint64_t;
using byte_t = char;

// constants
const std::uint8_t kXintSize = sizeof(xint_t);
const std::uint8_t kSignatureSize = 4;

// Head
static constexpr char kFormatSignature[kSignatureSize] = "XYZ";
static const std::uint8_t kMaxBlockNameLen = 32;
static const std::uint8_t kMaxTypeNameLen = 8;
static const std::uint8_t kMaxNDimXMat = 8;
static const std::uint8_t kHeadSize = {8 + 2*kSignatureSize + 4}; // size in bytes

// Block
static constexpr char kBlockSignatureBegin[kSignatureSize] = "<#>";
static constexpr char kBlockSignatureEnd[kSignatureSize] = ">#<";
static const std::uint8_t kBlockSize = {kMaxBlockNameLen + kMaxTypeNameLen + 
  (kMaxNDimXMat + 1) * kXintSize + (1 + 1)*1 + 2 * kSignatureSize};


// supported types registration
// ----------------------------
template <typename T>
constexpr bool alwaysFalse() { return false; };

template<typename T>
struct TypeInfo {
  // static_assert(alwaysFalse<T>(), "using not registered data_type");
  static const bool registered = false;
  static const std::uint8_t size = 0;
  // static constexpr char name[kMaxTypeNameLen] = "undef";
  static constexpr const char* const name = "undef";
};

#define XMAT_ADDTYPE(T, Name) \
template<> struct TypeInfo<T> {                            \
  static const bool registered = true;                     \
  static const std::uint8_t size = sizeof(T);              \
  static constexpr const char* const name = Name;                       \
};                                                         \

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


// streams
// -------
// StreamBase impl_ <>---------- Stream stream <>--- Output
//            <- StreamFileOut                 <>--- Input
//            <- StreamFileIn
//            <- StreamBytes

enum class Endian : std::uint8_t { 
  little = 0, 
  big = 1, 
  native = 2 
};

enum class StreamMode : char {
  in = 'i',
  out = 'o',
  undef = 'x'
};

class StreamBase {
 public:
  virtual ~StreamBase() {}

  virtual void close() = 0;
  virtual uint tell() = 0;
  virtual void seek(std::streamoff off, std::ios::seekdir way) = 0;
  virtual bool eof() const noexcept = 0;
  virtual uint size() noexcept = 0;
  virtual bool is_open() const noexcept = 0;

  virtual void write__(const char* ptr, std::streamsize n) {std::runtime_error("can't write");}
  virtual void read__(char* ptr, std::streamsize n)  {std::runtime_error("can't read");}

  Endian endian_{Endian::native};        // not implemented
  StreamMode mode_{StreamMode::undef};

 public: // different types
  template<typename T>
  uint write(const T& X) {
    write__(reinterpret_cast<const char*>(&X), sizeof(T));
    return 1;
  }

  template<typename T>
  uint write(const T* ptr_x, uint count) {
    write__(reinterpret_cast<const char*>(ptr_x), count*sizeof(T));
    return 1;
  }

  template<typename T>
  uint read(T& X) {
    read__(reinterpret_cast<char*>(&X), sizeof(T));
    return 1;
  }

  template<typename T>
  uint read(T* ptr_x, uint count) {
    read__(reinterpret_cast<char*>(ptr_x), count*sizeof(T));
    return 1;
  }
};


class StreamFileOut : public StreamBase {
 public:
  StreamFileOut() = default;
  
  StreamFileOut(const char* filename) : os_(filename, std::ios::binary) { 
    assert(os_.is_open()); 
    mode_ = StreamMode::out;
  }

  StreamFileOut(const std::string& filename) : StreamFileOut(filename.c_str()) {}
  StreamFileOut(const StreamFileOut& ) = delete;
  StreamFileOut(StreamFileOut&& ) = default;

  virtual void close() override {os_.close();}
  virtual uint tell() override {return os_.tellp();}
  virtual void seek(std::streamoff off, std::ios_base::seekdir way) override {os_.seekp(off, way);}
  virtual bool eof() const noexcept override {return os_.eof();}
  
  virtual uint size() noexcept override {
    const std::streampos pos = os_.tellp();
    os_.seekp(0, std::ios::end);
    const std::streampos sz = os_.tellp();
    os_.seekp(pos, std::ios::beg);
    return static_cast<uint>(sz);
  }
  
  virtual bool is_open() const noexcept override {return os_.is_open();}

  virtual void write__(const char* ptr, std::streamsize n) override { os_.write(ptr, n); }

  std::ofstream os_;
};


class StreamFileIn : public StreamBase {
 public:
  StreamFileIn() = default;

  StreamFileIn(const char* filename) : is_(filename, std::ios::binary) {
    assert(is_.is_open());
    mode_ = StreamMode::in;
  }

  StreamFileIn(const std::string& filename) : StreamFileIn(filename.c_str()) {}
  StreamFileIn(const StreamFileIn& ) = delete;
  StreamFileIn(StreamFileIn&& ) = default;

  virtual void close() override {is_.close();}
  virtual uint tell() override {return is_.tellg();}
  virtual void seek(std::streamoff off, std::ios_base::seekdir way) override {is_.seekg(off, way);}
  virtual bool eof() const noexcept override {return is_.eof();}
  
  virtual uint size() noexcept override {
    const std::streampos pos = is_.tellg();
    is_.seekg(0, std::ios::end);
    const std::streampos sz = is_.tellg();
    is_.seekg(pos, std::ios::beg);
    return static_cast<uint>(sz);
  }
  
  virtual bool is_open() const noexcept override {return is_.is_open();}

  virtual void read__(char* ptr, std::streamsize n) override { is_.read(ptr, n); }

  std::ifstream is_;
};


class StreamBytes : public StreamBase {
 public:
  StreamBytes(StreamMode mode, 
              byte_t* buff, 
              uint buff_size, 
              Endian endian = Endian::native) 
    : is_open_{true}, buff_{buff}, capacity_{buff_size},
      size_(mode == StreamMode::out ? 0 : buff_size) {
    mode_ = mode; 
    endian_ = endian;
  }

  StreamBytes(StreamMode mode, Endian endian = Endian::native)
    : StreamBytes{mode, nullptr, 0, endian} {}

  // output stream with dynamic buffer
  StreamBytes(Endian endian = Endian::native)
    : StreamBytes{StreamMode::out, nullptr, 0, endian} {}

  StreamBytes(const StreamBytes& ) = delete;
  StreamBytes(StreamBytes&& ) = default;

  virtual void close() override { is_open_ = false;}
  virtual uint tell() override {return cursor_;}
  
  virtual void seek(std::streamoff off, std::ios_base::seekdir way) override {
    const auto cursor = cursor_;
    if (way == std::ios_base::beg) cursor_ = off;
    else if (way == std::ios_base::cur) cursor_ += off;
    else if (way == std::ios_base::end) cursor_ = vec_.size() - off;
    else { assert(false); }
  }

  virtual bool eof() const noexcept override {return cursor_ == vec_.size();}
  virtual uint size() noexcept override { return size_; }
  virtual bool is_open() const noexcept override {return is_open_;}

  virtual void write__(const char* ptr, std::streamsize n) override {
    assert(mode_ == StreamMode::out);
    if (buff_) {
      assert(cursor_ + n <= capacity_);
      if (cursor_ + n < size_) {
        return;
      }
      std::copy_n(ptr, n, buff_ + cursor_);
    } else {
      // print to buff must support also writing in the middle if content.
      assert(cursor_ <= vec_.size());
      const auto next_size = std::max(cursor_ + n, size_);
      vec_.resize(next_size);
      std::copy_n(ptr, n, vec_.begin() + cursor_);
    }
    cursor_ += n;
    size_ = std::max(size_, cursor_);
  }

  virtual void read__(char* ptr, std::streamsize n) override {
    assert(mode_ == StreamMode::in);
    assert(cursor_ + n <= size_);
    std::copy_n(buff() + cursor_, n, ptr);
    cursor_ += n;
  }

 public: // specific
  void clear() noexcept { cursor_ = 0; size_ = mode_ == StreamMode::out ? 0 : size_; } // bad line
  bool is_empty() const noexcept { return cursor_ == 0; }  // bad line

  // change buff size
  // just for input mode and non-fixed buff
  void reserve(uint n) {
    assert(mode_ == StreamMode::in);
    assert(!isfixed()); 
    if(!isfixed()) { 
      vec_.resize(n);
      size_ = vec_.size();
    }
  }

 public: // buffer access
  bool isempty() const noexcept { return size_ == 0; }
  bool isfixed() const noexcept { return buff_; }

  byte_t* buff() noexcept {
    if (isfixed()) return buff_;
    else return vec_.data();
  }

  const byte_t* buff() const noexcept {
    if (isfixed()) return buff_;
    else return vec_.data();
  }

  uint capacity() const noexcept {
    if(isfixed()) return capacity_;
    else return vec_.size();
  }

  byte_t* begin() noexcept { return buff(); }
  byte_t* end() noexcept { return buff() + size_; }

  const byte_t* cbegin() const noexcept {return buff();}
  const byte_t* cend() const noexcept {return buff() + size_;}

 public: // private:
  bool is_open_ = false;
  uint cursor_{0};
  uint size_{0};              // used size

  byte_t* buff_ = nullptr;    // fixed buffer
  uint capacity_{0};         // fixed buffer

  std::vector<byte_t> vec_;   // dynamic buffer
};


// just contains objects to not controll it's deleting
class StreamObj {
 public:
  StreamObj(StreamFileOut&& stream) : stream_file_out_{std::move(stream)}, stream_{&stream_file_out_}, mode_{0} {}
  StreamObj(StreamFileIn&& stream) : stream_file_in_{std::move(stream)}, stream_{&stream_file_in_}, mode_{1} {}
  StreamObj(StreamBytes&& stream) : stream_bytes_{std::move(stream)}, stream_{&stream_bytes_}, mode_{2} {}

  StreamObj(const StreamObj& obj) = delete;

  StreamObj(StreamObj&& obj) 
    : mode_{obj.mode_}, 
      stream_file_out_(std::move(obj.stream_file_out_)), 
      stream_file_in_(std::move(obj.stream_file_in_)),
      stream_bytes_(std::move(obj.stream_bytes_)) {
    if (mode_ == 0) stream_ = &stream_file_out_;
    else if (mode_ == 1) stream_ = &stream_file_in_;
    else if (mode_ == 2) stream_ = &stream_bytes_;
    else {assert(false);}
  }

  StreamBase* stream() & {return stream_; }
  int mode() const noexcept { return mode_; }
  bool mode_file() const noexcept { return mode_ == 0 || mode_ == 1; }
  bool mode_bytes() const noexcept { return mode_ == 2; }

 // private:
  StreamFileOut stream_file_out_;
  StreamFileIn stream_file_in_;
  StreamBytes stream_bytes_;
  const int mode_ = -1; // 0 - file_out, 1 - file_in, 2 - bytes
  StreamBase* stream_;
};


// format implementation
// ---------------------
class Head final {
 public:
  uint dump(StreamBase& stream) {
    stream.write(xint_t{0});
    stream.write(format_signature_, kSignatureSize);
    stream.write(xint_size);
    stream.write(max_block_name_len);
    stream.write(max_type_name_len);
    stream.write(max_ndim);
    stream.write(format_signature_, kSignatureSize);
    return 1;
  }

  uint load(StreamBase& stream) {
    stream.read(total_size);
    stream.read(format_signature_, kSignatureSize);
    stream.read(xint_size);
    stream.read(max_block_name_len);
    stream.read(max_type_name_len);
    stream.read(max_ndim);
  
    char format_signature_end[kSignatureSize] = {};
    stream.read(format_signature_end, kSignatureSize);
    return 1;
  }

 public:
  char format_signature_[kSignatureSize] = "XYZ";
  std::uint8_t xint_size = kXintSize;
  std::uint8_t max_block_name_len = kMaxBlockNameLen;
  std::uint8_t max_type_name_len = kMaxTypeNameLen;
  std::uint8_t max_ndim = kMaxNDimXMat;

  uint total_size = 0;
};


class Block final {
 public:
  using shape_t = std::array<xint_t, kMaxNDimXMat>;

  Block() = default;

  uint dump(StreamBase& stream) {
    stream.write(kBlockSignatureBegin, kSignatureSize);
    stream.write(name_.data(), name_.size());
    stream.write(typename_.data(), typename_.size());
    stream.write(shape_.data(), shape_.size());
    stream.write(numel_);
    stream.write(typesize_);
    stream.write(ndim_);
    stream.write(kBlockSignatureEnd, kSignatureSize);
    return 1;
  }

  uint load(StreamBase& stream) {
    char sig_0[kSignatureSize] = {};
    char sig_1[kSignatureSize] = {};

    stream.read(sig_0, kSignatureSize);
    stream.read(name_.data(), name_.size());
    stream.read(typename_.data(), typename_.size());
    stream.read(shape_.data(), shape_.size());
    stream.read(numel_);
    stream.read(typesize_);
    stream.read(ndim_);
    stream.read(sig_1, kSignatureSize);
    return 1;
  }

  template<typename T>
  bool check_element() const {
    bool out = std::strcmp(TypeInfo<T>::name, typename_.cbegin()) == 0
            && TypeInfo<T>::size == typesize_;
    return out;
  }

 public:
  std::array<char, kMaxBlockNameLen> name_ = {};
  std::array<char, kMaxTypeNameLen> typename_ = {};
  shape_t shape_ = {0};
  xint_t numel_ = 0;
  std::uint8_t typesize_ = 0;
  std::uint8_t ndim_ = 0;

  const char* ptr_ = nullptr;
  uint pos_ = 0;
};


class IOBase {
 public:
  IOBase(StreamObj&& stream_obj) : stream_obj_{std::move(stream_obj)}, stream_{stream_obj_.stream_} {}

  // getters
  // -------
  bool is_open() const noexcept {return stream_->is_open(); }
  bool mode_file() const noexcept {return stream_obj_.mode_file(); }
  bool mode_bytes() const noexcept {return stream_obj_.mode_bytes(); }

  const Head& header() const noexcept {return header_;}
  StreamBytes& stream_bytes() noexcept {return stream_obj_.stream_bytes_; }
  const StreamBytes& stream_bytes() const noexcept {return stream_obj_.stream_bytes_; }
  byte_t* buff() noexcept { return stream_obj_.stream_bytes_.buff(); }
  const byte_t* buff() const noexcept { return stream_obj_.stream_bytes_.buff(); }
  uint size() const noexcept { return stream_->size(); }
  uint capacity() const noexcept { return stream_obj_.stream_bytes_.capacity(); }

 public: /*protected*/
  Head header_{};
  StreamObj stream_obj_;
  StreamBase* stream_ = nullptr;
};


template<typename T, typename Enable = void>
struct XSerial {
  // static Block dump(const T& value);
  // static T load(const Block& block, StreamBase& stream);
};


class Output : public IOBase {
 public:
  virtual ~Output() { close(); }

  Output(StreamObj&& stream_obj) : IOBase{std::move(stream_obj)} {
    assert(stream_->tell() == 0);
    header_.dump(*stream_);
  }

  void clear() noexcept {
    assert(stream_obj_.mode_bytes()); // just for bytes now
    stream_obj_.stream_bytes_.clear();
    header_.dump(*stream_);
  }

  template<typename T>
  void setitem(const char* name, const T& x) {
    Block block = XSerial<T>::dump(x);
    add_impl(name, block);
  }

  Block add_impl(const char* name, Block block) {
    assert(is_open());
    assert(block.ptr_);
    assign(block.name_, name);
    block.dump(*stream_);
    stream_->write(block.ptr_, block.typesize_*block.numel_);
    return block;
  }

  void close() {
    if (!stream_->is_open()) { return; }
    header_.total_size = stream_->size();
    stream_->seek(0, std::ios_base::beg);
    stream_->write(static_cast<xint_t>(stream_->size()));
    stream_->close();
  }
};


class Input : public IOBase {
 public:
  using block_iter_t = std::vector<Block>::const_iterator;

  Input(StreamObj&& stream_obj) : IOBase{std::move(stream_obj)} {
    if (stream_->mode_ != StreamMode::in) {
      throw std::runtime_error("wrong stream mode. stream_->mode_ != StreamMode::in");
    }
    if(!(mode_bytes() && size() == 0)) {  // uninitialized bytes buffer. 
      scan(); 
    }
  }

  void clear() noexcept {
    assert(stream_obj_.mode_bytes()); // just for bytes now
    stream_obj_.stream_bytes_.clear();
    header_ = Head{};
    content_.resize(0);
  }

  bool is_empty() const noexcept {
    return stream_obj_.stream_bytes_.is_empty() 
           && header_.total_size == 0
           && content_.size() == 0;
  }

  // loading data
  // ------------
  template<typename T>
  bool is(const char* name) const noexcept {
    const Block* ptr = get_block_ptr(name);
    if (ptr == nullptr) { return false; } 
    else { return XSerial<T>::is_dyn(*ptr); }
  }

template<typename T>
  bool is(const char* name, const T& y) const noexcept {
    const Block* ptr = get_block_ptr(name);
    if (ptr == nullptr) { return false; } 
    else { return XSerial<T>::is_dyn(*ptr, y); }
  }

  template<typename T>
  T getitem(const char* name) const {
    const Block& block = get_block(name);
    stream_->seek(block.pos_, std::ios_base::beg);
    return XSerial<T>::load(block, *stream_);
  }

  // if only size is the same
  template<typename T>
  void getitem(const char* name, T& y) const {
    const Block& block = get_block(name);
    stream_->seek(block.pos_, std::ios_base::beg);
    return XSerial<T>::load(block, *stream_, y);
  }

  // allow to resize()
  template<typename T>
  void getitem(const char* name, T& y, bool flag_fix_size) const {
    const Block& block = get_block(name);
    stream_->seek(block.pos_, std::ios_base::beg);
    return XSerial<T>::load(block, *stream_, y);
  }

  // fields access
  // -------------
  bool has(const std::string& name) const noexcept { return has(name.c_str()); }
  const Block& get_block(const std::string& name) const { return get_block(name.c_str()); }
  
  bool has(const char* name) const noexcept { return get_block_ptr(name) != nullptr; }

  const Block& get_block(const char* name) const {
    const Block* ptr = get_block_ptr(name);
    if(ptr == nullptr) { throw std::out_of_range("xmat::Input::get_block() wrong name"); }
    return *ptr;
  }

  const Block* get_block_ptr(const char* name) const noexcept {
    auto same_name = [name](const Block& block) { return std::strcmp(block.name_.cbegin(), name) == 0; };
    auto it = std::find_if(content_.cbegin(), content_.cend(), same_name);
    return it != content_.cend() ? &(*it) : nullptr;
  }

  // scan format content
  // -------------------
  void scan() { scan_header(); scan_data(); }

  void scan_header() {
    assert(content_.size() == 0);
    header_.load(*stream_);
  }

  void scan_data() {
    assert(header_.total_size != 0);
    assert(stream_->tell() == kHeadSize);
    if (header_.total_size == 0 || stream_->tell() != kHeadSize) {
      throw std::runtime_error("xmat::Input::scan_data. `scan_header` was skipped"); 
    }
    const auto neof = header_.total_size;
    while(stream_->tell() < neof) {
      Block block;
      block.load(*stream_);
      block.pos_ = stream_->tell();
      stream_->seek(block.typesize_ * block.numel_, std::ios_base::cur);
      content_.push_back(block);
    }
  }

 public:
  std::vector<Block> content_{};
};

} // namespace xmat
