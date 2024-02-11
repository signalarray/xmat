#pragma once

#include "xformat.hpp"
#include "xsocket.hpp"


namespace xmat {

// provaides protocol for xmat-data format exchange
// ------------------------------------------------

class ConnectionError : public SocketError {
 public:
  using SocketError::SocketError;
}; 


enum class Policy {
  fix,    // don't change buff
  dyn,    // 
  // once,   // resize buff just once
};


class Communication {
 public:
  virtual ~Communication() = default;
  Communication(const Communication&) = delete;
  Communication& operator=(const Communication&) = delete;
  Communication(Communication&&) noexcept = default;
  Communication& operator=(Communication&&) noexcept = default;

  Communication(const char* ip_addres, const char* port, bool blocked=true) {
    socket_ = Socket(ip_addres, port, blocked);

    if (socket_.mode_ == Socket::Mode::server) {
      socket_.set_options();
      socket_.bind();
      socket_.listen(4);
    }
  }

  void close () {
    
  }

  // for both `server` and `connection` try to establish connection:
  // if `server` call accespt()
  // if `client` call connect()
  // 
  // Return
  // ------
  // 
  // Exceptions
  // ----------
  // NetConnectionError() - if `client` and connection failed
  //                      - if any error during
  bool wait_for_connection(double timeout_sec, bool raise_exception=true) {
    bool flag = false;
    if(socket_.is_server()) {
      connection_object_ = socket_.accept(timeout_sec);
      if (connection_object_.is_open()) {
        flag = true;
        connection_ = &connection_object_;
      } else if(raise_exception) { // throw SocketError if isn't success
        throw ConnectionError("Connection.wait_for_connection.server failed");
      }
    } else { // if client
        socket_.connect(timeout_sec);  // throw SocketError if isn't success
        flag = true;
        connection_ = &socket_;
    }
    return flag;
  }

  // send packet
  void send(const Output& xout) {
    if(!connection_) { throw ConnectionError("Communication insn`t connected"); }
    if(xout.is_open()) { throw ConnectionError("Output{bytes} must be closed before sending"); }
    if(!xout.mode_bytes()) { throw ConnectionError("Output.mode must be `bytes`"); }
    const StreamBytes& sbytes = xout.stream_bytes();
    
    connection_->send(sbytes.buff(), sbytes.size());
  }

  void resend(const Input& xout) {
    if(!connection_) { throw ConnectionError("Communication insn`t connected"); }
    if(!xout.mode_bytes()) { throw ConnectionError("Output.mode must be `bytes`"); }
    const StreamBytes& sbytes = xout.stream_bytes();
    
    connection_->send(sbytes.buff(), sbytes.size());
  }

  // receive packet
  bool recv(Input& xin, double timeout_sec=1.0, Policy policy=Policy::dyn) {
    if(!connection_) { throw ConnectionError("Communication insn`t connected"); }
    // if(xin.is_open()) { throw ConnectionError("Output{bytes} must be closed before sending"); }
    if(!xin.is_empty()) { throw ConnectionError("Input stream isn't `empty`"); }
    if(!xin.mode_bytes()) { throw ConnectionError("Output.mode must be `bytes`"); }
    
    StreamBytes& sbytes = xin.stream_bytes();
    if(sbytes.isfixed() && policy != Policy::fix) {
      throw ConnectionError("wrong policy for fixed StreamBytes.buffer");
    }

    // process header
    // -----------------------------
    if(sbytes.capacity() < kHeadSize) {
      if(policy == Policy::fix) {
        throw ConnectionError("Small StreamBytes.buffer.Input.stream.buff_size isn`t small");
      }
      else if(policy == Policy::dyn) { assert(!sbytes.isfixed()); sbytes.reserve(kHeadSize); }
      else { 
        throw ConnectionError("just in case"); 
      }
    }
    int out = connection_->resv(sbytes.buff(), kHeadSize, timeout_sec);
    if(out == 0) {
       return false; // return if header wasn't received
    }

    try { 
      xin.scan_header(); 
    }catch (std::runtime_error& err) {
       throw ConnectionError(std::string{"Connection.recv.scan_header(): "} + err.what());
    }
    
    // process buff
    // --------------------
    const Head& h = xin.header();
    if(sbytes.capacity() < h.total_size) {
      if(policy == Policy::fix) {
        throw ConnectionError("Small StreamBytes.buffer.Input.stream.buff_size isn`t small");
      }
      else if(policy == Policy::dyn) {
        assert(!sbytes.isfixed());
        sbytes.reserve(h.total_size);
      } else { 
        throw ConnectionError("just in case"); 
      }
    }
    out = connection_->resv(sbytes.buff() + sbytes.tell(), 
                            h.total_size - sbytes.tell(), 
                            timeout_sec);
    if(out==0) {
      throw ConnectionError("just header was resaved. data part wasn't");
    }

    // scan data
    try {
      xin.scan_data();
    } catch (std::exception& err) {
      throw ConnectionError(std::string{"Connection.recv.scan_data(): "} + err.what());
    }
    return true;
  }
  
 public:
  Socket* connection_ = nullptr;
  Socket socket_;              // storage for server socket. if Connected as Server
  Socket connection_object_;   // storage for sccepted connections
};

} // namespase xmat
