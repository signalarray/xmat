Install
========

C++
----

CMake

Python
-------
**PyPi**

`xmat` runs under Python 3.10+ and awailable on `pypi.org`. To install it with pip, run the following:

``pip install xmat``


**Install from source code**

``pip install %{path_to_xmat_repository}/python``

Development mode. See some `help <https://setuptools.pypa.io/en/latest/userguide/development_mode.html>`_.

``pip install -e %{path_to_xmat_repository}/python``


Matlab
-------
The installation of `xmat` is no different from installing any other MATLAB toolbox.
You only need to download/clone the xmat-repository to a folder, and add the folder's path to MATLAB's path list by using the following command:

.. code-block:: matlab

  addpath('%{path_to_xmat_repository}/matlab')

Another way. Create a `startup.m <https://www.mathworks.com/help/matlab/ref/startup.html>`_ file in the `userpath` folder, which is on the MATLAB search path
and type ``addpath('%{path_to_xmat_repository}/matlab')`` in this file. MATLAB will execute this file every time it starts. The way to find your `userpath` is to call
`userpath <https://www.mathworks.com/help/matlab/ref/userpath.html>`_ standard function.




