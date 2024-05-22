#pragma once

#ifdef _WIN32
#define XMAT_USE_WINSOCKET

#undef UNICODE
#define WIN32_LEAN_AND_MEAN   // learn.microsoft.com/ru-ru/windows/win32/winsock/complete-client-code
#define NOMINMAX              // stackoverflow.com/questions/22744262
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
// linux
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#include <sys/socket.h>

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

// c-lang
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdint>

// cpp-lang
#include <sstream>
#include <utility>
#include <cmath>
#include <limits>   // for namespace time:
#include <chrono>
#include <thread>   // for this_thread::yield, sleep_for

// project
#include "xutil.hpp"
#include "xdatastream.hpp"


namespace xmat {

using portint_t = unsigned short;
constexpr size_t      k_xsbuf_size = 1024;
constexpr portint_t   k_xsport = 27015;
constexpr const char* k_xsport_str = "27015";

enum class xsstate : int {
  good,       // all is ok
  error,      // socket functions errors
  gaierror,   // ::getaddrinfo error
  fail,       // logic non-system error
  numel
};

constexpr const char* xsstate_str[static_cast<int>(xsstate::numel)] = 
  {"good", "error", "gaierror", "fail"};

// socket functions namespace
namespace impl {

#ifdef XMAT_USE_WINSOCKET
using socket_t      = SOCKET;
using address_len_t = int;
using size_p        = int;  // in ::send(..., len_t len, ...)

constexpr int         k_tcp_send_flags    = 0;
constexpr auto        k_tcp_so_reuseaddr  = SO_REUSEADDR;
constexpr socket_t    k_invalid_socket    = INVALID_SOCKET;
constexpr int         k_socket_error      = SOCKET_ERROR;

#else // unix
using socket_t      = int;
using address_len_t = socklen_t;
using size_p        = size_t;

constexpr int         k_tcp_send_flags    = MSG_NOSIGNAL;
constexpr auto        k_tcp_so_reuseaddr  = SO_REUSEADDR | SO_REUSEPORT;
constexpr socket_t    k_invalid_socket    = -1;
constexpr int         k_socket_error      = -1;
#endif


#ifdef XMAT_USE_WINSOCKET
inline int close(socket_t socket) noexcept { return closesocket(socket); }

inline void set_timeout(socket_t socket, size_t us) { /*TODO*/ }

// param[out] int: k_socket_error if error
inline int set_blocking(socket_t socket, bool block) {
  u_long blocking = block ? 0 : 1;
  return ioctlsocket(socket, static_cast<long>(FIONBIO), &blocking);
}

// error's stuff
// -------------
inline int err_get_code() { return WSAGetLastError(); }

inline std::string err_get_str(int errcode) {
  std::string msg;
  LPSTR buffer = nullptr;
  auto resout = FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    errcode,
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
    //MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    reinterpret_cast<LPTSTR>(&buffer),
    0,
    NULL
  );
  msg = buffer;
  LocalFree(buffer);
  return msg;
}

bool non_blocking_in_progress() {
  auto errcode = err_get_code();
  return errcode == WSAEWOULDBLOCK || errcode == WSAEALREADY;
}

#else
inline int close(socket_t socket) noexcept { return ::close(socket); }

inline void set_timeout(socket_t socket, size_t us) { /*TODO*/ }

// param[out] int: k_socket_error if error
void set_blocking(SocketHandle sock, bool block) {
  int status = fcntl(sock, F_GETFL);
  if (block) {
    return fcntl(sock, F_SETFL, status & ~O_NONBLOCK);
  }
  else {
    return fcntl(sock, F_SETFL, status | O_NONBLOCK);
  }
}

// error's stuff
// -------------
inline int err_get_code() { return errno; }

inline std::string err_get_str(int errcode) {
  return std::string(::strerror(errcode));
}


bool non_blocking_in_progress() {
  auto errcode = err_get_code();
  return errcode == EWOULDBLOCK;
}

#endif

inline sockaddr_in create_address(std::uint32_t address, unsigned short port) {
    sockaddr_in addr{};
    std::memset(&addr, 0, sizeof(addr)); // in case
    addr.sin_addr.s_addr = htonl(address);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    // addr.sin_len = sizeof(addr);   // if MACOS
    return addr;
}

inline socket_t invalid_socket() { return k_invalid_socket; }

inline bool is_invalid_socket(socket_t sock) { return sock == k_invalid_socket; }


inline std::string err_get_str() { return err_get_str(err_get_code()); }

inline void err_print(int errcode) { std::printf(err_get_str().c_str()); }

timeval double2timeval(double timeout) {
  double ipart = 0.0;
  double fpart = std::modf(timeout, &ipart);

  timeval time;
  time.tv_sec  = static_cast<long>(ipart);
  time.tv_usec = static_cast<int>(fpart * 1000000.0);
  return time;
}
} // namespace impl -----

