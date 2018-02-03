.. _conan-build:

Conan Build Instructions
========================

It is possible to build the engine using Conan.
It should be relatively simple, though it's necessary to have some prerequisites.

.. important::
  You should have Makefile, gcc/XCode/VS2015 and Conan installed

Then run :code:`make build` from the root directory.

There are also other build targets:
1. :code:`make run` starts the game.
2. :code:`make unit` runs unit tests.
3. :code:`make function` runs lua functional tests.

On Windows you can install Anaconda https://anaconda.org/anaconda/python .
Then you can open Anaconda promnt and install conan:

.. code-block:: bash
   
   conda install conan

Then set up VC environment using :code:`vcvarsall.bat`.
