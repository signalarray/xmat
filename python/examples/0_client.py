import common
import sys
import numpy as np
import xmat
import xmat.benchmark


def sample_0():
	print('xmat.0_client():')

	tcps = xmat.TCPService()
	client = tcps.connection((xmat.IPLOCALHOST, xmat.PORT))

	xout = xmat.MapStreamOut.byte()
	xout['a'] = np.array([1, 2, 3, 4])
	xout.close()

	client.send(xout)
	tcps.close()


def sample_1():
	xmat.benchmark.native_tcp_client()


if __name__ == '__main__':
	sample_1()