namespace time {
  double  nan() noexcept { return std::numeric_limits<double>::quiet_NaN(); }
  double  inf() noexcept { return std::numeric_limits<double>::infinity(); }
  bool    isnan(double t) noexcept { return std::isnan(t); }
  bool    isinf(double t) noexcept { return !std::isfinite(t); }
  void    sleep(double timeout) noexcept { std::this_thread::sleep_for(std::chrono::duration<double>{timeout}); }
}


class SocketError : public std::runtime_error {
 public:
  SocketError(xsstate state, const char* comment = nullptr, int sys_error_code = 0) 
   : std::runtime_error{make_msg(state, comment, sys_error_code)} { }

  static std::string make_msg(xsstate state, const char* comment = nullptr, int sys_error_code = 0) {
    std::ostringstream oss;
    oss << "xmat::xsstate-code := " << static_cast<unsigned int>(state) << "\n"
        << "xmat::xsstate-name := " << xsstate_str[static_cast<unsigned int>(state)] << "\n";

    if (comment) { 
      oss << "comment: " << comment << "\n\n"; 
    }
    
    if (state == xsstate::error) { // print socket-system error msg
      if (sys_error_code == 0) { 
        sys_error_code = impl::err_get_code(); 
      }
      oss << "state := error. \n"
          << "sys-code: = " << sys_error_code << "\n"
          << "sys-error-msg: " << impl::err_get_str(sys_error_code) << "\n";
    }
    else if (state == xsstate::gaierror) { // getaddrinfo spesific errors
      if (sys_error_code == 0) {
        sys_error_code = impl::err_get_code();
      }
      oss << "state := gaierror. "
          << "code: = " << sys_error_code << "\n"
          << "sys-error-msg: " << gai_strerror(sys_error_code) << "\n";
    }
    else if (state == xsstate::fail) { // print Socket-class-logic error(fail) msg
      oss << "state := fail. ";
    }
    return oss.str();
  }
};


class SocketStartup {
 public:

  size_t counter = 0;

  static SocketStartup* init() {
    static SocketStartup socket;
    socket.counter += 1;  // just for size-effect.
    return &socket;
  }

  SocketStartup(const SocketStartup&) = delete;
  SocketStartup(SocketStartup&&) = delete;
  SocketStartup& operator=(const SocketStartup&) = delete;
  SocketStartup& operator=(SocketStartup&&) = delete;

  ~SocketStartup() {
    std::printf("xmat::SocketStartup::!SocketStartup(): WSACleanup()\n");
#ifdef XMAT_USE_WINSOCKET
    int iResult = WSACleanup();
    if (iResult != 0) {
      // no-throw bucause of it's destructor. just print
#ifndef NDEBUG
      std::printf("xmat::SocketStartup::~SocketStartup()."
                  " WSACleanup failed with error:\nerrno: %d", WSAGetLastError());
#endif
      assert(false);
    }
#endif
  }

 private:
  SocketStartup() {
#ifndef NDEBUG
    std::printf("xmat::SocketStartup::SocketStartup(): WSAStartup(...)\n");
#endif
#ifdef XMAT_USE_WINSOCKET
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if(iResult != 0) { 
      throw SocketError(xsstate::error, "xmat::SocketStartup::SocketStartup(). "
                                        "WSAStartup() failed with error");
    }
#endif
  }
}; // class SocketStartup 


struct IPAddress {
  static IPAddress any() noexcept { return IPAddress{INADDR_ANY}; }
  static IPAddress localhost() noexcept { return IPAddress{INADDR_LOOPBACK}; }
  static IPAddress none() noexcept { return IPAddress{INADDR_NONE}; }

  static IPAddress make(VString address) {
    if (address.empty()) { return none(); }
    if (address == "0.0.0.0") { return any(); }
    const std::uint32_t ip = inet_addr(address.data());
    if (ip != INADDR_NONE) { return IPAddress(ntohl(ip)); }
    return none();
  }
  
  IPAddress() = default;

  explicit IPAddress(std::uint32_t address) : address_{htonl(address)} { }

