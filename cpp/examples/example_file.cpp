#include <cassert>
#include <iostream>
#include <fstream>

#include <cctype>
#include <iomanip>
#include <string>
#include <vector>
#include <array>
#include <complex>
#include <algorithm>
#include <type_traits>

#include "../include/xmat/xarray.hpp"
#include "../include/xmat/xserial.hpp"

#include "common.hpp"
#include "temp_data_folder.hpp"


std::ostream* kOutStream = &std::cout;

namespace {

  int write(const std::string& file_output) {
    print(__PRETTY_FUNCTION__, 1);
    print("write xmat::Array<T> to file.xmat", 1);

    using type_0 = double;
    xmat::NArray<type_0, 1> x0{{1}};
    x0(0) = 10.0 + 1.0/3.0;
    xmat::NArray<type_0, 1> x1{{3}};
    x1.enumerate();
    xmat::NArray<type_0, 2> x2{{3, 4}};
    x2.enumerate();
    xmat::NArray<type_0, 3> x3{{3, 4, 5}};
    x3.enumerate();
    xmat::NArray<type_0, 4> x4{{3, 4, 5, 6}};
    x4.enumerate();

    xmat::Output xout{xmat::StreamFileOut{file_output}};
    xout.setitem("f64x0", x0);
    xout.setitem("f64x1", x1);
    xout.setitem("f64x2", x2);
    xout.setitem("f64x3", x3);
    xout.setitem("f64x4", x4);

    using type_1 = float;
    xout.setitem("f32x0", x0.cast<type_1>());
    xout.setitem("f32x1", x1.cast<type_1>());
    xout.setitem("f32x2", x2.cast<type_1>());
    xout.setitem("f32x3", x3.cast<type_1>());
    xout.setitem("f32x4", x4.cast<type_1>());

    using type_2 = std::int8_t;
    xout.setitem("i8x0", x0.cast<type_2>());
    xout.setitem("i8x1", x1.cast<type_2>());
    xout.setitem("i8x2", x2.cast<type_2>());
    xout.setitem("i8x3", x3.cast<type_2>());
    xout.setitem("i8x4", x4.cast<type_2>());

    using type_3 = std::int32_t;
    xout.setitem("i32x0", x0.cast<type_3>());
    xout.setitem("i32x1", x1.cast<type_3>());
    xout.setitem("i32x2", x2.cast<type_3>());
    xout.setitem("i32x3", x3.cast<type_3>());
    xout.setitem("i32x4", x4.cast<type_3>());

    using type_4 = std::int64_t;
    xout.setitem("i64x0", x0.cast<type_4>());
    xout.setitem("i64x1", x1.cast<type_4>());
    xout.setitem("i64x2", x2.cast<type_4>());
    xout.setitem("i64x3", x3.cast<type_4>());
    xout.setitem("i64x4", x4.cast<type_4>());

    // unsigned
    using type_5 = std::uint8_t;
    xout.setitem("i8x0", x0.cast<type_5>());
    xout.setitem("i8x1", x1.cast<type_5>());
    xout.setitem("i8x2", x2.cast<type_5>());
    xout.setitem("i8x3", x3.cast<type_5>());
    xout.setitem("i8x4", x4.cast<type_5>());

    using type_6 = std::uint32_t;
    xout.setitem("i32x0", x0.cast<type_6>());
    xout.setitem("i32x1", x1.cast<type_6>());
    xout.setitem("i32x2", x2.cast<type_6>());
    xout.setitem("i32x3", x3.cast<type_6>());
    xout.setitem("i32x4", x4.cast<type_6>());

    using type_7 = std::uint64_t;
    xout.setitem("i64x0", x0.cast<type_7>());
    xout.setitem("i64x1", x1.cast<type_7>());
    xout.setitem("i64x2", x2.cast<type_7>());
    xout.setitem("i64x3", x3.cast<type_7>());
    xout.setitem("i64x4", x4.cast<type_7>());

    // complex
    // floating point
    using type_10 = std::complex<double>;
    xout.setitem("cf64x0", x0.cast<type_10>());
    xout.setitem("cf64x1", x1.cast<type_10>());
    xout.setitem("cf64x2", x2.cast<type_10>());

    using type_11 = std::complex<float>;
    xout.setitem("cf32x0", x0.cast<type_11>());
    xout.setitem("cf32x1", x1.cast<type_11>());
    xout.setitem("cf32x2", x2.cast<type_11>());

    // complex<integers> not supported now

    xout.close();
    printv(xout.header().total_size);
    
    print(1, "finish", 1, '-');
    return 1;
  }


  int write_1(const std::string& file_output) {
    print(__PRETTY_FUNCTION__, 1);
    print("write std::containers<T> to file.xmat", 1);

    xmat::Output xout{xmat::StreamFileOut{file_output}};

    using type_0 = double;
    using type_1 = float;
    using type_2 = std::int32_t;
    using type_3 = std::uint64_t;
    using type_4 = std::complex<double>;
    using type_5 = std::complex<float>;

    xout.setitem("t0x0", std::vector<type_0>{0, 1, 2, 3});
    xout.setitem("t0x1", std::array<type_0, 4>{0, 1, 2, 3});
    xout.setitem("t1x0", std::vector<type_1>{0, 1, 2, 3});
    xout.setitem("t1x1", std::array<type_1, 4>{0, 1, 2, 3});

    xout.setitem("t4x0", std::vector<type_4>{{0, 1}, {2, 3}, {4, 5}, {6, 7}});

    // strigs
    xout.setitem("string_oneline", 
      std::string{"some content in oneline string format: ~!@#$%^&*(){}[]<>+_`"});
    
    xout.setitem("string_multi_line", std::string{"line 1\nline 2\nline 3\n"});

    xout.close();
    printv(xout.header().total_size);
    
    print(1, "finish", 1, '-');
    return 1;
  }


  int read(const std::string& file_input) {
    print(__PRETTY_FUNCTION__, 1);
    print("reads data from file.xmat", 1);



    print(1, "finish", 1, '-');
    return 1;
  }
}



int main(int argc, char* argv[] ) {
  print("START: " __FILE__, 0, '=');

  try {
    std::string mode = "write";
    std::string folder = "cpp/";

    std::string file_data{data_folder};
    file_data += folder;
    file_data += "xout.xmat";

    if(mode == "write") {
      write_1(file_data);
    } else if(mode == "read") {
      read(file_data);
    }

    // 
  } 
  catch (std::exception& err) {
    print("exception in main():\n");
    print(err.what(), 1);
  }
  print("END: " __FILE__, 0, '=');
  return EXIT_SUCCESS;
}
