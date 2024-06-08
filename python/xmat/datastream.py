"""
for typing:
https://mypy.readthedocs.io/en/stable/cheat_sheet_py3.html#variables

comments:
https://sphinxcontrib-napoleon.readthedocs.io/en/latest/example_numpy.html

numpy dtype format and codes
https://numpy.org/doc/stable/reference/arrays.dtypes.html#specifying-and-constructing-data-types
"""
import os
import struct
from sys import byteorder
from typing import Union, Any
from abc import ABC, abstractmethod
from enum import Enum
from itertools import chain
from collections import namedtuple
from pathlib import Path
import warnings

import numpy as np
import scipy.stats


class Endian(Enum):
	LITTLE = '<'
	BIG = '>'
	NATIVE = BIG if byteorder == 'big' else LITTLE
	NONNATIVE = BIG if NATIVE == LITTLE else LITTLE
	NOTAPP = '|'

	@staticmethod
	def make(value):
		if value not in ('<', '>', '=', '|'):
			raise ValueError(f"xmat.Enum.make(): wrong value {value}")
		if value == '=':
			value = Endian.NATIVE.value
		return Endian(value)

	@staticmethod
	def change(endian):
		if endian not in (Endian.LITTLE, Endian.BIG):
			raise ValueError('xmat.Endian.other(): expect only Endian.LITTLE, Endian.BIG')
		return Endian.LITTLE if endian == Endian.BIG else Endian.BIG


# types supported
# /////////////////////////////////////////////////
DTYPES_SCALAR_NATIVE = (int, float, complex, bool)

_DTypeNative = namedtuple('_DataTypeNative', 'type numpytype format itemsize')
"""
Attributes:
type: class type
format: str
	docs.python.org/3/library/struct.html#format-characters
size: int
	size in bytes 
"""

DTYPES_SCALAR_NATIVE_TABLE = {
	int: _DTypeNative(int, np.int64, 'q', 8),
	float: _DTypeNative(float, np.float64, 'd', 8),
	complex: _DTypeNative(complex, np.complex128, '', 16),
	bool: _DTypeNative(bool, np.bool_, '?', '1')
}

DTYPES_STR = (str, bytes, bytearray)

DTYPES_SCALAR_NUMPY = (
	np.bytes_,    np.bool_,
	np.int8,      np.int16,     np.int32,  np.int64,
	np.uint8,	    np.uint16,    np.uint32, np.uint64,
	np.float32,	  np.float64,
	np.complex64,	np.complex128,
)

_ReadType = namedtuple(
	'_ReadType',
	('bytes_ bool int8 int16 int32 int64 uint8 uint16 uint32 uint64 float32 float64 complex64 complex128'
		' str bytes bytearray')
)

_xtype_args = chain((np.dtype(_) for _ in DTYPES_SCALAR_NUMPY), DTYPES_STR)

xtype = _ReadType(*_xtype_args)
""" DStream.read(dtype) arguments """


def to_array(x_) -> np.array:
	"""
	Parameters:
		x_: iterable
	"""
	iter_ = iter(x_)
	sample_ = next(iter_)
	type_ = type(sample_)
	if not isinstance(sample_, DTYPES_SCALAR_NATIVE):
		raise TypeError(
			f'xmat.DStream.pack(x, ..): x is iterable with wrong item-type: {type(sample_)}')
	dtype_ = DTYPES_SCALAR_NATIVE_TABLE[type_].numpytype
	return np.fromiter(chain((sample_,), iter_), dtype_)