  IPAddress(std::uint8_t byte0, std::uint8_t byte1, std::uint8_t byte2, std::uint8_t byte3) :
    address_(htonl(static_cast<std::uint32_t>((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3))) { }

  IPAddress(VString address) { address_ = make(address).address_; }

  std::string to_string() const {
      in_addr address{};
      address.s_addr = address_;
      return inet_ntoa(address);
  }

  std::int32_t to_int() const noexcept { return ntohl(address_); }

  bool is_none() { return address_ == INADDR_NONE; }

  static IPAddress localaddress() {
    SocketStartup::init();
    const impl::socket_t sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == impl::invalid_socket()) return none();

    sockaddr_in address = impl::create_address(ntohl(INADDR_LOOPBACK), 9);
    if (::connect(sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
        impl::close(sock);
        return none();
    }
 
    impl::address_len_t size = sizeof(address);
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&address), &size) == -1) {
        impl::close(sock);
        return none();
    } 
    impl::close(sock);
    return IPAddress(ntohl(address.sin_addr.s_addr));
  }

  std::uint32_t address_ = INADDR_NONE;   // stored in network order
};

bool operator==(const IPAddress& lhs, const IPAddress& rhs) { return lhs.address_ == rhs.address_; }

bool operator!=(const IPAddress& lhs, const IPAddress& rhs) {  return !(lhs == rhs); }

std::ostream& operator<<(std::ostream& stream, const IPAddress& address) { 
  return stream << address.to_string();
}


// ------------------------------------------------------------
class Socket {
 public:
  enum class Type { listener, socket, undef, numel };

  virtual ~Socket() { 
    if (!is_valid()) { return; }
    auto resout = impl::close(socket_id_);
#ifndef NDEBUG
    if (resout == impl::k_socket_error) {
      std::printf("xmat::Socket::destructor(): "
                  "impl::close(socket_id_) -> != 0\n message: \n");
      std::printf(impl::err_get_str().c_str());
    }
#endif
  }

  Socket(const Socket&) = delete;

  Socket& operator=(const Socket&) = delete;

  Socket(Socket&& other) noexcept { swap(*this, other); }
  
  Socket& operator=(Socket&& other) noexcept {
    Socket tmp{std::move(other)};
    swap(*this, tmp);
    return *this;
  }

  friend void swap(Socket& lhs, Socket& rhs) noexcept {
    std::swap(lhs.socket_id_,       rhs.socket_id_);
    std::swap(lhs.is_blocking_,     rhs.is_blocking_);
    std::swap(lhs.is_open_,         rhs.is_open_);
    std::swap(lhs.state_,           rhs.state_);
    std::swap(lhs.flag_exceptions_, rhs.flag_exceptions_);
    std::swap(lhs.type_,            rhs.type_);
  }

  bool operator==(const Socket& other) { return socket_id_ == other.socket_id_; }

  bool operator<(const Socket& other) { return socket_id_ < other.socket_id_; }


  bool blocking() const noexcept { return is_blocking_; }

  void blocking(bool block) {
    assert(is_valid() && "must be a valid value socket");
    if (impl::set_blocking(socket_id_, block) == impl::k_socket_error) {
      return handle_error(xsstate::error, "xmat::Socket::blocking(bool). impl::set_blocking failed");
    }
    is_blocking_ = block;
  }

  // state and error handling
  // ------------------------
  bool is_valid() const noexcept { return !impl::is_invalid_socket(socket_id_); }

  bool is_good() const noexcept { return state_ == xsstate::good; }

  xsstate state() const noexcept { return state_; }

  // if val := true, throw exception if eny error
  // if val := false, set status
  void exceptions(bool val) noexcept { flag_exceptions_ = val; }

  bool exceptions() const noexcept { return flag_exceptions_; }

  // true if:
  // if Listener: listen was called
  // if Socket::client: connect was called
  // if Socket::connection: accept was called for it
  // false if:
  // close was called
  bool is_open() const noexcept { assert(is_valid() || (!is_valid() && !is_open_)); return is_open_; }

  Type type() const noexcept { return type_; }

  impl::socket_t handle() const noexcept { return socket_id_; }


 protected:
  // See: stackoverflow.com/questions/14038589
  void handle_error(xsstate state, const char* comment = nullptr) const {
    assert(state != xsstate::good && "state isn't supposed to be an `ok`");
    state_ = state;
    if (exceptions()) { throw SocketError(state, comment); }
  }

 protected:
  Socket(Type type = Type::undef) : type_{type} { SocketStartup::init(); }

  void create() {
    assert(!is_valid() && "xmat::Socket::create(...)"
                          "current state must be invalid. call `close` before");
    const impl::socket_t sock = ::socket(PF_INET, SOCK_STREAM, 0);
    if (impl::is_invalid_socket(sock)) {
      return handle_error(xsstate::error, "Socket::create(). returns invalid socket");
    }
    setoptions(sock);
  }

