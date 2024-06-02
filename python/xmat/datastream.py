"""
for typing:
https://mypy.readthedocs.io/en/stable/cheat_sheet_py3.html#variables

comments:
https://sphinxcontrib-napoleon.readthedocs.io/en/latest/example_numpy.html

numpy dtype format and codes
https://numpy.org/doc/stable/reference/arrays.dtypes.html#specifying-and-constructing-data-types
"""

from sys import byteorder
from typing import Union, Any
from abc import ABC, abstractmethod
from enum import Enum
from itertools import chain
from collections import namedtuple
from pathlib import Path
import warnings

import numpy as np


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
	def other(endian):
		if endian not in (Endian.LITTLE, Endian.BIG):
			raise ValueError('xmat.Endian.other(): expect only Endian.LITTLE, Endian.BIG')
		return Endian.LITTLE if endian == Endian.BIG else Endian.BIG


# types supported
# /////////////////////////////////////////////////
DTYPES_STR = (str, bytes, bytearray)

DTYPES_SCALAR_NUMPY = (
	np.bytes_,
	np.int8,      np.int16,     np.int32,  np.int64,
	np.uint8,	    np.uint16,    np.uint32, np.uint64,
	np.float32,	  np.float64,
	np.complex64,	np.complex128,
)

_ReadType = namedtuple(
	'_ReadType',
	('bytes_ int8 int16 int32 int64 uint8 uint16 uint32 uint64 float32 float64 complex64 complex128'
		' str bytes bytearray')
)

_xtype_args = chain((np.dtype(_) for _ in DTYPES_SCALAR_NUMPY), DTYPES_STR)

xtype = _ReadType(*_xtype_args)
""" DStream.read(dtype) arguments """


class DStream(ABC):

	def __init__(self, mode: str, endian: Endian = Endian.NATIVE):
		"""
		Parameters
		----------
		mode: str {'w', 'r'}
		"""
		self.mode = mode
		self.endian = endian

	def __enter__(self):
		return self

	def __exit__(self, exc_type, exc_val, exc_tb):
		self.close()

	def writable(self):
		return self.mode == 'w'

	def readable(self):
		return self.mode == 'r'

	@abstractmethod
	def close(self):
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

	def size(self):
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
		return self._write_bytes(self.tobytes(x, move))

	def tobytes(self, x, move: bool = False):
		""" implementation for write(..)
		"""
		if not self.writable():
			raise RuntimeError("xmat.DStream.write(..): stream isn't writable")

		if isinstance(x, np.ndarray):
			# numpy.ndarray
			if x.dtype not in DTYPES_SCALAR_NUMPY:
				raise TypeError(f"xmat.DStream.write(..): x is numpy.ndarray. unsupported dtype: {x.dtype}")

			if Endian.make(x.dtype.byteorder) != self.endian:
				x = x.byteswap(inplace=move)

			return x.tobytes()

		elif isinstance(x, DTYPES_STR):
			# python built-in strings and bytes
			if isinstance(x, str):
				return x.encode('ascii')
			else:
				return x

		elif isinstance(x, DTYPES_SCALAR_NUMPY):
			# numpy scalar types
			if Endian.make(x.dtype.byteorder) != self.endian:
				x = x.byteswap(inplace=move)
			return x.tobytes()

		else:
			raise ValueError("xmat.DStream.write(A): unsupported type: {}".format(type(x)))

	def read(self, dtype: Union[type, np.dtype], numel: int = 0) -> Any:
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
			numel_ = numel if numel > 0 else 1
			buf = self._read_bytes(dtype.itemsize * numel_)
			y = np.frombuffer(buf, dtype, numel_)

			if Endian.make(dtype.byteorder) != self.endian:
				y = y.byteswap(inplace=True)

			return y if numel > 0 else y[0]

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

	@abstractmethod
	def _write_bytes(self, buf: bytes) -> None:
		pass

	@abstractmethod
	def _read_bytes(self, n) -> bytes:
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

	def close(self):
		return self.file.close()

	def tell(self):
		return self.file.tell()

	def seek(self, offset, whence):
		return self.file.seek(offset, whence)

	def _write_bytes(self, buf):
		self.file.write(buf)

	def _read_bytes(self, n):
		return self.file.read(n)


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

	def close(self):
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

		status = not (self.cursor >= 0 & self.cursor <= len(self.buf))
		if not status:
			self.cursor = _cursor
			raise ValueError("xmat.DStreamByte.seek(..): out of bounds")

	def size(self):
		return len(self.buf)

	def _write_bytes(self, buf):
		self.buf.extend(buf)
		self.cursor = len(buf)

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


