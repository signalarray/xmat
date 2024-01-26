#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <complex>
#include <iterator>
#include <memory>
#include <exception>


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
  static_assert(alwaysFalse<T>(), "using not registered data_type");
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

XMAT_ADDTYPE(char,                 "_char");  // char
XMAT_ADDTYPE(std::int8_t,          "ri08");   // int signed
XMAT_ADDTYPE(std::int16_t,         "ri16");
XMAT_ADDTYPE(std::int32_t,         "ri32");
XMAT_ADDTYPE(std::int64_t,         "ri64");
XMAT_ADDTYPE(std::uint8_t,         "ui08");   // int unsigned
XMAT_ADDTYPE(std::uint16_t,        "ui16");
XMAT_ADDTYPE(std::uint32_t,        "ui32");
XMAT_ADDTYPE(std::uint64_t,        "ui64");
XMAT_ADDTYPE(float,                "rf32");   // floating point
XMAT_ADDTYPE(double,               "rf64");
XMAT_ADDTYPE(std::complex<float>,  "cf32");   // complex
XMAT_ADDTYPE(std::complex<double>, "cf64");


// util
template<std::size_t Size>
uint assign(std::array<char, Size>& a, const char* s) {
  const uint n = std::strlen(s);
  if (n > a.size()) {
    throw std::runtime_error(__PRETTY_FUNCTION__);
  }
  std::copy_n(s, n, a.begin());
  return n;
}



// streams
// -------
// StreamBase impl_ <>---------- Stream stream <>--- Output
//            <- StreamFileOut                 <>--- Input
//            <- StreamFileIn
//            <- StreamBytes

enum class Endian : std::uint8_t { little = 0, big = 1, native = 2 };
enum class StreamMode : char { in = 'i', out = 'o', undef = 'x' };

class StreamBase {
 public:
  virtual ~StreamBase() {}

  virtual void close() {}
  virtual uint tell() {return 0;}
  virtual void seek(std::streamoff off, std::ios::seekdir way) {/*std::ios_base::{beg, cur, end}*/}
  virtual bool eof() {return false;}
  virtual uint size() {return 0;}
  virtual bool is_open() const {return false; }

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
  virtual bool eof() override {return os_.eof();}
  virtual uint size() override {return 0;}
  virtual bool is_open() const override {return os_.is_open();}

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
  virtual bool eof() override {return is_.eof();}
  virtual uint size() override {return 0;}
  virtual bool is_open() const override {return is_.is_open();}

  virtual void read__(char* ptr, std::streamsize n) override { is_.read(ptr, n); }

  std::ifstream is_;
};


class StreamBytes : public StreamBase {
 public:
  StreamBytes(StreamMode mode, 
              byte_t* buff, 
              uint buff_size, 
              Endian endian = Endian::native) 
    : is_open_{true}, buff_{buff}, buff_size_{buff_size}, 
      size_(mode == StreamMode::out ? 0 : buff_size) {
    mode_ = mode; 
    endian_ = endian;
  }

  // output stream with dynamic buffer
  StreamBytes(Endian endian = Endian::native)
    : StreamBytes{StreamMode::out, nullptr, 0, endian} { }

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

  virtual bool eof() override {return cursor_ = vec_.size();}
  virtual uint size() override {return vec_.size();}
  virtual bool is_open() const override {return is_open_;}

  virtual void write__(const char* ptr, std::streamsize n) override {
    assert(mode_ == StreamMode::out);
    if (buff_) {
      assert(cursor_ + n <= size_);
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
    std::copy_n(buff_ + cursor_, n, ptr);
    cursor_ += n;
  }

 public: // buffer access
  byte_t* buff() {
    if (buff_) return buff_;
    else return vec_.data();
  }

  const byte_t* buff() const {
    if (buff_) return buff_;
    else return vec_.data();
  }

  uint size() const { return size_; }

  byte_t* begin() {return buff();}
  byte_t* end() {return buff() + size_;}

  const byte_t* cbegin() const {return buff();}
  const byte_t* cend() const {return buff() + size_;}

 public: // private:
  bool is_open_ = false;
  uint cursor_{0};
  uint size_{0};              // used size

  byte_t* buff_ = nullptr;    // fixed buffer
  uint buff_size_{0};         // fixed buffer

  std::vector<byte_t> vec_;   // dynamic buffer
};


// just contains objects to not controll it's deleting
class StreamObj {
 public:
  StreamObj(StreamFileOut&& stream) : stream_file_out_{std::move(stream)}, stream_{&stream_file_out_}, which_{0} {}
  StreamObj(StreamFileIn&& stream) : stream_file_in_{std::move(stream)}, stream_{&stream_file_in_}, which_{1} {}
  StreamObj(StreamBytes&& stream) : stream_bytes_{std::move(stream)}, stream_{&stream_bytes_}, which_{2} {}

  StreamObj(const StreamObj& obj) = delete;

  StreamObj(StreamObj&& obj) 
    : which_{obj.which_}, 
      stream_file_out_(std::move(obj.stream_file_out_)), 
      stream_file_in_(std::move(obj.stream_file_in_)),
      stream_bytes_(std::move(obj.stream_bytes_)) {
    if (which_ == 0) stream_ = &stream_file_out_;
    else if (which_ == 1) stream_ = &stream_file_in_;
    else if (which_ == 2) stream_ = &stream_bytes_;
    else {assert(false);}
  }

  StreamBase* stream() & {return stream_; }

 // private:
  StreamFileOut stream_file_out_;
  StreamFileIn stream_file_in_;
  StreamBytes stream_bytes_;
  const int which_ = -1;
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

 public:
  std::array<char, kMaxBlockNameLen> name_ = {};
  std::array<char, kMaxTypeNameLen> typename_ = {};
  std::array<xint_t, kMaxNDimXMat> shape_ = {0};
  xint_t numel_ = 0;
  std::uint8_t typesize_ = 0;
  std::uint8_t ndim_ = 0;

  const char* ptr_ = nullptr;
  uint pos_ = 0;
};


class Output final {
 public:
  Output(StreamObj&& stream_obj)
    : stream_obj_{std::move(stream_obj)}, stream_{stream_obj_.stream_}
  {
    header_.dump(*stream_);
  }

  ~Output() { close(); }

  Block add(const char* name, Block block) {
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

 public:
  Head header_{};
  StreamObj stream_obj_;
  StreamBase* stream_ = nullptr;
};


class Input final {
 public:
  Input(StreamObj&& stream_obj)
    : stream_obj_{std::move(stream_obj)}, stream_{stream_obj_.stream_}
  {
    if (stream_->mode_ != StreamMode::in) {
      throw std::runtime_error("stream_->mode_ != StreamMode::in");
    }

    scan();
  }

  // Block block(const char* name) {
  // }

 private:
  void scan() {
    header_.load(*stream_);
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
  Head header_{};
  StreamObj stream_obj_;
  StreamBase* stream_ = nullptr;

  std::vector<Block> content_{};
};
} // namespace xmat
