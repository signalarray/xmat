import time
from importlib.util import find_spec

import numpy as np
from . import datastream
from . import tcpsocket

_MESSAGE_REPEAT = 32
_SIZE_RANGE = [10, 20]
_MSG_SIZE = tuple(2**k for k in range(_SIZE_RANGE[0], _SIZE_RANGE[1] + 1))
_IP_PORT = tcpsocket.PORT


def native_tcp_server():
	print('benchmark.native_tcp_server')
	N = len(_MSG_SIZE)
	listener = tcpsocket.TCPListener((tcpsocket.IPHOST, _IP_PORT), 1)
	connection = listener.accept()

	t = np.zeros((N, _MESSAGE_REPEAT), dtype=float)
	legend_ = []
	for n in range(N):
		msg_size = _MSG_SIZE[n]
		legend_.append(f'm:{msg_size:12d}')
		print('new message size: ', msg_size)
		for m in range(_MESSAGE_REPEAT):
			t0 = time.time()
			data = connection.recv_(msg_size)
			t[n, m] = time.time() - t0
			#  print content just for verification
			print('msg: ', len(data), 'data[-8::]', data[-8::])

		print(f'finish message size: {msg_size/2**10} Kb\n, time: {np.mean(t[n, 2:]):3.6f}\n')
	plot_result(t, 'native-server', legend_)


def native_tcp_client(ipaddress: str = tcpsocket.IPLOCALHOST):
	print('benchmark.native_tcp_client')
	N = len(_MSG_SIZE)
	datatotal = bytes(n % 256 for n in range(_MSG_SIZE[-1] + _MESSAGE_REPEAT + N))
	print('datatotal is done:')

	connection = tcpsocket.TCPConnection((ipaddress, _IP_PORT))
	t = np.zeros((N, _MESSAGE_REPEAT), dtype=float)
	legend_ = []
	for n in range(N):
		msg_size = _MSG_SIZE[n]
		legend_.append(f'm:{msg_size:12d}')
		print('new message size: ', msg_size)
		for m in range(_MESSAGE_REPEAT):
			t0 = time.time()
			connection.send_(datatotal[n+m:n+m+msg_size])
			t[n, m] = time.time() - t0
			#  print content just for verification
			print('data[-8::]', datatotal[n+m+msg_size-8:n+m+msg_size])

		print(f'finish message size: {msg_size/2**10} Kb\n, time: {np.mean(t[n, 2:]):3.6f}\n')
	plot_result(t, 'native-client', legend_)


# xmat - format tcp - transfer: numeric elements
# ----------------------------------------------
def xmat_tcp_server():
	print('benchmark.native_tcp_server')
	N = len(_MSG_SIZE)
	listener = tcpsocket.TCPListener((tcpsocket.IPHOST, _IP_PORT), 1)
	connection = listener.accept()

	t = np.zeros((N, _MESSAGE_REPEAT), dtype=float)
	legend_ = []
	for n in range(N):
		msg_size = _MSG_SIZE[n]
		legend_.append(f'm:{msg_size:12d}')
		print('new message size: ', msg_size)
		for m in range(_MESSAGE_REPEAT):
			t0 = time.time()
			xin = connection.recv()
			data = xin.getitem('data')
			t[n, m] = time.time() - t0
			#  print content just for verification
			print('msg: ', len(data), 'data[-8::]', data[-8::])

		print(f'finish message size: {msg_size/2**10} K\n, time: {np.mean(t[n, 2:]):3.6f}\n')
	plot_result(t, 'native-server', legend_)


def xmat_tcp_client(dtype: str = 'float', ipaddress: str = tcpsocket.IPLOCALHOST):
	print('benchmark.xmat_tcp_client')
	N = len(_MSG_SIZE)
	# datatotal = bytes(n % 256 for n in range(_MSG_SIZE[-1] + _MESSAGE_REPEAT + N))
	datatotal = np.arange(_MSG_SIZE[-1] + _MESSAGE_REPEAT + N, dtype=dtype)

	print('datatotal is done:')

	connection = tcpsocket.TCPConnection((ipaddress, _IP_PORT))
	t = np.zeros((N, _MESSAGE_REPEAT), dtype=float)
	legend_ = []
	for n in range(N):
		msg_size = _MSG_SIZE[n]
		legend_.append(f'm:{msg_size:12d}')
		print('new message size: ', msg_size)
		for m in range(_MESSAGE_REPEAT):
			t0 = time.time()
			xout = datastream.MapStreamOut.byte()
			xout.setitem('data', datatotal[n + m:n + m + msg_size])
			xout.close()
			connection.send(xout)
			t[n, m] = time.time() - t0
			#  print content just for verification
			print('data[-8::]', datatotal[n + m + msg_size - 8:n + m + msg_size])

		print(f'finish message size: {msg_size/2**10} K\n, time: {np.mean(t[n, 2:]):3.6f}\n')
	plot_result(t, 'native-client:' + dtype, legend_)


def plot_result(t, figure_name, legend):
	s = figure_name + ' summary: \n'
	ss = [f'{_MSG_SIZE[n]/2**10: 12.0f}K: {np.mean(t[n, 2:]): 4.9f} sec\n' for n in range(t.shape[0])]
	print(''.join([s] + ss))

	if not find_spec('matplotlib'):
		return
	import matplotlib.pyplot as plt
	fig = plt.figure(figure_name)
	ax = fig.add_axes((0.1, 0.1, 0.8, 0.8))
	ax.plot(t[2:, :].T, marker='o')
	ax.legend(legend)
	ax.grid(True)
	plt.show()
