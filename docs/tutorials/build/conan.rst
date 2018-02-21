.. _conan-build:

Conan Build Instructions
========================

It is possible to build the engine using Conan.
It should be relatively simple, though it's necessary to have some prerequisites.

Before you start
----------------

.. important::
  You should have Makefile, gcc/XCode/VS2015 and Conan installed

On Windows you can install Anaconda https://anaconda.org/anaconda/python .
Then you can open Anaconda prompt and install conan:

.. code-block:: bash
   
   conda install conan

Set up VC environment using :code:`vcvarsall.bat` and you're all set.

Build And Run
-------------

To build the project run :code:`make build` from the root directory.

Other convenient build targets:

1. :code:`make run` starts the game.
2. :code:`make editor` run editor.
3. :code:`make unit` runs unit tests.
4. :code:`make functional` runs lua functional tests.

If you do not want to use :code:`make`, you can run several build commands manually:

.. code-block:: bash

  # add gsage conan repository to Conan
  conan remote add gsage https://api.bintray.com/conan/gsage/main --insert

  # install Conan dependencies
  conan install -g cmake -o gsage:with_ogre=1.9.0 -o gsage:with_librocket=True -o with_lua_version:luajit-2.0.5 --build=outdated .

  # build the project
  conan build .