# binary-data-stream
# ------------------
class DStream(ABC):

	def __init__(self, mode: str, endian: Endian = Endian.NATIVE):
		"""
		Parameters
		----------
		mode: str {'w', 'r'}
		"""
		self.mode = mode
		self.endian = endian
		self._is_open = True

	def __enter__(self):
		return self

	def __exit__(self, exc_type, exc_val, exc_tb):
		self.close()

	@property
	def is_open(self) -> bool:
		return self._is_open

	def writable(self) -> bool:
		return self.mode == 'w'

	def readable(self) -> bool:
		return self.mode == 'r'

	def close(self):
		if self._is_open:
			self._close()
		self._is_open = False

	@abstractmethod
	def _close(self):
		pass

	@abstractmethod
	def tell(self):
		pass

	@abstractmethod
	def seek(self, offset, whence):
		"""
		Parameters
		----------
		offset: The offset from the beginning of the file.
		whence:
			direction:
				os.SEEK_SET or 0: absolute file positioning
				os.SEEK_CUR or 1: seek relative to the current position
				os.SEEK_END or 2: seek relative to the file’s end
		"""
		pass

	def size(self) -> int:
		"""
		Returns
		-------
		int: number of bytes
		"""

		pos = self.tell()
		self.seek(0, 2)
		n = self.tell()
		self.seek(pos, 0)
		return n

	def write(self, x, move: bool = False):
		"""
		Parameters
		----------
		x:
			supported types:
				numpy.ndarray
				DTYPES_STR
				DTYPES_SCALAR_NUMPY
				DTYPES_SCALAR_PY
		move: bool
			can be modified for swapbytes()

		Examples
		--------
		ods = xmat.DStream_(...)
		ods.write(numpy.array([1, 2, 3], dtype=float))
		ods.write("asdas")
		ods.write(b"asdas")
		ods.write(numpy.int32(2**17))
		ods.write(100)
		ods.write(3.14j)

		# will fails
		ods.write([1, 2, 3])
		ods.write((1.0j, 2.0j, 3.0j))
		ods.write(["line0", "line1"]) # fail
		"""
		return self._write_bytes(self.pack(x, move))

	def pack(self, x, move: bool = False):
		""" implementation for write(..)
		"""
		if not self.writable():
			raise RuntimeError("xmat.DStream.write(..): stream isn't writable")

		if isinstance(x, np.ndarray):
			# numpy.ndarray
			# -------------
			if x.dtype not in DTYPES_SCALAR_NUMPY:
				raise TypeError(f"xmat.DStream.write(..): x is numpy.ndarray. unsupported dtype: {x.dtype}")
			if Endian.make(x.dtype.byteorder) != self.endian:
				x = x.byteswap(inplace=move)
			return x.tobytes()

		elif isinstance(x, DTYPES_SCALAR_NUMPY):
			# numpy scalar types
			# ------------------
			if Endian.make(x.dtype.byteorder) != self.endian:
				x = x.byteswap(inplace=move)
			return x.tobytes()

		elif isinstance(x, DTYPES_SCALAR_NATIVE):
			# python native-numeric-times
			# --------------------
			if type(x) is complex:
				format = self.endian.value + 'd'
				buf = struct.pack(format, x.real, x.imag)
			else:
				format = self.endian.value + DTYPES_SCALAR_NATIVE_TABLE[type(x)].format
				buf = struct.pack(format, x)
			return buf

		elif isinstance(x, DTYPES_STR):
			# python built-in strings and bytes
			# --------------------
			if isinstance(x, str):
				return x.encode('ascii')
			else:
				return x

		elif hasattr(x, '__iter__'):
			# python iterable native-numeric-types
			# -------------
			return self.pack(to_array(x), move)

		else:
			raise ValueError("xmat.DStream.write(A): unsupported type: {}".format(type(x)))

	def read(self, dtype: Union[type, np.dtype], numel: int) -> Any:
		"""
		Parameters
		----------
		numel: int
			if numel := 0 returns scalar
			if dtype in (str, bytes, bytearray): `numel` must be grater 0

		dtype: type | np.dtype | xtype
			if dtype is type: it can be: DTYPES_STR
			if dtype is np.dtype it can be: like - DTYPES_SCALAR_NUMPY

		Returns
		-------
		if dtype in DTYPES_STR -> dtype
		else -> numpy.ndarray<dtype, 1d>

		Examples
		--------
		y = ids.read()
		"""

		if not self.readable():
			raise RuntimeError("xmat.DStream.write(..): stream isn't readable")

		if dtype in DTYPES_SCALAR_NUMPY:
			# numpy.array or scalar
			buf = self._read_bytes(dtype.itemsize * numel)
			y = np.frombuffer(buf, dtype, numel)

			if Endian.make(dtype.byteorder) != self.endian:
				y = y.byteswap(inplace=True)
			return y

		elif dtype in DTYPES_STR:
			# built-in string-like types
			if numel < 1:
				raise ValueError(f'xmat.DStream.read(.., numel): `numel` must be > 0 if dtype is: {dtype}')

			buf = self._read_bytes(numel)
			if dtype == str:
				y = buf.decode('ascii')
				return y
			else:
				return buf

		else:
			raise TypeError(f"xmat.DStream.read(..): unspported `dtype`: {dtype}")

	def read_num(self, dtype: np.dtype):
		if not self.readable():
			raise RuntimeError("xmat.DStream.write(..): stream isn't readable")

		if dtype in DTYPES_SCALAR_NUMPY:
			# numpy.array or scalar
			buf = self._read_bytes(dtype.itemsize)
			y = np.frombuffer(buf, dtype, 1)

			if not Endian.make(dtype.byteorder) in (self.endian, Endian.NOTAPP):
				y = y.byteswap(inplace=True)

			return y[0]

		else:
			raise TypeError(f"xmat.DStream.read_num(..): wrong dtype: {dtype}")

	@abstractmethod
	def _write_bytes(self, buf: bytes) -> None:
		pass

	@abstractmethod
	def _read_bytes(self, n) -> bytearray:
		"""
		Note:
			must return especially bytearray, but not bytes. Because bytes don't provide `writable` option
			to array constructed from it
		"""
		pass


