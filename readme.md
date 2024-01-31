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
rf32        4             real float32
rf64        8             real double64

ci08        2             complex signed int8
ci16        4             complex signed int16
ci32        8             complex signed int32
ci64        16            complex signed int64
cu08        2             complex unsigned int8
cu16        4             complex unsigned int16
cu32        8             complex unsigned int32
cu64        16            complex unsigned int64
cf32        8             complex float32
cf64        16            complex double64
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
  xb name           [h.max_block_name_lenght * 1b]
  xb typename       [h.max_type_name_lenght * 1b]
  xu shape          [h.max_ndim * 1u]
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
