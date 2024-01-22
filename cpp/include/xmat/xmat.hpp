#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <exception>


namespace xmat {

using uint = std::size_t;
using intx_t = std::size_t;
using byte_t = char;

// constants
const std::uint8_t kUfixSize = sizeof(intx_t);
static const std::uint8_t kXFormatSignatureSize = 4;

// types supported

// streams
// -------
// StreamBase impl_ <>---------- Stream stream <>--- Save
//            <- StreamFileOut                 <>--- Load
//            <- StreamFileIn
//            <- StreamBytes

enum class Endian : std::uint8_t { little = 0, big = 1, native = 2 };
enum class StreamMode : char { in = 'i', out = 'o', undef = 'x' };

class StreamBase {
 public:
  virtual ~StreamBase() = default;

  virtual uint close() {return 0;}
  virtual uint tell() {return 0;}
  virtual void seek(std::streamoff off, std::ios::seekdir way) {}
  virtual bool eof() {return false;}
  virtual uint size() {return 0;}

  virtual void write(const char* ptr, std::streamsize n) {std::runtime_error("can't write");}
  virtual void read(char* ptr, std::streamsize n)  {std::runtime_error("can't read");}

  Endian endian_{Endian::native};        // not implemented
  StreamMode mode_{StreamMode::undef};
};


class StreamFileOut : public StreamBase {
 public:
  StreamFileOut(const char* filename) : os_(filename, std::ios::binary) { assert(os_.is_open()); }
  StreamFileOut(const std::string& filename) : StreamFileOut(filename.c_str()) {}

  virtual uint close() override {os_.close(); return 0; }
  virtual uint tell() override {return os_.tellp();}
  virtual void seek(std::streamoff off, std::ios_base::seekdir way) override {os_.seekp(off, way);}
  virtual bool eof() override {return os_.eof();}
  virtual uint size() override {return 0;}

  virtual void write(const char* ptr, std::streamsize n) override { os_.write(ptr, n); }

  std::ofstream os_;
};


class StreamFileIn : public StreamBase {
 public:
  StreamFileIn(const char* filename) : is_(filename, std::ios::binary) {assert(is_.is_open());}
  StreamFileIn(const std::string& filename) : StreamFileIn(filename.c_str()) {}

  virtual uint close() override {is_.close(); return 0; }
  virtual uint tell() override {return is_.tellg();}
  virtual void seek(std::streamoff off, std::ios_base::seekdir way) override {is_.seekg(off, way);}
  virtual bool eof() override {return is_.eof();}
  virtual uint size() override {return 0;}

  virtual void read(char* ptr, std::streamsize n) override { is_.read(ptr, n); }

  std::ifstream is_;
};


class StreamBytes : public StreamBase {
 public:
  StreamBytes(StreamMode mode, 
              byte_t* buff, 
              uint buff_size, 
              Endian endian = Endian::native) 
    : buff_(buff), buff_size_(buff_size), 
      size_(mode == StreamMode::out ? 0 : buff_size) {
    mode_ = mode; 
    endian_ = endian;
  }

  // output stream with dynamic buffer
  StreamBytes(Endian endian = Endian::native)
    : StreamBytes{StreamMode::out, nullptr, 0, endian} { }

  virtual uint close() override { return 0; }
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

  virtual void write(const char* ptr, std::streamsize n) override {
    assert(mode_ == StreamMode::out);
    if (buff_) {
      assert(cursor_ + n < size_);
      std::copy_n(ptr, n, buff_ + cursor_);
    } else {
      assert(cursor_ <= vec_.size());
      vec_.insert(std::next(vec_.begin(), cursor_), ptr, ptr + n);
    }
    cursor_ += n;
    size_ = std::max(size_, cursor_);
  }

  virtual void read(char* ptr, std::streamsize n) override {
    assert(mode_ == StreamMode::in);
    assert(cursor_ + n < size_);
    std::copy_n(buff_ + cursor_, n, ptr);
    cursor_ += n;
  }

  // own methods
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

  // private:
  uint cursor_{0};
  uint size_{0};              // used size

  byte_t* buff_ = nullptr;    // fixed buffer
  uint buff_size_{0};         // fixed buffer

  std::vector<byte_t> vec_;   // dynamic buffer
};


class Stream {
 public:
  Stream(StreamBase* stream_base) : impl_(stream_base) {}


 private:
  StreamBase* impl_ = nullptr;
};


// format
// ------
class Header {
 public:
  static constexpr char kFormatSignature[kXFormatSignatureSize] = "XYZ";
  static const std::uint8_t kMaxBlockNameLen = 32;
  static const std::uint8_t kMaxTypeNameLen = 8;
  static const std::uint8_t kMaxNDimXMat = 8;
  static const std::uint8_t kSize = {8 + 2*kXFormatSignatureSize + 4}; // size in bytes

};


class Block {
 public:
  static constexpr char kBlockBeginSignature[kXFormatSignatureSize] = "<#>";
  static constexpr char kBlockEndSignature[kXFormatSignatureSize] = ">#<";

  static const std::uint8_t kSize = {Header::kMaxBlockNameLen + Header::kMaxTypeNameLen + 
    (1 + 1 + kMaxNDimIndex + 1) * kUfixSize + 2 * kXFormatSignatureSize};

};


// storage
// -------
class Save {
 public:
  // Save()

};

class Load {
 public:
  // Load() 

}; 


} // namespace xmat
