"""start
Save xmat-file
^^^^^^^^^^^^^^
save variable as a key-value pair in a file
stop"""

import pathlib
import numpy as np
import numpy.random

import xmat

# set the file name
file_out = pathlib.Path(__file__).parents[2].joinpath('data', 'example_python.xmat')

xout = xmat.MapStreamOut.file(file_out)

# floating-point numbers
xout.setitem('a', 3.14)
xout.setitem('b', [n/3 for n in range(-3, 4)])
xout.setitem('c', np.arange(2*3).reshape([2, 3]))
xout.setitem('d', np.arange(2*3*4).reshape([2, 3, 4]))
xout.setitem('e', np.arange(3**5).reshape([3, 3, 3, 3, 3]))

# integer numbers
xout.setitem('f', numpy.random.randint(0, 255, size=(2, 3), dtype=np.uint8))
xout.setitem('g', numpy.random.randint(-128, 127, size=(2, 3), dtype=np.int8))

# complex-value numbers
xout.setitem('h', np.exp(1j*2*np.pi*3*np.linspace(0, 1, 32)))
xout.setitem('i', np.exp(np.pi*np.array([2., 3., 4.])*np.linspace(0, 1, 32).reshape([-1, 1])))

# ascii-string
xout.setitem('j', 'string variable from python')

# name of a data-block could be any ascii-string with less than 32 length
xout.setitem('longer-name', 1.)
xout.setitem('maximun_allowed_block_name_is_32', 2.)
xout.setitem('name_can_contain_any_ascii:@<>[]', 3.)

xout.close()