class DStreamFile(DStream):
	"""	www.w3schools.com/python/python_ref_file.asp
	"""

	def __init__(self, filename: Union[str, Path], mode: str):
		"""
		Parameters
		----------
		filename: str or pathlib.Path
		mode: {'w', 'r'}
		"""

		DStream.__init__(self, mode)
		self.file = open(filename, mode + 'b')

	@staticmethod
	def out(filename: Union[str, Path]):
		return DStreamFile(filename, 'w')

	@staticmethod
	def in_(filename: Union[str, Path]):
		return DStreamFile(filename, 'r')

	def _close(self):
		return self.file.close()

	def tell(self):
		return self.file.tell()

	def seek(self, offset, whence):
		return self.file.seek(offset, whence)

	def _write_bytes(self, buf):
		self.file.write(buf)

	def _read_bytes(self, n):
		return bytearray(self.file.read(n))


class DStreamByte(DStream):

	def __init__(self, mode: str):
		"""
		Parameters
		----------
		mode: str {'w', 'r'}
		"""
		DStream.__init__(self, mode)
		self.buf = bytearray()
		self.cursor: int = 0

	@staticmethod
	def out():
		return DStreamByte('w')

	@staticmethod
	def in_():
		return DStreamByte('r')

	def set_buffer(self, buf: Union[bytes, bytearray]):
		if not self.readable():
			raise RuntimeError('xmat.DStreamByte.set_buffer(buf): stream must be readable')
		if not isinstance(buf, (bytes, bytearray)):
			raise RuntimeError(f'xmat.DStreamByte.set_buffer(buf): wrong buf type {type(buf)}')
		self.buf = buf
		self.cursor = 0

	def push_buffer(self, buf: Union[bytes, bytearray]):
		if not self.readable():
			raise RuntimeError('xmat.DStreamByte.set_buffer(buf): stream must be readable')
		if not isinstance(buf, (bytes, bytearray)):
			raise RuntimeError(f'xmat.DStreamByte.set_buffer(buf): wrong buf type {type(buf)}')
		self.buf.extend(buf)

	def _close(self):
		pass

	def tell(self):
		return self.cursor

	def seek(self, offset, whence):
		_cursor = self.cursor
		if whence == 0: 		# from the beginning
			self.cursor = offset
		elif whence == 1:		# current position
			self.cursor += offset
		elif whence == 2:		# from end of file
			self.cursor = len(self.buf) - offset
		else:
			raise ValueError("xmat.DStreamByte.seek(.., whence): `whence` wrong value")

		status = self.cursor >= 0 & self.cursor <= len(self.buf)
		if not status:
			self.cursor = _cursor
			raise ValueError("xmat.DStreamByte.seek(..): out of bounds")

	def size(self):
		return len(self.buf)

	def _write_bytes(self, buf):
		if self.cursor == len(buf):
			self.buf.extend(buf)
		elif self.cursor + len(buf) > len(self.buf):
			self.buf += b'\0' * (self.cursor + len(buf) - len(self.buf))

		self.buf[self.cursor:self.cursor + len(buf)] = buf
		self.cursor += len(buf)

	def _read_bytes(self, n):
		if self.cursor + n > self.size():
			raise ValueError('xmat.DStreamByte.read(n): `n` exceeds buffer size')

		buf = self.buf[self.cursor:self.cursor+n]
		self.cursor += n
		return buf


