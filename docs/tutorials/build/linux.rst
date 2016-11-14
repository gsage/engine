.. _linux-build:

Linux Build Instructions
========================

Getting Ogre Package
--------------------

Install Mercurial
^^^^^^^^^^^^^^^^^^

.. code-block:: bash

    sudo apt-get install mercurial

Download Ogre Sources
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

  hg clone https://bitbucket.org/sinbad/ogre
  cd ogre
  hg update v1-9

Install Ogre Dependencies
^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

  sudo apt-get install libfreetype6-dev libboost-date-time-dev \
  libboost-thread-dev nvidia-cg-toolkit libfreeimage-dev \
  zlib1g-dev libzzip-dev libois-dev libcppunit-dev doxygen \
  libxt-dev libxaw7-dev libxxf86vm-dev libxrandr-dev libglu-dev \
  libboost-dev cmake libx11-dev g++

Building Ogre
^^^^^^^^^^^^^

.. code-block:: bash

  mkdir build
  cd build
  cmake ../
  make -j4
  sudo make install

Getting libRocket package
-------------------------

Downloading Sources
^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

  git clone https://github.com/libRocket/libRocket.git
  cd libRocket

Building
^^^^^^^^

.. code-block:: bash

  cd Build
  cmake . -DBUILD_LUA_BINDINGS=1
  make -j4
  sudo make install

Building Engine
---------------

.. important::
  Gcc 4.9+ is required to build the engine.

Dependencies
^^^^^^^^^^^^

.. code-block:: bash

  sudo apt-get install libois-dev libluajit-5.1-dev libjsoncpp-dev libmsgpack3

Building
^^^^^^^^

.. code-block:: bash

  mkdir build
  cd build
  cmake ../
  make -j4

If you want to build editor, you should install Qt5 and define :code:`CMAKE_PREFIX_PATH` variable in the PATH to the Qt cmake modules.
