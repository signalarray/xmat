#include <cstring>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <exception>

#include "../../include/xmat/xsocket.hpp"
#include "../../include/xmat/xserial.hpp"
#include "../../include/xmat/xprint.hpp"
#include "../common.hpp"


std::ostream* kOutStream = &std::cout;


int main() {  
  using std::cout;
  char buf[xmat::k_xsbuf_size + 1] = {};

  try {
    // connect
    xmat::TCP tcp{};
    auto socket = tcp.client(xmat::IPAddress::localhost(), xmat::k_xsport, 1.0);
    cout << "connected\n";

    cout << "send messages\n";
    const std::uint64_t N = 8, Nlast = N - 1;
    for (std::uint64_t n = 0; n < N; ++n) {
      cout << "send iter " << n << "\n";

      xmat::OMapStream<> xout{};
      xout.setitem("n", n);
      if (n == Nlast) xout.setitem("stop", 1);
      else xout.setitem("msg", std::string{"string content"});
      xout.close();
      socket->send(xout, 1.0);

      const auto count = tcp.wait(0.5, std::nothrow);
      if (count) {
        xmat::IMapStream<> xin{};
        socket->recv(xin, 1.0);
        cout << "recv while sending n: " << xin.at("n").get<std::uint64_t>() << "\n";
      }
    }

    cout << "\n\nreceive remaining messages\n";
    while(true) {
      tcp.wait(10.0);
      xmat::IMapStream<> xin{};
      socket->recv(xin, 1.0);
      auto n = xin.at("n").get<std::uint64_t>();
      cout << "recv after sending n: " << n << "\n";
      if (n == Nlast - 1) break;
    }
    cout << "finish successfully\n";
  }
  catch (std::exception& err) {
    cout << "\nexception in main: message: >>\n" 
         << err.what();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