  void setoptions(impl::socket_t sock) {
    assert(!is_valid() && "xmat::Socket::settings(...)"
                          "current state must be non-valid");
    socket_id_ = sock;
    const int yes = 1;
    if(::setsockopt(socket_id_,
                    SOL_SOCKET,
                    impl::k_tcp_so_reuseaddr,
                    reinterpret_cast<const char*>(&yes), 
                    sizeof(yes)) == impl::k_socket_error) {
      return handle_error(xsstate::error, "xmat::Socket::setoptions(). "
                                          "setsockopt(.., impl::tcp_so_reuseaddr, ..)");
    }
    if (::setsockopt(socket_id_,
                     IPPROTO_TCP,
                     TCP_NODELAY,
                     reinterpret_cast<const char*>(&yes), 
                     sizeof(yes)) == impl::k_socket_error) {
      return handle_error(xsstate::error, "xmat::Socket::setoptions(). "
                                          "setsockopt(.., TCP_NODELAY, ..)");
    }
  }

 public:
  void close() {
    is_open_ = false;
    if (!is_valid()) { return; }
    if(impl::close(socket_id_) == impl::k_socket_error ) {
      return handle_error(xsstate::error);
    }
    socket_id_ = impl::k_invalid_socket;
  }

 private:
  void move(const Socket& other) {
    // just instead walking trougth all members:
    std::memcpy(reinterpret_cast<void*>(this), reinterpret_cast<const void*>(&other), sizeof(other));
  }

 public:
  impl::socket_t socket_id_ = impl::k_invalid_socket;
  bool is_blocking_         = true;
  bool is_open_             = false;
  mutable xsstate state_    = xsstate::good;
  bool flag_exceptions_     = true;
  Type type_                = Type::undef;
};


// ------------------------------------------------------------
class TCPSocket : public Socket {
 public:
  friend class TCPListener;

  TCPSocket() : Socket{Socket::Type::socket} {}

  // param[in] timeout:
  //  if timeout == 0.0: finish immediately
  //  if time::isinf(timeout): never timeout, and will wait until the first file descriptor is ready
  //                           the same: ::select(.., timeout := NULL)
  void connect(IPAddress address, unsigned int port, double timeout) {
    assert(is_good());
    if (is_valid()) {
      return handle_error(xsstate::fail, "xmat::TCPSocket::connect(..). Listener is already used.");
    }
    create();
    if(!is_good()) { return; }

    sockaddr_in address_v = impl::create_address(address.to_int(), port);

    if (time::isinf(timeout)) {
      if (::connect(socket_id_, 
                    reinterpret_cast<sockaddr*>(&address_v), 
                    sizeof(address_v)) == impl::k_socket_error) { 
        return handle_error(xsstate::error, "TCPSocket::connect(). connect failed");
      }
      is_open_ = true;
      return;
    }
    // end for infinite-timeout case

    // finite-timeout case
    // ---------------------------
    bool blocking_tmp = blocking();
    if (blocking_tmp) blocking(false);

    auto out2 = ::connect(socket_id_, reinterpret_cast<sockaddr*>(&address_v), sizeof(address_v));
    if( out2 != impl::k_socket_error) { // if error: it can be: non_blocking_in_progress()
      blocking(blocking_tmp);
      is_open_ = true;
      return;
    }
    else if (!impl::non_blocking_in_progress()) {
      return handle_error(xsstate::error, "xmat::TCPSocket::connect().\n"
                                          "finite-timeout ::connect(...) fail");
    }

    if (!blocking_tmp) {
      // it's ok for non-blocking mode. call connect untill is_open() == true
      return;
    }
    // end for: non-blocking AND finite-timeout

    // finite-timeout AND blocking
    // ---------------------------
    fd_set selector;
    FD_ZERO(&selector);
    FD_SET(socket_id_, &selector);

    timeval time = impl::double2timeval(timeout);
    auto count = ::select(static_cast<int>(socket_id_+1), NULL, &selector, NULL, &time);
    blocking(true);

    if (count > 0) {
      if (remoteaddress() != IPAddress::none()) {
        is_open_ = true;
      }
      else {
        return handle_error(xsstate::error, "xmat::TCPSocket::connect().\n"
                                     "non-zero-timeout AND blocking. ::select() \n"
                                     "Connection refused");
      }
    }
    else {
      return handle_error(xsstate::error, "xmat::TCPSocket::connect().\n"
                                   "non-zero-timeout AND blocking. ::select \n"
                                   "Failed to connect before timeout is over");
    }
  }

