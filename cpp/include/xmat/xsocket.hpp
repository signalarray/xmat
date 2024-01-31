/* 
*/

#pragma once

// linux
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
// #define _XOPEN_SOURCE_EXTENDED 1     // https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-recv-receive-data-socket#d386468e303
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

// c-lang
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// cpp-lang
#include <iostream>
#include <string>
#include <sstream>
#include <array>
#include <algorithm>
#include <exception>
#include <chrono>
#include <thread>


namespace xmat {

using uint = std::size_t;

const char* const IP_SERVER = "0.0.0.0";
const char* const IP_LOCALHOST = "127.0.0.1";
const char* const PORT = "4096";
const int PORT_NUM = 4096;
const uint kDefaultMsgLen = 128;
const uint kMaxIpLen = INET6_ADDRSTRLEN;
const uint kMaxPortLen = 16;


class SocketError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};


namespace impl {

std::array<char, kMaxPortLen> portstr(int a) {
  std::array<char, kMaxPortLen> str{};
  std:sprintf(str.begin(), "%d", a);
  return str;
}

template<uint N>
uint assign(std::array<char, N>& dest, const char* src) {
  auto n = std::strlen(src) + 1;
  if (n > N) { throw std::overflow_error("cxx::assing()"); }
  std::copy_n(src, n, dest.begin());
  return n;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

} // namespace `impl`


class Socket final {
 public:

  enum class Mode {
    server,
    connection,
    client,
    undef
  };

  Socket() = default;

  // for server: ip := 0.0.0.0
  // if blocked == true - blocking 
  // if blocked == false - non-blocking
  // See also: about EWOULDBLOCK
  // https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-recv-receive-data-socket
  Socket(const char* ip_addres, const char* port, bool blocked=true) {
    impl::assign(ip_, ip_addres);
    impl::assign(port_, port);
    blocked_ = blocked;

    mode_ = strcmp(ip_addres, IP_SERVER) == 0 ? Mode::server : Mode::client;
    addrinfo hints{};
	  hints.ai_family = AF_INET;
	  hints.ai_socktype = SOCK_STREAM;
    if (mode_ == Mode::server) { // may be isn't necessary
	    hints.ai_flags = AI_PASSIVE; // use my IP
    }

    const int rv = getaddrinfo(ip_.cbegin(), port_.cbegin(), &hints, &addrinfo_);
	  if (rv != 0) {
      std::ostringstream oss{};
      oss << "getaddrinfo: " << gai_strerror(rv) << '\n';
      throw SocketError(oss.str());
    }

    // get ip string
		sockaddr_in *ipv4 = (sockaddr_in *)addrinfo_->ai_addr;
    inet_ntop(addrinfo_->ai_family, &(ipv4->sin_addr), ip_.begin(), sizeof(ip_.size()));

    // make socket descriptor
    socket_id_ = ::socket(addrinfo_->ai_family, addrinfo_->ai_socktype, addrinfo_->ai_protocol);
    if (socket_id_ < 0 ) { throw SocketError("error in: SocketTCP{}"); }
    
    // for server.accept() non blocking.
    // note: may be just for server
    // if (mode_ == Mode::server) { ::fcntl(socket_id_, F_SETFL, O_NONBLOCK); }
    if (!blocked_) { ::fcntl(socket_id_, F_SETFL, O_NONBLOCK); }
    is_open_ = true;
  }

  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;
  Socket(Socket&&) noexcept = default;
  Socket& operator=(Socket&&) noexcept = default;
  ~Socket() { close(); } // close can throw Exception

  void close() {
    if(!is_open_) { return; }

    if (addrinfo_) {
      ::freeaddrinfo(addrinfo_);
      addrinfo_ = nullptr;
    }
    const int out = ::close(socket_id_);
    if (out) { throw SocketError{"error in: SocketTCP::close(), can't close the socket\n"}; }
    is_open_ = ready_ = false;
  }

  bool is_open() { return is_open_; }

  // `client` methods
  // ----------------

