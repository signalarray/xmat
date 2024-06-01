TODO
====
```
cpp:
- [++++------] check MapStream<file>
- [++++++++--] cpp TCP full test
               + sudden shutdown handling
- [++++++++--] check NArray->serial
- [+++++++++-] change data format implementation
               - Head.BOM check-error
- [+++++++++-] add big-little endian support
```

# XMAT Format Description
```
    +---------+---------------------+---------------------+-----+-------------------------+
    |  Header |  Block[0] / Data[0] |  Block[1] / Data[1] | ... |  Block[N-1] / Data[N-1] |
    +---------+---------------------+---------------------+-----+-------------------------+
```

### Header Format: 17 bytes.
```
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    | Sign  |BOM|   Total Size  |I|S|B|
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

             Sign:  format signature
              BOM:  byte order marker
       Total Size:  message total length
                I:  size of int (always 8 )
                S:  maximum number of dimensions in a multidimensional array
                B:  maximum block name length
```

### Block Format: (8 + ndim * 8 + len(name)) bytes.

```
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |o|t|s|b|-zeros-| Shape[0]      / Shape[1]      / Shape[2]      /
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /     ...       / Shape[s-1]    |Block Name    ...         ...  /
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    | Data
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                o:  array elements order: {`C`: C-language, `F`: Fortran}, char
                t:  tipe id, uint8
                s:  number of dimentions, uint8
                b:  block name length (with out end '\0'), uint8
            Shape:  size of array in each dimension, uint8[s]
       Block Name:  block name, char[b]
```

### Data: numel * sizeof(type) bytes


Socket Communication Examples
==========================
Example 0:
----------
![socket-example-0](/doc/resources/socket-scheme-0.png)

Example 1:
--------------
![socket-example-1](/doc/resources/socket-scheme-1.png)

Example 2:
--------------
![socket-example-2](/doc/resources/socket-scheme-2.png)

# Types Supported

```
id    sizeof  label   name
------------------------
# 00-15
0x00  1       c       char
0x01  1       ?       bool  (specific implementation required)

# 16-31: i (*)
0x10  1       i0      int8
0x11  2       i1      int16
0x12  4       i2      int32
0x13  8       i3      int64
0x14  16      i4      int128

# 32-47: I
0x20  2       I0      complex int8
0x21  4       I1      complex int16
0x22  8       I2      complex int32
0x23  16      I3      complex int64
0x24  32      I4      complex int128

# 48-63: u
0x30  1       u0      unsigned int8
0x31  2       u1      unsigned int16
0x32  4       u2      unsigned int32
0x33  8       u3      unsigned int64
0x34  16      u4      unsigned int128

# 64-79: U
0x40  2       U0      complex unsigned int8
0x41  4       U1      complex unsigned int16
0x42  8       U2      complex unsigned int32
0x43  16      U3      complex unsigned int64
0x44  32      U4      complex unsigned int128

# 80-95: f
0x50  1       f0      float8(**)
0x51  2       f1      float16(**)
0x52  4       f2      float32
0x53  8       f3      float64

# 96-111: F
0x60  2       F0      complex float8
0x61  4       F1      complex float16
0x62  8       F2      complex float32
0x63  16      F4      complex float64

*:
ik: nbits = 8*2^k. 
  i0 = int 8 bit, 
  i4 = 8*2^4 = int 128 bit

**:
for example cuda float 16 bit half.
```