  // just in cases
  void connect__(const char* address, const char* port) {
    assert(is_good());
    close();

    addrinfo *result = NULL, *ptr = NULL, hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if(::getaddrinfo(address, port, &hints, &result) != 0) {
      return handle_error(xsstate::gaierror, "TCPSocket::connect__(). getaddrinfo(...) fail");
    }

    for(ptr=result; ptr != NULL ; ptr=ptr->ai_next) {
      create();
      if(::connect(socket_id_, ptr->ai_addr, (int)ptr->ai_addrlen) == impl::k_socket_error) {
        close();
      }
      break;
    }
    ::freeaddrinfo(result);
    if(!is_valid()) {
      return handle_error(xsstate::fail, "TCPSocket::connect__() unsuccess");
    }
    is_open_ = true;
  }

  size_t send(const void* buf, size_t len) {
    assert(is_valid() && is_good());

    size_t sent = 0;
    if (!buf || !len) {
      handle_error(xsstate::fail, "TCPSocket::send(...). !buf || !len");
      return sent;
    }

    for (sent = 0; sent < len;) {
      auto result = ::send(socket_id_,
                           static_cast<const char*>(buf) + sent,
                           static_cast<impl::size_p>(len - sent),
                           impl::k_tcp_send_flags);
      if (result < 0) {
        handle_error(xsstate::error, "TCPSocket::send() failed");
      }
      sent += static_cast<size_t>(result);
    }
    return sent;
  }


  void sendall(const void* buf, size_t len, double timeout) {
    assert(is_valid() && is_good());

    if (!buf || !len) {
      return handle_error(xsstate::fail, "TCPSocket::send(...). !buf || !len");
    }

    std::chrono::duration<double> dt{timeout};
    auto tend = std::chrono::system_clock::now() + dt;

    size_t total = 0, left = len;
    while(total < len && std::chrono::system_clock::now() < tend) {
      auto sent = ::send(socket_id_,
                         static_cast<const char*>(buf) + total,
                         static_cast<impl::size_p>(left),
                         impl::k_tcp_send_flags);
      if (sent == impl::k_socket_error) {
        return handle_error(xsstate::error, "TCPSocket::send() failed");
      }

      assert(sent >= 0);
      total += static_cast<size_t>(sent);
      left -= static_cast<size_t>(sent);
      std::this_thread::yield();  //??
    }
    if (total != len) {
      return handle_error(xsstate::fail, "TCPSocket::sendall() failed. total != len");
    }
  }


  size_t recv(void* buf, size_t len) {
    assert(is_valid() && is_good());
    size_t sent = 0;
    if (!buf || !len) {
      handle_error(xsstate::fail, "TCPSocket::send(...). !buf || !len");
      return sent;
    }

    auto received = ::recv(socket_id_, static_cast<char*>(buf), 
                           static_cast<impl::size_p>(len), 
                           impl::k_tcp_send_flags);

    if (received == impl::k_socket_error && !impl::non_blocking_in_progress()) {
      handle_error(xsstate::error, "xmat::TCPSocket::recvall(...).\n");
    } 
    else if ( received == 0 ) {
      handle_error(xsstate::error, "xmat::TCPSocket::recvall(...).\n"
                                   "connection has been closed");
    }
    return static_cast<size_t>(received > 0 ? received : 0);
  }


  // param[in] timeout - sec
  void recvall(void* buf, size_t len, double timeout) {
    assert(is_valid() && is_good());
    if (!buf || !len) {
      return handle_error(xsstate::fail, "TCPSocket::send(...). !buf || !buf");
    }

    std::chrono::duration<double> tt{timeout};
    auto tend = std::chrono::system_clock::now() + tt;

    size_t total = 0, left = len;
    while(total < len && std::chrono::system_clock::now() < tend) {
      auto received = recv(static_cast<char*>(buf) + total, left);
      if (!is_good()) {
        return;
      }
      assert(received >= 0);
      total += static_cast<size_t>(received);
      left -= static_cast<size_t>(received);
      std::this_thread::yield();  //??
    }
    if (total != len) {
      return handle_error(xsstate::fail, "TCPSocket::recvall() failed. total != len");
    }
  }


  // xmat::BugIn, xmat::BugOut send - recv
  // -------------------------------------
  /// \tparam MemSourceT one of: 
  ///         bbuf_memsource_default, AllocatorMSGlobal<char>, AllocatorMSRef<char>
  template<typename MemSourceT, Endian endian_tag>
  void send(const OMapStream_<ODStream_<OBBuf_<MemSourceT>, endian_tag>>& xout, 
            double timeout) 
  {
    if(xout.buf().is_open()) {
      return handle_error(xsstate::fail,
        "TCPSocket::send(BugOut_ xout). xout.buf().is_open() := false. xout must be closed");
    }
    if(xout.head().total_size == 0) {
      return handle_error(xsstate::fail,
        "TCPSocket::send(BugOut_ xout). xout.head().total_size := 0. xout must be not empty");
    }
    sendall(xout.buf().data(), xout.buf().size(), timeout);
  }