  // support both: blocking and non-bloking socket. 
  // if blocking - timeout_sec - just ignored. does one attempt to connect in blokking way
  void connect(const double timeout_sec) {
    assert(is_open_);
    if(mode_ != Mode::client) { throw SocketError("Socket::connet(). Socket isn't `client`"); }

    const std::chrono::duration<double> tt{timeout_sec};
    const std::chrono::duration<double> dt{0.05};
    const auto tend = std::chrono::system_clock::now() + tt;

    int out = 1;
    do {
      out = ::connect(socket_id_, addrinfo_->ai_addr, addrinfo_->ai_addrlen);
      if (!blocked_ && out < 0) {
        std::this_thread::sleep_for(dt); 
      }
    } while (!blocked_ && out < 0 && std::chrono::system_clock::now() < tend);

    if (out < 0) { 
      std::ostringstream oss;
      oss << "Socket.client.connect(timeout) fail. errno(" << errno << ")::" << strerror(errno) << "\n";
      throw SocketError(oss.str());
    }
    ready_ = true;
  }

  void connect_default_() {
    assert(is_open_);
    if(mode_ != Mode::client) { throw SocketError("Socket::connet(). Socket isn't client"); }
    const int out = ::connect(socket_id_, addrinfo_->ai_addr, addrinfo_->ai_addrlen);
    if (out < 0) { 
      std::ostringstream oss;
      oss << "Socket.client.connect fail. errno(" << errno << ")::" << strerror(errno) << "\n";
      throw SocketError(oss.str());
    }
    ready_ = true;
  }

  // `server` methods
  // ----------------

  // always blocking
  // it's better to always call it
  void set_options(int options = SO_REUSEADDR | SO_REUSEPORT) {
    // may be just for server. may be no.
    if(mode_ != Mode::server) { throw SocketError("set_options(). Socket.mode != `server`"); }
    int yes = 1;
    const int rv2 = setsockopt(socket_id_, SOL_SOCKET, options, &yes, sizeof(options));
    if (rv2 < 0) { throw SocketError{"error in: SocketTCP::set_options"}; }
  }

  // always blocking
  void bind() {
    if(mode_ != Mode::server) { throw SocketError("bind(). Socket.mode != `server`"); }
    assert(is_open_);
    const int out = ::bind(socket_id_, addrinfo_->ai_addr, addrinfo_->ai_addrlen);
    if (out < 0) { throw SocketError{"nInvalid address/Address not supported \n"}; }
  }

  // always blocking
  void listen(int max_queue) {
    if(mode_ != Mode::server) { throw SocketError("listen(). Socket.mode != `server`"); }
    const int out = ::listen(socket_id_, max_queue);
    if (out < 0) { throw SocketError{"error in: Socket::listen()"}; }
  }

  Socket accept() {
    Socket connection{};
    accept__(connection);
    return connection;
  }

  // support both: blocking and non-blocking ways
  // wait for input connection over `timeout_sec` with `time_step`-long pauses
  Socket accept(double timeout_sec) {
    std::chrono::duration<double> tt{timeout_sec};
    std::chrono::duration<double> dt{0.05};
    auto tend = std::chrono::system_clock::now() + tt;

    Socket connection{};
    do {
      accept__(connection);
      if (connection.is_open_) { 
        break;
      }
      std::this_thread::sleep_for(dt); // or use: std::this_thread::yield();
    } while (std::chrono::system_clock::now() < tend);
    return connection;
  }

  bool accept__(Socket& connection) {
    assert(is_open_);
    if(mode_ != Mode::server) { throw SocketError("accept(). Socket.mode != `server`"); }
    bool flag = false;
    socklen_t sz = sizeof(connection_addr_);
    int new_socket_id = ::accept(socket_id_, (sockaddr*)&connection_addr_, &sz);

    if (new_socket_id >= 0) {
      flag = true;
      connection.blocked_ = blocked_;
      if (!connection.blocked_) { ::fcntl(new_socket_id, F_SETFL, O_NONBLOCK); }

      connection.socket_id_ = new_socket_id;
      connection.is_open_ = true;
      connection.ready_ = true;
      connection.mode_ = Mode::connection;

      // input connection ip
      inet_ntop(connection_addr_.ss_family,
                impl::get_in_addr((sockaddr*)&connection_addr_),
                connection.ip_.begin(),
                connection.ip_.size());              
      impl::assign(connection.port_, port_.begin());
    }
    return flag;
  }

