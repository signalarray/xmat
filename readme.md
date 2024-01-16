XMAT binary format
=======================

Supported types
---------------
```
typename | size(bytes) | info
----------------------------------
_char       1             ASCII characters
  
ri08        1             real signed int8      
ri16        2             real signed int16
ri32        4             real signed int32
ri64        8             real signed int64
ru08        1             real unsigned int8
ru16        2             real unsigned int16
ru32        4             real unsigned int32
ru64        8             real unsigned int64
rf32        4             real float(32)
rf64        8             real double(64)

ci08        2             complex signed int8
ci16        4             complex signed int16
ci32        8             complex signed int32
ci64        16            complex signed int64
cu08        2             complex unsigned int8
cu16        4             complex unsigned int16
cu32        8             complex unsigned int32
cu64        16            complex unsigned int64
cf32        8             complex <float(32), float32>
cf64        16            complex <double(64), double(64)>
```

Format Scheme
-------------
```
Header:
  8b total size
  4b format_signature
  1b uintx_size => 1u       # {4, 8} bytes
  1b max_block_name_lenght
  1b max_type_name_lenght
  1b max_ndim
  4b format_signature

Block[0]:
  4b block_signature_begin
  xb name           [h.max_block_name_lenght * b]
  xb typename       [h.max_type_name_lenght * b]
  xu shape          [h.max_ndim * u]
  1u numel
  1b typesize
  1b ndim
  4b block_signature_end

Data[0]:
  xx data           [b.numel * b.typesize]

...

Block[N]:
  ...
Data[N]:
  ...
```


Walking trough blocks
---------------------
```
q1 = max_block_name_len
q2 = max_type_name_len
q3 = max_ndim

header_size = 2*4b + 4*1b = 12*1b

block_size = @(1u,q1,q2,q3) 2*4b + (q1+q2)*1b + q3*1u + 1u + 2*1b
bloc_size(4, 32, 8, 4) = 67*1b

cursor = header_size
while (!end):
  cursor += 

```


Project tree plan
-----------------

```
cpp:
  include/xmat:
    xmat.hpp
    xarr.hpp
    xsocket.hpp

  test:
    test0.cpp
    CMakeLists.txt

  examples:
    example_xarr.cpp
    example_eigen.cpp
    example_armadillo.cpp
    example_xtensor.cpp    
    example_opencv.cpp
    CMakeLists.txt
  CMakeLists.txt

matlab:
  +xmat:
  examples:

python:
  xmat.py
  test.py
  examples.py

data:
  file.txt

.gitignore
readme.md
changelog.md
CMakeLists.txt
```