  template<typename MemSourceT, Endian endian_tag>
  void recv(IMapStream_<IDStream_<IBBuf_<MemSourceT>, endian_tag>>& xin, 
            double timeout)
  {
    if(xin.buf().size() != 0) {
      return handle_error(xsstate::fail,
        "TCPSocket::recv(BugIn_ xin). xin.buf().size() :!= 0. xout must be empty");
    }
    
    // -- try read Head
    char* ptr_head = xin.buf().push_reserve(k_head_size);
    recvall(static_cast<void*>(ptr_head), k_head_size, timeout);
    if (!is_good()){
      return handle_error(xsstate::fail, 
        "TCPSocket::recv(BugIn_ xin). recvall(header) failed");
    }
    xin.scan_head();

    // -- read Data. if fail -> throw Exception
    assert(xin.head().total_size >= k_head_size);
    size_t data_size = xin.head().total_size - k_head_size;
    char* ptr_data = xin.buf().push_reserve(data_size);
    recvall(static_cast<void*>(ptr_data), data_size, timeout);
    if(!is_good()) {
      return handle_error(xsstate::fail, 
        "TCPSocket::recv(BugIn_ xin). recvall(data) failed");
    }
    xin.buf().push_all();
  }


  // ------------------------------
  IPAddress remoteaddress() const {
    assert(is_valid());
    auto out = IPAddress::none();
    if (!is_valid()) {
      return out;
    }
    sockaddr_in address{};
    impl::address_len_t size = sizeof(address);
    if (getpeername(socket_id_, reinterpret_cast<sockaddr*>(&address), &size) == 0) {
      out = IPAddress(ntohl(address.sin_addr.s_addr));
    }
    return out;
  }


  portint_t remoteport() const { 
    assert(is_valid());
    portint_t out = 0;
    if (!is_valid()) {
      return out;
    }
    sockaddr_in address{};
    impl::address_len_t size = sizeof(address);
    if (getpeername(socket_id_, reinterpret_cast<sockaddr*>(&address), &size) == 0) {
      out = ntohs(address.sin_port);
    }
    return out;
  }


  portint_t localport() const { 
    assert(is_valid());
    portint_t out = 0;
    if (!is_valid()) {
      return out;
    }
    sockaddr_in address{};
    impl::address_len_t size = sizeof(address);
    if (getsockname(socket_id_, reinterpret_cast<sockaddr*>(&address), &size) == 0) {
      out = ntohs(address.sin_port);
    }
    return out;
  }
};


// ------------------------------------------------------------
class TCPListener : public Socket {
 public:
  
  TCPListener() : Socket{Socket::Type::listener} {}

  // param[in] nconn - number of connections for listenning
  void listen(unsigned short port, IPAddress address = IPAddress{0, 0, 0, 0}, int nconn = SOMAXCONN) {
    if (is_valid()) {
      return handle_error(xsstate::fail, "xmat::TCPListener::listen(). Listener is already used.");
    }

    create();
    assert(is_good() && is_valid());
    
    if (address.is_none()) {
      return handle_error(xsstate::fail, "xmat::TCPListner::listen. `address` := `none`");
    }

    sockaddr_in addr = impl::create_address(address.to_int(), port);
    if (::bind(socket_id_,
               reinterpret_cast<sockaddr*>(&addr),
               sizeof(addr)) == impl::k_socket_error)
    {
      return handle_error(xsstate::error, "xmat::TCPListener::listen(). `bind` failed");
    }

    if (::listen(socket_id_, nconn) == impl::k_socket_error) {
      return handle_error(xsstate::error, "xmat::TCPListener::listen(). `listen` failed");
    }
    is_open_ = true;
  }

  void accept(TCPSocket& socket) {
    assert(is_valid() && "TCPListener is invalid");

    sockaddr_in address{};
    impl::address_len_t length = sizeof(address);
    const impl::socket_t remote = ::accept(socket_id_, reinterpret_cast<sockaddr*>(&address), &length);
    if (impl::is_invalid_socket(remote)) {
      return handle_error(xsstate::error, "TCPListener::accept(). `accept` failed");
    }
    socket.close();
    socket.setoptions(remote);
  }