# -------------------------------
# xmat format
# -------------------------------
SIGNATURE = b'xmat'
SIGN_SIZE = len(SIGNATURE)
BOM = np.int16(1)
SIZEOF_XSIZE_T = np.uint8(8)
MAX_NDIM = np.uint8(8)
MAX_NAME = np.uint8(32)


DTYPEOUT = {
	np.uint8(int('0x01', 0)): xtype.bytes,
	np.uint8(int('0x02', 0)): xtype.bool,

	np.uint8(int('0x10', 0)): xtype.int8,
	np.uint8(int('0x11', 0)): xtype.int16,
	np.uint8(int('0x12', 0)): xtype.int32,
	np.uint8(int('0x13', 0)): xtype.int64,

	np.uint8(int('0x20', 0)): xtype.uint8,
	np.uint8(int('0x21', 0)): xtype.uint16,
	np.uint8(int('0x22', 0)): xtype.uint32,
	np.uint8(int('0x23', 0)): xtype.uint64,

	np.uint8(int('0x52', 0)): xtype.float32,
	np.uint8(int('0x53', 0)): xtype.float64,

	np.uint8(int('0x62', 0)): xtype.complex64,
	np.uint8(int('0x63', 0)): xtype.complex128,
}


DTYPEIN = {type_: tid for tid, type_ in DTYPEOUT.items()}


class XHead:
	def __init__(self):
		self.sign: bytes = SIGNATURE										# Sign
		self.bom: np.uint16 = BOM												# BOM
		self.total: np.uint64 = np.uint64(0)						# Total Size
		self.sizeof_int: np.uint8 = SIZEOF_XSIZE_T			# I
		self.maxndim: np.uint8 = MAX_NDIM								# S
		self.maxname: np.uint8 = MAX_NAME								# B

	def dump(self, ods: DStream):
		if len(self.sign) != SIGN_SIZE:
			raise ValueError(f'xmat.XHead.dump(..): wrong `signature`: {self.sign}')

		ods.write(self.sign)
		ods.write(np.uint16(self.bom))
		ods.write(np.uint64(self.total))
		ods.write(np.uint8(self.sizeof_int))
		ods.write(np.uint8(self.maxndim))
		ods.write(np.uint8(self.maxname))

	def load(self, ids: DStream):
		self.sign = ids.read(xtype.bytes, SIGN_SIZE)
		if self.sign != SIGNATURE:
			raise ValueError(f'xmat.XHead.load(..): wrong `signature`: {self.sign}')

		self.bom: np.uint16 = ids.read_num(xtype.uint16)
		if self.bom != BOM:
			ids.endian = Endian.change(ids.endian)
			bom_ = self.bom.byteswap()
			if bom_ != BOM:
				raise ValueError(f'xmat.XHead.load(..): wrong `bom`: {self.bom}|{bom_}')
			warnings.warn(f'xmat.XHead.load():  wrong endian value {Endian.change(ids.endian)} changed to: {ids.endian}')
			self.bom = bom_

		self.total = ids.read_num(xtype.uint64)
		self.sizeof_int = ids.read_num(xtype.uint8)
		self.maxndim = ids.read_num(xtype.uint8)
		self.maxname = ids.read_num(xtype.uint8)

		if self.sizeof_int != SIZEOF_XSIZE_T:
			raise ValueError(f'xmat.XHead.load(..): wrong `size of int` value: {self.sizeof_int}')

	@staticmethod
	def nbytes():
		return 17

	def __repr__(self):
		w = 24
		s = "mat.XHead:\n" + \
			f"{'sign':>{w}}: {self.sign.decode()}\n" + \
			f"{'bom':>{w}}: {self.bom.tobytes()}\n" + \
			f"{'total':>{w}}: {self.total}\n" + \
			f"{'sizeof_int':>{w}}: {self.sizeof_int}\n" + \
			f"{'maxdim':>{w}}: {self.maxndim}\n" + \
			f"{'maxname':>{w}}: {self.maxname}\n"
		return s


