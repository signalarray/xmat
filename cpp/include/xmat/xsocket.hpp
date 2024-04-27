#pragma once

#ifdef _WIN32
#define XMAT_USE_WINSOCKET

#undef UNICODE
#define WIN32_LEAN_AND_MEAN

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



// project
#include "xutil.hpp"


namespace xmat {

using portint_t     = unsigned short;

const size_t k_sock_buf_n = 1024;
const portint_t k_port = 27015;
constexpr const char* k_cport = "27015";

enum class SocketState {
  done,
  not_ready,
  partial,
  disconnected,
  error
};

// socket functions namespace
namespace impl {

#ifdef XMAT_USE_WINSOCKET
using socket_t      = SOCKET;
using address_len_t = int;
using size_p        = int;
#else // unix
using socket_t      = int;
using address_len_t = socklen_t;
using size_p        = size_t;
#endif


#ifdef XMAT_USE_WINSOCKET
const int tcp_send_flags = 0;
const auto tcp_so_reuseaddr = SO_REUSEADDR;

sockaddr_in create_address(std::uint32_t address, unsigned short port) {
    sockaddr_in addr{};
    addr.sin_addr.s_addr = htonl(address);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    return addr;
}

constexpr socket_t k_invalid_socket = INVALID_SOCKET;

socket_t invalid_socket() { return k_invalid_socket; }

void close(socket_t socket) { closesocket(socket); }

void set_timeout(socket_t socket, size_t us) { /*TODO*/ }

SocketState get_error() {
  switch (WSAGetLastError()) {
    case WSAEALREADY:     return SocketState::not_ready;
    case WSAEWOULDBLOCK:  return SocketState::not_ready;
    case WSAECONNRESET:   return SocketState::disconnected;
    case WSAENETRESET:    return SocketState::disconnected;
    case WSAECONNABORTED: return SocketState::disconnected;
    case WSAETIMEDOUT:    return SocketState::disconnected;
    case WSAENOTCONN:     return SocketState::disconnected;
    case WSAEISCONN:      return SocketState::done;
    default:              return SocketState::error;
  }
}

inline void print_error(int errcode) {
  const DWORD size = 256;
  char buffer[size];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                NULL, 
                errcode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_ENGLISH_US), 
                buffer, 
                size, 
                NULL);
  std::printf("xmat::xsocket::print_error : %s\n", buffer);
}

inline int print_error() { 
  int errcode = WSAGetLastError(); 
  print_error(errcode); 
  return errcode;
}

#else
const int tcp_send_flags = MSG_NOSIGNAL;
const auto tcp_so_reuseaddr = SO_REUSEADDR | SO_REUSEPORT;

sockaddr_in create_address(std::uint32_t address, unsigned short port) {
    sockaddr_in addr{};
    addr.sin_addr.s_addr = htonl(address);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    // addr.sin_len = sizeof(addr); // MAC
    return addr;
}

constexpr socket_t k_invalid_socket = -1;

socket_t invalid_socket() { return k_invalid_socket; }

void close(socket_t socket) { ::close(socket); }

void set_timeout(socket_t socket, size_t us) { /*TODO*/ }

SocketState get_error() {
  if ((errno == EAGAIN) || (errno == EINPROGRESS)) {
    return SocketState::not_ready;
  }

  switch (errno) {
    case EWOULDBLOCK:  return SocketState::not_ready;
    case ECONNRESET:   return SocketState::disconnected;
    case ENETRESET:    return SocketState::disconnected;
    case ECONNABORTED: return SocketState::disconnected;
    case ETIMEDOUT:    return SocketState::disconnected;
    case ENOTCONN:     return SocketState::disconnected;
    case EPIPE:        return SocketState::disconnected;
    default:           return SocketState::error;
  }
}
#endif
} // namespace impl

class SocketError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};


class SocketStartup {
 public:

  size_t counter = 0;

