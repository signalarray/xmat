"""start
Load xmat-file
^^^^^^^^^^^^^^
get the value by string key
stop"""

import pathlib
import numpy as np
import xmat

# set the file name to load
file_in = pathlib.Path(__file__).parents[2].joinpath('data', 'example_python.xmat')

xin = xmat.MapStreamIn.file(file_in)
print(xin)

a = xin.getitem('a')
b = xin.getitem('b')
c = xin.getitem('c')

xin.close()