class XBlock:
	def __init__(self):
		self.pos: int = 0

		self.morder: str = 'C'														# o
		self.tid: np.uint8 = np.uint8(0)								# t
		self.ndim: np.uint8 = np.uint8(0)								# s
		self.namelen: np.uint8 = np.uint8(0)						# b
		self.shape: tuple[np.uint64] = tuple()					# Shape[ndim]
		self.name: str = ''															# Block Name[namelen]

	def dumpvar(self, x, name, ods: DStream = None, numpy_move = False):
		self.namelen = len(name)
		self.name = name

		if isinstance(x, np.ndarray):
			# numpy.ndarray
			# -------------
			if x.dtype not in DTYPES_SCALAR_NUMPY:
				raise TypeError(
					f"xmat.DStream.write(..): x is numpy.ndarray. unsupported dtype: {x.dtype}")

			self.tid = DTYPEIN[x.dtype]
			self.ndim = x.ndim
			self.shape = x.shape

		elif isinstance(x, DTYPES_SCALAR_NUMPY):
			# numpy scalar types
			# ------------------
			self.tid = DTYPEIN[np.dtype(type(x))]
			self.ndim = np.uint8(0)
			self.shape = x.shape

		elif isinstance(x, DTYPES_SCALAR_NATIVE):
			# numpy scalar types
			# ------------------
			type_ = np.dtype(DTYPES_SCALAR_NATIVE_TABLE[type(x)].numpytype)
			self.tid = DTYPEIN[type_]
			self.ndim = np.uint8(0)
			self.shape = 0

		elif isinstance(x, DTYPES_STR):
			# str-like: str is supposed the only ascii encoding string
			self.tid = DTYPEIN[xtype.bytes]
			self.ndim = np.uint8(1)
			self.shape = (len(x),)

		elif hasattr(x, '__iter__'):
			self.dumpvar(to_array(x), name, ods, numpy_move=True)
			return

		else:
			raise ValueError("xmat.DStream.write(A): unsupported type: {}".format(type(x)))

		if ods:
			self.dump(ods)
			ods.write(x, numpy_move)

	def dump(self, ods: DStream):
		# raise ValueError(f'xmat.XBlock.dump')
		if self.morder not in 'CF':
			raise ValueError(f'xmat.XBlock.dump(): wrong `morder`: {self.morder}')
		if self.tid not in DTYPEOUT:
			raise ValueError(f'xmat.XBlock.dump(): wrong `tid`: {self.tid}')
		# if self.ndim == len(self.shape):
		# 	raise ValueError(f'xmat.XBlock.dump(): wrong `ndim`')
		if self.namelen != len(self.name):
			raise ValueError(f'xmat.XBlock.dump(): wrong `namelen`')

		ods.write(self.morder)
		ods.write(np.uint8(self.tid))
		ods.write(np.uint8(self.ndim))
		ods.write(np.uint8(self.namelen))
		ods.write(b'\0\0\0\0')
		if self.ndim != 0:
			ods.write(np.array(self.shape, dtype=np.uint64))
		if self.namelen != 0:
			ods.write(self.name)

	def load(self, ids: DStream):
		self.pos = ids.tell()

		self.morder = ids.read(xtype.str, 1)
		self.tid = ids.read_num(xtype.uint8)
		self.ndim = ids.read_num(xtype.uint8)
		self.namelen = ids.read_num(xtype.uint8)
		space_ = ids.read(xtype.bytes, 4)
		if space_ != b'\0\0\0\0':
			raise ValueError(f'xmat.XBlock.load(): wrong zeros-space: {space_}')
		self.shape = ids.read(xtype.uint64, self.ndim)
		self.name = ids.read(xtype.str, self.namelen)

	def typesize(self) -> int:
		return 1 if DTYPEOUT[self.tid] is xtype.bytes else DTYPEOUT[self.tid].itemsize

	def nbytes(self) -> int:
		return 8 + SIZEOF_XSIZE_T * self.ndim + self.namelen

	def data_pos(self) -> int:
		return self.pos + self.nbytes()

	def data_nbytes(self) -> int:
		return self.typesize() * int(self.numel())

	def numel(self) -> int:
		return int(np.prod(self.shape))

	def __str__(self):
		return self.__format__('1')

	def __format__(self, format_spec):
		w = int(format_spec)
		return f"{self.name:>{w}}: id: 0x{self.tid:02x} {str(DTYPEOUT[self.tid]):>16} {self.morder}:{self.shape}"

	def __repr___(self):
		s = (
			f" morder: {self.morder}\n"
			f"    tid: 0x{self.tid:02x}\n"
			f"   ndim: {self.ndim}\n"
			f"namelen: {self.namelen}\n"
			f"  shape: {self.shape}\n"
			f"   name: {self.name}\n"
		)
		return s