  static SocketStartup* init() {
    static SocketStartup socket;
    socket.counter += 1;  // just for size-effect. to avoid compilter optimization.
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
      assert(false);
#ifndef NDEBUG
      std::printf("xmat::SocketStartup::~SocketStartup(). WSACleanup failed with error:\nerrno: %d", WSAGetLastError());
#endif      
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
        throw SocketError("xmat::SocketStartup::SocketStartup(). WSAStartup failed with error");
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
class TCPBase {
 public:
  virtual ~TCPBase() { close(); }

  TCPBase(const TCPBase&) = delete;

  TCPBase& operator=(const TCPBase&) = delete;

  TCPBase(TCPBase&& other) : socket_id_{other.socket_id_} {
    other.socket_id_ = impl::invalid_socket();
  }
  
  TCPBase& operator=(TCPBase&& other) {
    if (&other == this)  { return *this; }
    close();
    socket_id_ = other.socket_id_;
    other.socket_id_ = impl::invalid_socket();
    return *this;
  }

  // error handling
  // --------------
  bool is_invalid() const noexcept { return socket_id_ == impl::k_invalid_socket; }

  SocketState state() const noexcept { return status_; }

  // if val := true, throw exception if eny error
  // if val := false, set status
  void exceptions(bool val) noexcept { flag_exceptions_ = val; }

  bool exceptions() const noexcept { return flag_exceptions_; }

 protected:
  TCPBase() { SocketStartup::init(); }

  void create() {
    assert(is_invalid() && "xmat::Socket::create(...) current state must be non-valid");
    if (is_invalid()) { return; }

    const impl::socket_t sock = ::socket(PF_INET, SOCK_STREAM, 0);
    if (sock == impl::k_invalid_socket) {
      throw SocketError("xmat::TCPBase.create() error\n");
    }
  }

  void settings(impl::socket_t sock) {
    assert(is_invalid() && "xmat::Socket::settings(...) current state must be non-valid");
    if (!is_invalid()) { return; }

    socket_id_ = sock;
    const int yes = 1;
    if(::setsockopt(socket_id_, SOL_SOCKET, impl::tcp_so_reuseaddr, 
                    reinterpret_cast<const char*>(&yes), sizeof(yes)) == -1) {
                    
    }
    if (::setsockopt(socket_id_, IPPROTO_TCP, TCP_NODELAY,
                      reinterpret_cast<const char*>(&yes), sizeof(yes)) == -1) {

    }
  }

  void close() {
    if (is_invalid()) { return; }
    impl::close(socket_id_);
    socket_id_ = impl::k_invalid_socket;
  }

  impl::socket_t socket_id_ = impl::k_invalid_socket;
  SocketState status_ = SocketState::done;
  bool flag_exceptions_ = true;
};


// ------------------------------------------------------------
class TCPSocket : public TCPBase {
 public:
  enum class Mode { client, connection, undef };

  friend class TCPListener;

  TCPSocket() = default;

  SocketState connect(IPAddress address, unsigned int port) {
    disconnect();
    create();

    sockaddr_in address_v = impl::create_address(address.to_int(), port);
    if (::connect(socket_id_, reinterpret_cast<sockaddr*>(&address_v), sizeof(address_v)) == -1) {
      return impl::get_error();
    }
    return SocketState::done;
  }

  // just in case
  SocketState connect(const char* address, const char* port) {
    disconnect();

    addrinfo *result = NULL, *ptr = NULL, hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    auto iResult = getaddrinfo(address, port, &hints, &result);
    if ( iResult != 0 ) { 
      impl::print_error(); 
      return SocketState::error;
    }

    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
      create();
      if(::connect(socket_id_, ptr->ai_addr, (int)ptr->ai_addrlen) == -1) {
        close();
      }
      break;
    }
    ::freeaddrinfo(result);
    return socket_id_ != impl::invalid_socket() ? SocketState::done : SocketState::error;
  }

  void disconnect() { close(); }


  // ----
  IPAddress remoteaddress() const { 
    if (socket_id_ != impl::invalid_socket()) {
      sockaddr_in address{};
      impl::address_len_t size = sizeof(address);
      if (getpeername(socket_id_, reinterpret_cast<sockaddr*>(&address), &size) != -1) {
        return IPAddress(ntohl(address.sin_addr.s_addr));
      }
    }
    return IPAddress::none();
  }
  
  unsigned short remoteport() const { 
    if (socket_id_ != impl::invalid_socket()) {
      sockaddr_in address{};
      impl::address_len_t size = sizeof(address);
      if (getpeername(socket_id_, reinterpret_cast<sockaddr*>(&address), &size) != -1) {
        return ntohs(address.sin_port);
      }
    }
    return 0;
  }

  unsigned short localport() const { 
    if (socket_id_ != impl::invalid_socket()) {
      sockaddr_in address{};
      impl::address_len_t size = sizeof(address);
      if (getsockname(socket_id_, reinterpret_cast<sockaddr*>(&address), &size) != -1) {
        return ntohs(address.sin_port);
      }
    }
    return 0;
  }

  Mode mode_ = Mode::undef;
};



// ------------------------------------------------------------
class TCPListener : public TCPBase {
 public:
  
  TCPListener() = default;

  SocketState listen(unsigned short port, IPAddress address = IPAddress{0, 0, 0, 0}) {
    close();
    create();
    
    if (address.is_none()) { return SocketState::error; }

    sockaddr_in addr = impl::create_address(address.to_int(), port);
    if (::bind(socket_id_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
      printf("Failed to bind listener socket\n");
      impl::print_error();
      return SocketState::error;
    }

    if (::listen(socket_id_, SOMAXCONN) == -1) {
      printf("Failed to bind listener socket\n");
      return SocketState::error;
    }
    return SocketState::done;
  }

  SocketState accept(TCPSocket& socket) {
    if (socket_id_ == impl::invalid_socket()) {
        printf("Failed to accept a new connection, the socket is not listening");
        return SocketState::error;
    }

    sockaddr_in address{};
    impl::address_len_t length = sizeof(address);
    const impl::socket_t remote = ::accept(socket_id_, reinterpret_cast<sockaddr*>(&address), &length);

    if (remote == impl::invalid_socket()) { return impl::get_error(); }

    socket.close();
    socket.settings(remote);

    return SocketState::done;
  }

  unsigned int localport() const {
    if (socket_id_ == impl::invalid_socket()) { 
      return 0;   
    }
    sockaddr_in address{};
    impl::address_len_t size = sizeof(address);
    if (getsockname(socket_id_, reinterpret_cast<sockaddr*>(&address), &size) != -1) {
        return ntohs(address.sin_port);
    }
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

