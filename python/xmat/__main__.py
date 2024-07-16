"""
call command:
python -m xmat native_tcp_server
"""

from . import benchmark


if __name__ == '__main__':
	print('xmat.benchmark:\n--------------\n')
	import sys
	if len(sys.argv) == 1:
		print('commands:')
		print('native_tcp_server',
				'native_tcp_client [ip-server]',
				'xmat_tcp_server',
				'xmat_tcp_client [[dtype: {float, complex, int, ...}], ip-server]', sep='\n')
		exit(0)

	command = sys.argv[1]
	if command == 'native_tcp_server':
		benchmark.native_tcp_server()
	elif command == 'native_tcp_client':
		benchmark.native_tcp_client(*sys.argv[2:])
	elif command == 'xmat_tcp_server':
		benchmark.xmat_tcp_server()
	elif command == 'xmat_tcp_client':
		benchmark.xmat_tcp_client(*sys.argv[2:])
	else:
		print('error: wrong command')
		exit(1)
