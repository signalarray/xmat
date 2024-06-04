from collections import namedtuple

import numpy as np

import xmat
from xmat import xtype
from common import temp_data_folder


def samples_0():
	dt0 = np.dtype('int')
	print(dt0)
	print(dt0 == np.int32)
	print(np.int_ == np.int32)

	print(xmat.datastream.Endian.NATIVE)
	print(xmat.datastream.Endian.NONNATIVE)


def samples_1():
	x = np.array([1, 2])
	print("x.flags['C']: {0}".format(x.flags['C']))
	print("x.flags['F']: {0}".format(x.flags['F']))
	print("x.flags['O']: {0}".format(x.flags['O']))
	print("x.flags['W']: {0}".format(x.flags['W']))
	print("x.flags['A']: {0}".format(x.flags['A']))
	print("x.flags['X']: {0}".format(x.flags['X']))
	print("x.flags['FNC']: {0}".format(x.flags['FNC']))
	print("x.flags['FORC']: {0}".format(x.flags['FORC']))
	print("x.flags['CA']: {0}".format(x.flags['CA']))
	print("x.flags['FA']: {0}".format(x.flags['FA']))

	print('------------')
	print('x.nbytes  : ', x.nbytes)
	print('x.shape   : ', x.shape)
	print('x         : ', x.tobytes())
	print('x.byteswap: ', x.byteswap().tobytes())


def sample_2():
	x0 = np.array(range(4), dtype='i2')
	x1 = x0[1::2]
	print(x0)
	print(x1)

	ods = xmat.DStreamByte('w')
	print(ods.tobytes(x0))
	print(ods.tobytes(x1))
	print(ods.tobytes(1))


def sample_3():
	print('xmat.DStreamByte')
	print(xtype.bytearray)
	print(xtype.int8)

	ods = xmat.DStreamByte.out()
	ods.write(np.int32(1))

	ids = xmat.DStreamByte.in_()
	ids.set_buffer(ods.buf)
	i0 = ids.read(xtype.int16)
	i1 = ids.read(np.dtype('i2'))
	print(i0)
	print(i1)


def sample_4():
	print(' xmat.DStreamFile')

	x0 = np.array(range(4), dtype='i2')

	folder_data = temp_data_folder
	filename = folder_data.joinpath('py_0.xmat')

	ods = xmat.DStreamFile.out(filename)
	ods.write(x0)
	ods.close()

	ids = xmat.DStreamFile.in_(filename)
	i0 = ids.read(xtype.int32)
	i1 = ids.read(xtype.int32)
	print(i0)
	print(i1)


def sample_5():
	print('xmat.XHead:')

	ods = xmat.DStreamByte.out()
	h = xmat.XHead()
	h.total = 1024
	print(h)
	h.dump(ods)

	ids = xmat.DStreamByte.in_()
	ids.set_buffer(ods.buf)
	h2 = xmat.XHead()
	print('h2 default: \n', h2)

	h2.load(ids)
	print('h2 load: \n', h2)


def sample_6():
	print('xmat.XBlock:')

	x = np.array(range(8))
	b0 = xmat.XBlock(np.int32(1))
	print(b0)
	b0 = xmat.XBlock(x)
	print(b0)
	b0 = xmat.XBlock(x[::2])
	print(b0)
	b0 = xmat.XBlock('string')
	print(b0)

	b0 = xmat.XBlock(np.uint16(31), 'specific-name')
	print('\n repr scalar:\n', b0.__repr___())


def sample_7():
	print('xmat.MapStreamOut')

	x = np.array(range(8))

	xout = xmat.MapStreamOut.byte()
	xout['a'] = x
	xout['long-name'] = np.int64(1024)
	xout.close()

	xin = xmat.MapStreamIn.byte(xout.dstream.buf)
	print(xin)
	a = xin['a']
	print(a)
	b = xin['long-name']
	print(b)


def sample_8():
	print('xmat.MapStream_.file')

	folder_data = temp_data_folder
	filename = folder_data.joinpath('py_0.xmat')

	x = np.array(range(8))

	xout = xmat.MapStreamOut.file(filename)
	xout['a'] = x
	xout['long-name'] = np.int64(1024)
	xout.close()

	xin = xmat.MapStreamIn.file(filename)
	print(xin)
	a = xin['a']
	print(a)
	b = xin['long-name']
	print(b)


def main():
	sample_8()


if __name__ == '__main__':
	main()