  portint_t localport() const {
    assert(is_good() && is_valid());
    unsigned int out = 0;
    sockaddr_in address{};
    impl::address_len_t size = sizeof(address);
    if (getsockname(socket_id_, reinterpret_cast<sockaddr*>(&address), &size) == impl::k_socket_error) {
      handle_error(xsstate::error, "TCPListener::localport(). `getsockname` failed");
    } else {
      out = ntohs(address.sin_port);
    }
    return out;
  }
};


class Selector {
 public:
  virtual ~Selector() { }

  Selector() { clear(); }

  void add(Socket& socket) {
    assert(socket.is_valid());
    if (!socket.is_valid()) {
      return;
    }
#ifdef XMAT_USE_WINSOCKET
    if(size_ >= FD_SETSIZE) {
      return handle_error(xsstate::fail, "xmat::Selector::add(..). size >= FD_SETSIZE.\n"
                                          "The socket can't be added to the selector because the "
                                          "selector is full. This is a limitation of your operating "
                                          "system's FD_SETSIZE setting. See: FD_SETSIZE comment");
    }
    if (FD_ISSET(socket.handle(), &all_)) { // why isn't in crossplatform code?
      return;
    }
#else
    if (socket.handle() >= FD_SETSIZE) {
        return handle_error(xsstate::fail, "xmat::Selector::add(..). socket.handle() >= FD_SETSIZE\n"
                                           "The socket can't be added to the selector because its "
                                           "ID is too high. This is a limitation of your operating "
                                           "system's FD_SETSIZE setting. See: FD_SETSIZE comment");
    }
#endif
    ++size_;
    max_socket_id_ = std::max(max_socket_id_, static_cast<int>(socket.handle()));
    FD_SET(socket.handle(), &all_);
  }


  void remove(Socket& socket) {
    assert(socket.is_valid());
    if (!socket.is_valid()) {
      return;
    }
#ifdef XMAT_USE_WINSOCKET
    if (!FD_ISSET(socket.handle(), &all_)) {
      return;
    }
    --size_;
#else
    if (socket.handle() >= FD_SETSIZE) {
      return;
    }
#endif
    FD_CLR(socket.handle(), &all_);
    FD_CLR(socket.handle(), &ready_);
  }


  void clear() {
    FD_ZERO(&all_);
    FD_ZERO(&ready_);

    max_socket_id_ = 0;
    size_ = 0;
  }

  // param[in] timeout:
  //  if timeout == 0.0: finish immediately
  //  if time::isinf(timeout): never timeout, and will wait until the first file descriptor is ready
  //                           the same: ::select(.., timeout := NULL)
  int wait(double timeout) {
    timeval time = impl::double2timeval(timeout);
    ready_ = all_;

    // Wait until one of the sockets is ready for reading, or timeout is reached
    // The first parameter is ignored on Windows
    int count = ::select(max_socket_id_ + 1, &ready_, NULL, NULL, time::isinf(timeout) ? NULL : &time);
    return count;
  }


  bool is_ready(Socket& socket) const {
    assert(socket.is_valid());
    if (!socket.is_valid()) { 
      return false;
    }
#ifndef XMAT_USE_WINSOCKET
    if (handle >= FD_SETSIZE) {
      return false;
    }
#endif
    return FD_ISSET(socket.handle(), &ready_) != 0;
  }


  bool exceptions() const noexcept { return flag_exceptions_; }

  void exceptions(bool value) noexcept { flag_exceptions_ = value; }

  void handle_error(xsstate state, const char* comment = nullptr) const {
    assert(state != xsstate::good && "state isn't supposed to be an `ok`");
    state_ = state;
    if (exceptions()) { throw SocketError(state, comment); }
  }

 private:
  fd_set all_;
  fd_set ready_;
  int max_socket_id_ = 0; // for select(nfds, ...) arg. used for only UNIX.
                          // in Windows: only for compatibility with Berkeley sockets.
  int size_ = 0;

  mutable xsstate state_ = xsstate::good;
  bool flag_exceptions_  = true;
};


template<size_t Nsock = 8, size_t Nlis = 2>
class TCP_ {
 public:

  TCP_() {}

  TCPListener* listener(unsigned short port, IPAddress address = IPAddress{0, 0, 0, 0}, int nconn = SOMAXCONN) {
    if (n_lis_ >= max_nlis()) {
      throw SocketError(xsstate::fail, "xmat::TCPGroup::listener(...). out_of_range");
    }
    TCPListener item{};
    item.exceptions(exceptions());
    item.listen(port, address, nconn);
    if (!item.is_good()) {
      return nullptr;
    }
    selector_.add(item);
    listeners_[n_lis_++] = std::move(item);
    return &listeners_[n_lis_-1];
  }


