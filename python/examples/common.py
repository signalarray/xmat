import sys
from pathlib import Path

from importlib.util import find_spec

# import xmat.benchmark

if not find_spec('xmat'):
	print('xmat module added to search-path manually')
	sys.path.append(str(Path(__file__).parents[1]))


temp_data_folder: Path = Path(__file__).parents[2].joinpath('data')


def sample_0():
	# xmat.benchmark.native_tcp_server()
	pass


if __name__ == '__main__':
	sample_0()