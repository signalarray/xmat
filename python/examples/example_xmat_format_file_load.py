"""start
Load xmat-file
^^^^^^^^^^^^^^
get the value by string key
stop"""

import common

# start example
import pathlib
import numpy as np
import xmat

# set the file name to load
file_in = pathlib.Path(__file__).parents[2].joinpath('data', 'example_python.xmat')

xin = xmat.MapStreamIn.file(file_in)
print(xin)

print(xin.getitem('a'))
print(xin.getitem('b'))
print(xin.getitem('c'))

xin.close()