# -------------
# map-stream
# -------------
class MapStream(ABC):
	def __init__(self, stream: DStream):
		self.head: XHead = XHead()
		self.dstream: DStream = stream
		self.morder: str = 'C'
		self.flipshape: bool = False

	def __del__(self):
		if self.is_open:
			self.close()

	def close(self):
		self.dstream.close()

	@property
	def endian(self) -> Endian:
		return self.dstream.endian

	@property
	def is_open(self) -> bool:
		return self.dstream.is_open


class MapStreamOut(MapStream):
	"""
	Examples:
	xout = xmat.MapStreamOut.byte()
	xout['vector'] = np.array([1, 2, 3])
	xout['string'] = "string variable"
	xout.close()
	"""

	def __init__(self, stream: DStream):
		if not stream.writable():
			raise ValueError(f'xmat.MapStreamOut(): wrong stream.mode: must be `writable`')

		MapStream.__init__(self, stream)
		self.names_list: list[str] = []

		self.head.dump(self.dstream)

	@staticmethod
	def file(filename: Union[str, Path], endian: Endian = Endian.NATIVE):
		xout = DStreamFile.out(filename)
		xout.endian = endian
		return MapStreamOut(xout)

	@staticmethod
	def byte(endian: Endian = Endian.NATIVE):
		xout = DStreamByte.out()
		xout.endian = endian
		return MapStreamOut(xout)

	def close(self):
		if not self.is_open:
			return

		self.head.total = type(self.head.total)(self.dstream.size())
		self.dstream.seek(6, 0)
		self.dstream.write(self.head.total)
		self.dstream.close()

	def setitem(self, name: str, x, npmove=False):
		if name in self.names_list:
			raise ValueError(f"xmat.MapStreamOut.setitem(name, ...): name '{name}' is already in")
		XBlock().dumpvar(x, name, self.dstream, npmove)

	def __setitem__(self, key, value):
		return self.setitem(key, value)


