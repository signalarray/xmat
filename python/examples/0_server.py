import common
import sys
import xmat


def sample_0():
	print('xmat.0_server():')

	tcps = xmat.TCPService()
	listener = tcps.listener((xmat.IPHOST, xmat.PORT))
	conn = tcps.accept(listener)
	print('connection accepted:')

	xin = conn.recv()
	print('received xin:\n', xin)

	tcps.close()


def main():
	num = int(sys.argv[1])
	f = (sample_0, )
	f[num]()


if __name__ == '__main__':
	main()
