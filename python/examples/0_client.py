import common
import sys
import numpy as np
import xmat


def sample_0():
	print('xmat.0_client():')

	tcps = xmat.TCPService()
	client = tcps.connection((xmat.IPLOCALHOST, xmat.PORT))

	xout = xmat.MapStreamOut.byte()
	xout['a'] = np.array([1, 2, 3, 4])
	xout.close()

	client.send(xout)
	tcps.close()


def main():
	num = int(sys.argv[1])
	f = (sample_0, )
	f[num]()


if __name__ == '__main__':
	main()
