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


// project
#include "xutil.hpp"
#include "xstream.hpp"


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
constexpr int         k_socket_error      = 1;
#endif


#ifdef XMAT_USE_WINSOCKET
inline int close(socket_t socket) noexcept { return closesocket(socket); }

inline void set_timeout(socket_t socket, size_t us) { /*TODO*/ }

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

#else
inline int close(socket_t socket) noexcept { return ::close(socket); }

inline void set_timeout(socket_t socket, size_t us) { /*TODO*/ }

// error's stuff
// -------------
inline int err_get_code() { return errno; }

inline std::string err_get_str(int errcode) {
  return std::string(::strerror(errcode));
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
} // namespace impl -----


class SocketError : public std::runtime_error {
 public:
  SocketError(xsstate state, const char* comment = nullptr, int sys_error_code = 0) 
   : std::runtime_error{make_msg(state, comment, sys_error_code)} { }

  static std::string make_msg(xsstate state, const char* comment = nullptr, int sys_error_code = 0) {
    std::ostringstream oss;
    oss << "xmat::xsstate-code := " << static_cast<unsigned int>(state) << "\n"
        << "xmat::xsstate-name := " << xsstate_str[static_cast<unsigned int>(state)] << "\n";

    if (comment) { 
      oss << "comment: " << comment << "\n"; 
    }
    
    if (state == xsstate::error) { // print socket-system error msg
      if (sys_error_code == 0) { 
        sys_error_code = impl::err_get_code(); 
      }
      oss << "state := error. \n"
          << "sys-code: =" << sys_error_code << "\n"
          << "sys-error-msg: " << impl::err_get_str(sys_error_code) << "\n";
    }
    else if (state == xsstate::gaierror) { // getaddrinfo spesific errors
      if (sys_error_code == 0) {
        sys_error_code = impl::err_get_code();
      }
      oss << "state := gaierror. "
          << "code: =" << sys_error_code << "\n"
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

bool operator==(const IPAddress& lhs, const IPAddress& rhs) {
    return lhs.address_ < rhs.address_;
}

std::ostream& operator<<(std::ostream& stream, const IPAddress& address) { 
  return stream << address.to_string();
}


// ------------------------------------------------------------
class SocketBase {
 public:
  virtual ~SocketBase() { 
    if (!is_valid()) { return; }
    auto resout = impl::close(socket_id_);
#ifndef NDEBUG
    if (resout == impl::k_socket_error) {
      std::printf("xmat::SocketBase::destructor(): "
                  "impl::close(socket_id_) -> != 0\n message: \n");
      std::printf(impl::err_get_str().c_str());
    }
#endif
  }

  SocketBase(const SocketBase&) = delete;

  SocketBase& operator=(const SocketBase&) = delete;

  SocketBase(SocketBase&& other) noexcept { std::swap(*this, other); }
  
  SocketBase& operator=(SocketBase&& other) {
    SocketBase tmp(std::move(other));
    std::swap(*this, tmp);
    return *this;
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

 protected:
  // See: stackoverflow.com/questions/14038589
  void handle_error(xsstate state, const char* comment = nullptr) const {
    assert(state != xsstate::good && "state isn't supposed to be an `ok`");
    state_ = state;
    if (exceptions()) { throw SocketError(state, comment); }
  }

 protected:
  SocketBase() { SocketStartup::init(); }

  void create() {
    assert(!is_valid() && "xmat::Socket::create(...)"
                          "current state must be invalid. call `close` before");
    const impl::socket_t sock = ::socket(PF_INET, SOCK_STREAM, 0);
    if (impl::is_invalid_socket(sock)) {
      return handle_error(xsstate::error, "SocketBase::create(). returns invalid socket");
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
      return handle_error(xsstate::error, "xmat::SocketBase::setoptions(). "
                                          "setsockopt(.., impl::tcp_so_reuseaddr, ..)");
    }
    if (::setsockopt(socket_id_,
                     IPPROTO_TCP,
                     TCP_NODELAY,
                     reinterpret_cast<const char*>(&yes), 
                     sizeof(yes)) == impl::k_socket_error) {
      return handle_error(xsstate::error, "xmat::SocketBase::setoptions(). "
                                          "setsockopt(.., TCP_NODELAY, ..)");
    }
  }

  void close() {
    if (!is_valid()) { return; }
    if(impl::close(socket_id_) == impl::k_socket_error ) {
      return handle_error(xsstate::error);
    }
    socket_id_ = impl::k_invalid_socket;
  }

 public:
  impl::socket_t socket_id_ = impl::k_invalid_socket;
  mutable xsstate state_ = xsstate::good;
  bool flag_exceptions_ = true;
};


// ------------------------------------------------------------
class TCPSocket : public SocketBase {
 public:
  enum class Mode { client, connection, undef };

  friend class TCPListener;

  TCPSocket() = default;

  void connect(IPAddress address, unsigned int port) {
    assert(is_good());
    close();
    create();
    if(!is_good()) { return; }

    sockaddr_in address_v = impl::create_address(address.to_int(), port);
    if (::connect(socket_id_, 
                  reinterpret_cast<sockaddr*>(&address_v), 
                  sizeof(address_v)) == impl::k_socket_error) {
      return handle_error(xsstate::error, "TCPSocket::connect(). connect failed");
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
  }

  size_t send(const void* buf, size_t len) {
    assert(is_valid() && is_good());

    size_t sent = 0;
    if (!buf || !len) {
      handle_error(xsstate::fail, "TCPSocket::send(...). !data || !size");
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


  size_t recv(void* buf, size_t len) {
    assert(is_valid() && is_good());

    size_t sent = 0;
    if (!buf || !len) {
      handle_error(xsstate::fail, "TCPSocket::send(...). !data || !size");
      return sent;
    }

    auto received = ::recv(socket_id_, 
                           static_cast<char*>(buf), 
                           static_cast<impl::size_p>(len),
                           impl::k_tcp_send_flags);
    if (received <= 0) {
      received = 0;
      handle_error(xsstate::error, "TCPSocket::recv() failed");
    }
    return received;
  }

  // xmat::BugIn, xmat::BugOut send - recv
  // -------------------------------------
  template<typename MemSourceT>
  void send(const BugOut_<OBBuf_<MemSourceT>>& xout) {
    if(xout.buf().is_open()) {
      return handle_error(xsstate::fail,
        "TCPSocket::send(BugOut_ xout). xout.buf().is_open() := false. xout must be closed");
    }
    if(xout.head().total_size == 0) {
      return handle_error(xsstate::fail,
        "TCPSocket::send(BugOut_ xout). xout.head().total_size := 0. xout must be not empty");
    }

    send(xout.buf().data(), xout.buf().size());
  }

  template<typename MemSourceT>
  void recv(BugIn_<IBBuf_<MemSourceT>>& xin) {
    if(xin.buf().size() != 0) {
      return handle_error(xsstate::fail,
        "TCPSocket::recv(BugIn_ xin). xin.buf().size() :!= 0. xout must be empty");
    }
    
    // -- try read Head
    char* ptr_head = xin.buf().push_reserve(k_head_size);
    if (recv(static_cast<void*>(ptr_head), static_cast<size_t>(k_head_size)) < k_head_size){
      return handle_error(xsstate::fail, 
        "TCPSocket::recv(BugIn_ xin). recv(header) < k_head_size. `head` has wrong len");
    }
    xin.scan_head();

    // -- read Data. if fail -> throw Exception
    assert(xin.head().total_size >= k_head_size);
    size_t data_size = xin.head().total_size - k_head_size;
    char* ptr_data = xin.buf().push_reserve(data_size);
    if(recv(static_cast<void*>(ptr_data), data_size) < data_size) {
      return handle_error(xsstate::fail, 
        "TCPSocket::recv(BugIn_ xin). recv(data) < k_head_size. `data` has wrong len");
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
    if (getpeername(socket_id_, reinterpret_cast<sockaddr*>(&address), &size)) {
      handle_error(xsstate::error, "TCPSocket::remoteaddress(). `getpeername` failed");
    }
    else {
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
    if (getpeername(socket_id_, reinterpret_cast<sockaddr*>(&address), &size)) {
      handle_error(xsstate::error, "TCPSocket::remoteport(). `getpeername` failed");
    }
    else{
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
    if (getsockname(socket_id_, reinterpret_cast<sockaddr*>(&address), &size)) {
      handle_error(xsstate::error, "TCPSocket::remoteport(). `getsockname` failed");
    }
    else{
      out = ntohs(address.sin_port);
    }
    return out;
  }

  Mode mode_ = Mode::undef;
};


// ------------------------------------------------------------
class TCPListener : public SocketBase {
 public:
  
  TCPListener() = default;

  // param[in] nconn - number of connections for listenning
  void listen(unsigned short port, int nconn = SOMAXCONN, IPAddress address = IPAddress{0, 0, 0, 0}) {
    close();
    create();
    assert(is_good() && is_valid());
    
    if (address.is_none()) { 
      return handle_error(xsstate::fail, "TCPListner::listen. `address` := `none`");
    }

    sockaddr_in addr = impl::create_address(address.to_int(), port);
    if (::bind(socket_id_, 
               reinterpret_cast<sockaddr*>(&addr),
               sizeof(addr)) == impl::k_socket_error)
    {
      return handle_error(xsstate::error, "TCPListener::listen(). `bind` failed");
    }

    if (::listen(socket_id_, nconn) == impl::k_socket_error) {
      return handle_error(xsstate::error, "TCPListener::listen(). `listen` failed");
    }
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
      handle_error(xsstate::error, "TCPListener::localport(). `getsockname` gailed");
    } else {
      out = ntohs(address.sin_port);
    }
    return out;
  }
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