  TCPSocket* client(IPAddress address, unsigned int port, double timeout) {
    if (n_sock_ >= max_nsock()) {
      throw SocketError(xsstate::fail, "xmat::TCPGroup::socket(...). out_of_range");
    }
    TCPSocket item{};
    item.exceptions(exceptions());
    item.connect(address, port);
    if (!item.is_good()) {
      return nullptr;
    }
    selector_.add(item);
    sockets_[n_sock_++] = std::move(item);
    return &sockets_[n_sock_-1];
  }


  TCPSocket* accept(TCPListener* listener) {
    if (n_sock_ >= max_nsock()) {
      throw SocketError(xsstate::fail, "xmat::TCPGroup::accept(...). out_of_range");
    }
    if (!contains(listener)) {
      throw SocketError(xsstate::fail, "xmat::TCPGroup::accept(listener). alien listener");
    }
    if (!is_ready(listener)) {
      throw SocketError(xsstate::fail, "xmat::TCPGroup::accept(listener). listener isn't ready");
    }

    TCPSocket remote{};
    listener->accept(remote);
    if (!listener->is_good()) {
      return nullptr;
    }
    selector_.add(remote);
    sockets_[n_sock_++] = std::move(remote);
    return &sockets_[n_sock_-1];
  }

  // selector-methods
  // ----------------
  int wait(double timeout) { return selector_.wait(timeout); }

  bool is_ready(Socket* socket) { assert(contains(socket)); return selector_.is_ready(*socket); }

  // remove ----
  void remove(Socket* socket) {
    assert(contains(socket));
    if (socket->type() == Socket::Type::undef) {
      throw SocketError(xsstate::fail, 
                        "xmat::TCPGroup::remove(..). socket.type() == Socket::Type::undef");
    }
    if (socket->type() == Socket::Type::listener) {
      support_remove_element(listeners_.begin(), listeners_.end(), socket, selector_);
      --n_lis_;
    } 
    else {
      support_remove_element(sockets_.begin(), sockets_.end(), socket, selector_);
      --n_sock_;
    }
  }

  // support
  bool contains(Socket* socket) {
    if (socket->type() == Socket::Type::listener) {
      auto it = std::find_if(listeners_.begin(), listeners_.end(),
                             [tgt = socket] (const TCPListener& item) { return &item == tgt; });
      return it != listeners_.begin();
    } 
    else {
      auto it = std::find_if(sockets_.begin(), sockets_.end(),
                             [tgt = socket] (const TCPSocket& item) { return &item == tgt; });
      return it != sockets_.begin();
    }
  }

 private:
  template<typename Iter, typename T>
  friend void support_remove_element(Iter begin, Iter end, T* element, Selector& sel) {
    // look up by pointer
    auto it = std::find_if(begin, end, 
                           [tgt = element] (const T& item) { return &item == tgt; });
    if (it == end) {
      throw SocketError(xsstate::fail, 
                        "xmat::TCPGroup::remove(sock). socket isn`t in this group");
    }
    sel.remove(*element);
    std::move(it+1, end, it);
  }

 public:
  bool exceptions() const noexcept { return selector_.exceptions(); }

  void exceptions(bool value) noexcept { selector_.exceptions(value); }

  TCPListener& ilistener(size_t idx) { return listeners_[idx]; }

  TCPSocket& isocket(size_t idx) { return sockets_[idx]; }

  size_t nlis() const noexcept { return n_lis_; }

  size_t nsock() const noexcept { return n_sock_; }

  static size_t max_nlis() noexcept { return Nlis; }

  static size_t max_nsock() noexcept { return Nsock; }

 private:
  std::array<TCPListener, Nlis> listeners_;
  std::array<TCPSocket, Nsock> sockets_;
  size_t n_lis_ = 0;
  size_t n_sock_ = 0;

  Selector selector_;
};

} // namespace xmat


/* 
See for help:
------------

## Windows Sockets 2:
https://learn.microsoft.com/ru-ru/windows/win32/winsock/windows-sockets-start-page-2

## Beej's Guide to Network Programming:
https://beej.us/guide/bgnet/html/#platform-and-compiler

## timeout:
https://stackoverflow.com/questions/4181784/how-to-set-socket-timeout-in-c-when-making-multiple-connections
https://stackoverflow.com/questions/2597608/c-socket-connection-timeout
https://learn.microsoft.com/ru-ru/windows/win32/api/winsock/nf-winsock-setsockopt
*/
