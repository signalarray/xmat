Overview
========

*xmat* is a cross platform serialization library meant for multi-dimensional numeric arrays structured in key-value format.

*xmat* provides:

- binary format for file and network data exchange.
- TCP-sockets aimed for this format
- data types supported:
    - integer and float numeric types (including complex numbers)
    - ascii-characters
- programming languages supported: C++14+, Python 3.10+, Matlab
- the following C++ compilers are supported::
    - Unix: GCC
    - Windows: Visual C++, MinGW 12


Types supported
---------------

.. list-table:: types summary
   :header-rows: 1

   * - xmat
     - C++
     - Python
     - NumPy
     - Matlab
     - Size
     - Id
   * - string
     - char
     - str
     - \numpy.str_
     - char
     - 1
     - 0x01
   * - [x] bool
     - bool
     - bool
     - \numpy.bool_
     - logical
     - 1
     - 0x02
   * - int8
     - std::int8_t
     - --
     - numpy.int8
     - int8
     - 1
     - 0x10
   * - int16
     - std::int16_t
     - --
     - numpy.int16
     - int16
     - 2
     - 0x11
   * - int32
     - std::int32_t
     - --
     - numpy.int32
     - int32
     - 4
     - 0x12
   * - int64
     - std::int64_t
     - int
     - numpy.int64
     - int64
     - 8
     - 0x13
   * - complex int8
     - std::complex<std::int8_t>
     - --
     - --
     - --
     - 2
     - 0x20
   * - complex int16
     - std::complex<std::int16_t>
     - --
     - --
     - --
     - 4
     - 0x21
   * - complex int32
     - std::complex<std::int32_t>
     - --
     - --
     - --
     - 8
     - 0x22
   * - complex int64
     - std::complex<std::int64_t>
     - --
     - --
     - --
     - 16
     - 0x23
   * - uint8
     - std::uint8_t
     - --
     - numpy.uint8
     - uint8
     - 1
     - 0x30
   * - uint16
     - std::uint16_t
     - --
     - numpy.uint16
     - uint16
     - 2
     - 0x31
   * - uint32
     - std::uint32_t
     - --
     - numpy.uint32
     - uint32
     - 4
     - 0x32
   * - uint64
     - std::uint64_t
     - --
     - numpy.uint64
     - uint64
     - 8
     - 0x33
   * - complex uint8
     - std::complex<std::uint8_t>
     - --
     - --
     - --
     - 2
     - 0x40
   * - complex uint16
     - std::complex<std::uint16_t>
     - --
     - --
     - --
     - 4
     - 0x41
   * - complex uint32
     - std::complex<std::uint32_t>
     - --
     - --
     - --
     - 8
     - 0x42
   * - complex uint64
     - std::complex<std::uint64_t>
     - --
     - --
     - --
     - 16
     - 0x43
   * - float32
     - float
     - --
     - numpy.float32
     - single
     - 4
     - 0x52
   * - float64
     - double
     - float
     - numpy.float64
     - double
     - 8
     - 0x53
   * - complex float32
     - std::complex<float>
     - --
     - numpy.complex64
     - single
     - 8
     - 0x62
   * - complex float64
     - std::complex<double>
     - complex
     - numpy.complex128
     - double
     - 16
     - 0x63


Benchmark
---------

.. list-table:: benchmark
   :header-rows: 1

   * - Size
     - 1K
     - 4K
     - 16K
     - 64K
     - 256K
     - 1024K
   * - **C++**: char
     - 1
     - 4
     - 16
     - 64
     - 256
     - 1024
   * - C++: double
     - 1
     - 4
     - 16
     - 64
     - 256
     - 1024
   * - C++: complex double
     - 1
     - 4
     - 16
     - 64
     - 256
     - 1024
   * - **Python**: char
     - 1
     - 4
     - 16
     - 64
     - 256
     - 1024
   * - Python: double
     - 1
     - 4
     - 16
     - 64
     - 256
     - 1024
   * - Python: complex double
     - 1
     - 4
     - 16
     - 64
     - 256
     - 1024
   * - **Matlab**: char
     - 1
     - 4
     - 16
     - 64
     - 256
     - 1024
   * - Matlab: double
     - 1
     - 4
     - 16
     - 64
     - 256
     - 1024
   * - Matlab: complex double
     - 1
     - 4
     - 16
     - 64
     - 256
     - 1024