class MapStreamIn(MapStream):
	"""
	Attributes
	----------
	encoding: str {'', 'ascii', 'utf-8', ...}
		if encoding != '' -> decode all bytes in stream to str with encoding. buf.decode(encoding) -> str
		docs.python.org/3/library/codecs.html#standard-encodings

	Examples:
	xin = xmat.MapStreamIn.byte()
	xin.push_buffer(--bytes--)
	check = 'vector' in xin
	vector = xin['vector']
	check = 'string' in xin
	xin.close()
	"""
	def __init__(self, stream: DStream, ready_for_scan: bool = True):
		MapStream.__init__(self, stream)

		if not self.dstream.readable():
			raise ValueError(f'xmat.MapStreamIn(): wrong stream.mode: must be `readable`')

		self.encoding: str = ''

		self.map = dict()
		if ready_for_scan:
			self.scan()

	@staticmethod
	def file(filename: Union[str, Path], endian: Endian = Endian.NATIVE):
		xin = DStreamFile.in_(filename)
		xin.endian = endian
		return MapStreamIn(xin, True)

	@staticmethod
	def byte(buf: bytes = None, endian: Endian = Endian.NATIVE):
		xin = DStreamByte.in_()
		xin.endian = endian
		if buf is None:
			return MapStreamIn(xin, False)
		else:
			xin.push_buffer(buf)
			return MapStreamIn(xin, True)

	def push_buffer(self, buf):
		if isinstance(self.dstream, DStreamByte):
			return self.dstream.push_buffer(buf)
		else:
			raise ValueError(f"xmat.MapStreamIn.push_buffer(buf): dstream must be a xmat.DStreamByte")

	def getitem(self, name: str):
		if name not in self.map:
			raise KeyError(f"xmat.MapStreamIn.getitem(name): name: `{name}` not in")

		b: XBlock = self.map[name]
		type_ = DTYPEOUT[b.tid]
		self.dstream.seek(b.data_pos(), os.SEEK_SET)

		if type_ in DTYPES_SCALAR_NUMPY:
			# numpy type
			if b.ndim == 0:
				return self.dstream.read_num(type_)
			else:
				y: np.ndarray = self.dstream.read(type_, b.numel())
				y.reshape(b.shape, order=b.morder)
				return y

		elif type_ is xtype.bytes:
			# load string
			y: bytes = self.dstream.read(type_, b.numel())
			if self.encoding:
				return y.decode(self.encoding)
			else:
				return y

		else:
			raise ValueError(f"unexpected DTYPE[xblock.tid]: {type_}")

	def __getitem__(self, name):
		return self.getitem(name)

	def getall(self):
		out = {name: self.getitem(name) for name in self.keys()}

	def __contains__(self, item: str):
		return item in self.map

	def keys(self):
		# all = {name: xin.getitem(name) for name in self.keys()}
		return self.map.keys()

	# scan and load blocks
	def scan(self):
		self.scan_head()
		self.scan_data()

	def scan_head(self):
		self.head.load(self.dstream)
		if self.head.total < self.head.nbytes():
			raise ValueError(f"xmat.MapStreamIn.scan_head(): wrong self.head.total: {self.head.total}")

	def scan_data(self):
		while self.dstream.tell() < self.head.total:
			b = XBlock()
			b.load(self.dstream)
			self.map[b.name] = b
			self.dstream.seek(b.data_nbytes(), os.SEEK_CUR)

		if self.dstream.tell() != self.head.total:
			raise ValueError(f"xmat.MapStreamIn.scan_data(): wrong dstream size. may be corrupted")

	def __repr__(self):
		maxname = max(len(name) for name in self.map.keys()) + 2
		s = f"xmat.MapStreamIn:\n{self.head}\n" + \
				'blocks:\n' + \
				''.join(f'[{n:>4}] {_: {maxname}}\n' for n, _ in enumerate(self.map.values()))
		return s