DTYPE = {
	np.uint8(int('0x01', 0)): xtype.str,

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


DTID = {type_: tid for tid, type_ in DTYPE.items()}


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

		self.bom: np.uint16 = ids.read(xtype.uint16)
		if self.bom != BOM:
			ids.endian = Endian.other(ids.endian)
			bom_ = self.bom.swapbytes()
			if bom_ != BOM:
				raise ValueError(f'xmat.XHead.load(..): wrong `bom`: {self.bom}|{bom_}')
			warnings.warn(f'xmat.XHead.load():  wrong endian value {Endian.other(ids.endian)} changed to: {ids.endian}')
			self.bom = bom_

		self.total = ids.read(xtype.uint64)
		self.sizeof_int = ids.read(xtype.uint8)
		self.maxndim = ids.read(xtype.uint8)
		self.maxname = ids.read(xtype.uint8)

		if self.sizeof_int != SIZEOF_XSIZE_T:
			raise ValueError(f'xmat.XHead.load(..): wrong `size of int` value: {self.sizeof_int}')

	@staticmethod
	def nbytes():
		return 17

	def __repr__(self):
		pass


class XBlock:
	def __init__(self):
		self.morder: bytes = b'C'												# o
		self.tid: np.uint8 = np.uint8(0)								# t
		self.ndim: np.uint8 = np.uint8(0)								# s
		self.namelen: np.uint8 = np.uint8(0)						# b
		self.shape: tuple[np.uint64] = tuple()					# Shape[ndim]
		self.name: str = ''															# Block Name[namelen]

		self.pos: int = 0

	def make(self, x, name: str = 'default'):
		# self.morder
		self.namelen = len(name)
		self.name = name

		if isinstance(x, np.ndarray):
			# numpy.ndarray
			if x.dtype not in DTYPES_SCALAR_NUMPY:
				raise TypeError(f"xmat.DStream.write(..): x is numpy.ndarray. unsupported dtype: {x.dtype}")

			self.tid = DTID[x.dtype]
			self.ndim = x.ndim
			self.shape = x.shape

		elif isinstance(x, DTYPES_STR):
			# str-like: str is supposed the only ascii encoding string
			self.tid = DTID[xtype.str]
			self.ndim = np.uint8(1)
			self.shape = (len(x), )

		elif isinstance(x, DTYPES_SCALAR_NUMPY):
			# numpy scalar types
			self.tid = DTID[type(x)]
			self.ndim = np.uint8(0)
			self.shape = (0, )

		else:
			raise ValueError("xmat.DStream.write(A): unsupported type: {}".format(type(x)))

	def dump(self, ods: DStream):
		# raise ValueError(f'xmat.XBlock.dump')
		if self.morder not in 'CF':
			raise ValueError(f'xmat.XBlock.dump(): wrong `morder`: {self.morder}')
		if self.tid not in DTYPE:
			raise ValueError(f'xmat.XBlock.dump(): wrong `tid`: {self.tid}')
		if self.ndim != len(self.shape):
			raise ValueError(f'xmat.XBlock.dump(): wrong `ndim`')
		if self.namelen != len(self.name):
			raise ValueError(f'xmat.XBlock.dump(): wrong `namelen`')

		ods.write(self.morder.decode('ascii'))
		ods.write(np.uint8(self.tid))
		ods.write(np.uint8(self.ndim))
		ods.write(np.uint8(self.namelen))
		ods.write(b'\0\0\0\0')
		ods.write(np.array(self.shape, dtype=np.uint64))
		ods.write(self.name)

	def load(self, ids: DStream):
		self.pos = ids.tell()

		self.morder = ids.read(xtype.bytes, 1)
		self.tid = ids.read(xtype.uint8)
		self.ndim = ids.read(xtype.uint8)
		self.namelen = ids.read(xtype.uint8)
		space_ = ids.read(xtype.bytes, 4)
		if space_ != b'\0\0\0\0':
			raise ValueError(f'xmat.XBlock.load(): wrong zeros-space: {space_}')
		self.shape = ids.read(xtype.uint64, self.ndim)
		self.name = ids.read(xtype.str, self.namelen)

	# --------------------------
	def typesize(self):
		pass

	def nbytes(self):
		pass

	def data_pos(self):
		pass

	def data_nbytes(self):
		pass

	def numel(self):
		pass

	def __repr__(self):
		pass
