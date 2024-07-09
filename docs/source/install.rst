Install
========

C++
----

CMake

Python
-------

setup.py


Matlab
-------
The installation of `xmat` is no different from installing any other MATLAB toolbox.
You only need to download/clone the xmat-repository to a folder, and add the folder's path to MATLAB's path list by using the following command:

.. code-block:: matlab

  addpath('../xmat/matlab')

Another way. Create a `startup.m <https://www.mathworks.com/help/matlab/ref/startup.html>`_ file in the `userpath` folder, which is on the MATLAB search path
and type ``addpath('../xmat/matlab')`` in this file. MATLAB will execute this file every time it starts. The way to find your `userpath` is to call
`userpath <https://www.mathworks.com/help/matlab/ref/userpath.html>`_ standard function.




