from pathlib import Path

temp_data_folder: Path = Path(__file__).parents[2].joinpath('data')


def main():
	p = Path(__file__)
	print(p)
	print(p.parent)
	print(p.parents[2])
	print(p.name)
	print(p.stem)
	print(p.suffix)

	p1 = p.parent.joinpath('subdir')
	print(p)
	print(p1)

	print('temp_data_folder: ', temp_data_folder)

	file = open(temp_data_folder.joinpath('out.xmat'), 'w')
	file.write('asd')
	file.close()


if __name__ == '__main__':
	main()