  // `connection` methods for exchange
  // ---------------------------------

  // do in blockking way in both cases: bloking and non-blocking socket.
  // if sending successful: return int > 0
  // else: throw SocketException
  int send(const char* buff, const int len) {
    if(!(mode_ == Mode::client || mode_ == Mode::connection)) {
      throw SocketError("send(). mode != Mode::client && mode != Mode::connection");
    }
    assert(ready_);

    int const max_attempt = 128;
    int count = 0;
    int total = 0, bytesleft = len, n;

    while(total < len) {
        count += 1;
        n = ::send(socket_id_, buff+total, bytesleft, 0);
        if (n == -1 || count > max_attempt) { 
          break; 
        }
        total += n;
        bytesleft -= n;
    }
    if(n < 0) { 
      std::ostringstream oss;
      oss << "Socket.send() failed: out = send(...) < 0. errno: " << ::strerror(errno) << "\n";
      throw SocketError(oss.str());
    }
    if(count > max_attempt ) { SocketError("Socket.send() faild: max_attempt exceeded"); }
    return count;
  }

  // Blocking rule:
  // -------------
  // mode blocking:
  //  - all data received
  //  - error code returned after ::recv
  //
  // mode non-blocking:
  //  - time is out:
  //    - behaves like `blocking` if any data sample was received during the timeout
  //    - didn't receive any sample of data: return -1;
  //  
  // Return
  // ------
  // int > 0: if success
  // -1 : if data isn't available (only for non-blocking mode)
  // 
  // Exceptions
  // ----------
  // SocketError: 
  //  - error in recv
  //  - connection closed
  //  - time exceed
  int resv(char* buff, const int len, double timeout_sec = 1.0) {
    if(!(mode_ == Mode::client || mode_ == Mode::connection)) {
      throw SocketError("resv(). mode != Mode::client && mode != Mode::connection"); 
    }    
    assert(ready_);

    std::chrono::duration<double> tt{timeout_sec};
    std::chrono::duration<double> dt{0.05};
    auto tend = std::chrono::system_clock::now() + tt;

    int count = 0;
    int total = 0, bytesleft = len, n;

    do {
      count += 1;
      n = ::recv(socket_id_, buff+total, bytesleft, 0);
      if (n == 0 || (n == -1 && errno != EWOULDBLOCK)) {
        break;
      }
      n = int(!(n==-1)) * n; // n = n == -1 ? 0 : n;
      total += n;
      bytesleft -= n;
      if (total == 0) { // just if didn't accept a single piece of data
        std::this_thread::sleep_for(dt);
      }
    } while (total < len      // length condition
             && (blocked_ ||  // if socket is blocking - timeout doesn't matter
                 !(total == 0 && std::chrono::system_clock::now() > tend)));  // if buffer is steel empty

    if (n == -1 && errno == EWOULDBLOCK) {
      return -1; // just the data isn't available

    } else {
      if(n < 0) {
        std::ostringstream oss;
        oss << "Socket.send() failed: out = send(...) < 0. errno: " << strerror(errno) << "\n";
        throw SocketError(oss.str()); 
      }
      if(n == 0) { throw SocketError("Socket.send() failed: out = send(...) == 0. Means connection is closed"); }
      // timeout exceed
      if(total == 0) { throw SocketError("Socket.send() failed. timeout exceed."); }
    }
    return count;
  }

 public:
  std::array<char, kMaxIpLen> ip_{};
  std::array<char, kMaxPortLen> port_{};

  int socket_id_ = -2;
  bool is_open_ = false;
  bool ready_ = false;
  Mode mode_ = Mode::undef;
  bool blocked_ = false;

 private:
  // for: server, client
  addrinfo* addrinfo_ = nullptr;

  // for: connection
  sockaddr_storage connection_addr_{};
};

} // namespace xmat
