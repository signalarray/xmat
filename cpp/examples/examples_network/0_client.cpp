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

    // send messages
    const size_t N = 6;
    for (int n = 0, Nlast = N-1; n < N; ++n) {
      cout << "iter " << n << "\n";
      auto oss = std::ostringstream{};
      if (n == Nlast) {
        oss << "close";
      }
      else {
        oss << "message: [" << n << "] content";
      }
      std::string str = oss.str();
      str.resize(xmat::k_xsbuf_size);

      socket->sendall(str.c_str(), str.size(), 1.0);
      cout << "sent:  " << oss.str() << "\n";

      if (n == Nlast) break;

      // waiting for reply
      std::fill_n(buf, xmat::k_xsbuf_size, '\0');
      socket->recvall(buf, xmat::k_xsbuf_size, 1.0);
      cout << "reply: " << buf << "\n";

      xmat::time::sleep(1.5);
    }
    cout << "finish successfully\n";
  }
  catch (std::exception& err) {
    cout << "exception in main: message: >>\n" 
         << err.what();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
