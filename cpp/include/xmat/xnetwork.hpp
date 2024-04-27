#pragma once

#include <sstream>
#include <utility>

#include "xutil.hpp"
#include "xsocket.hpp"
#include "xstream.hpp"

// ----------------
// Isn't actual now
// ----------------

namespace xmat {


class TCPConnection {
 public:

  TCPConnection() {} // is_open() == false

  // client
  TCPConnection(VString ip_addres, VString port, bool blocked=true) {
    socket_ = Socket(ip_addres, port, blocked);
  }

  // connection
  TCPConnection(Socket&& socket) : socket_{std::move(socket)} {
    assert(socket_.mode_ == Socket::Mode::connection || socket_.mode_ == Socket::Mode::undef);
  }

  void wait_for_connection(double timeout_sec) {
    assert(mode() == Socket::Mode::client && "TCPConnection.wait_for_connection(). .mode must be only client");
    if (mode() == Socket::Mode::client) {
      socket_.connect(timeout_sec);  // throw SocketError if isn't success
    }
    if (!is_open()) { 
      throw SocketError("xmat::TCPServer::wait_for_connection() failed");
    }
  }

  // read/write methods
  void send(const char* buff, const int len) { socket_.send(buff, len); }

  template<typename BugOutTArg>
  void send(const BugOut_<BugOutTArg>& xout) {
    if(xout.buf().is_open()) { throw SocketError("TCPConnection.send(): xout must be closed"); }
    if(xout.head().total_size == 0) { throw SocketError("TCPConnection.send(): xout.head.total_size must be grater 0"); }
    
    socket_.send(xout.buf().data(), xout.buf().size());
  }

  template<typename BugInTArg>
  bool resv(BugIn_<BugInTArg>& xout, double timeout_sec) {
    if(xout.buf().size() != 0) { throw SocketError("TCPConnection.resv(): xout must be empty()"); }
    
    // -- try read Head
    char buf_head[k_head_size];
    int flagok = socket_.resv(buf_head, k_head_size, timeout_sec);
    if(!flagok) { 
      return false;
    }
    xout.buf().push(buf_head, k_head_size);
    xout.scan_head();

    // -- read Data. if fail -> throw Exception
    auto data_size = xout.head().total_size - k_head_size;
    char* ptr_data = xout.buf().push_reserve(data_size);
    flagok = socket_.resv(ptr_data, data_size, timeout_sec);
    if(!flagok) {
      throw SocketError("TCPConnection.resv(): data receiving failed");
    }

    xout.buf().push_all();
    return true;
  }

  // getters
  bool is_open() const noexcept { return socket_.is_open(); }
  Socket::Mode mode() const noexcept { return socket_.mode_; }

 public:
  Socket socket_;
};


class TCPServer {
 public:
  TCPServer(VString port, bool blocked=true) {
    socket_ = Socket(xmat::IP_SERVER, port.ptr, blocked);
    socket_.set_options();
    socket_.bind();
    socket_.listen(4);
  }

  TCPConnection wait_for_connection(double timeout_sec) {
    TCPConnection conn{socket_.accept(timeout_sec)};
    if (!conn.is_open()) {
      throw SocketError("xmat::TCPServer::wait_for_connection() failed");
    }
    return conn;
  }

  Socket socket_;
};
}
