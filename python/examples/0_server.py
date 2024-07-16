import common
import sys
import xmat
import xmat.benchmark


def sample_0():
	print('xmat.0_server():')

	tcps = xmat.TCPService()
	listener = tcps.listener((xmat.IPHOST, xmat.PORT))
	conn = tcps.accept(listener)
	print('connection accepted:')

	xin = conn.recv()
	print('received xin:\n', xin)

	tcps.close()


def sample_1():
	xmat.benchmark.native_tcp_server()


if __name__ == '__main__':
	sample_1()
