.. _mac-build:

Mac Build Instructions
======================

.. important::
  This instruction should work with XCode 7.2 and Mac OS X 10.10.

Getting Ogre Package
--------------------

Install Mercurial
^^^^^^^^^^^^^^^^^^

.. code-block:: bash

    brew install mercurial

Download Ogre Sources
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

  hg clone https://bitbucket.org/sinbad/ogre
  cd ogre
  hg update v1-9

.. _ogre-deps-label:

Install Ogre Dependencies
^^^^^^^^^^^^^^^^^^^^^^^^^

Download cg toolkit:
https://developer.nvidia.com/cg-toolkit-download

Two options:

1. Download from here http://sourceforge.net/projects/ogre/files/ogre-dependencies-mac/1.9/OgreDependencies_OSX_libc%2B%2B_20130610.zip/download
2. Install by brew.

Building Ogre
^^^^^^^^^^^^^

Follow this instruction:
http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Building+Ogre3D+1.9+Statically+in+Mac+OS+X+(Yosemite)

.. important::
  Do NOT build static! Gsage can't handle static Ogre plugins initialization yet.

Getting libRocket package
-------------------------

Downloading Sources
^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

  git clone https://github.com/unix4ever/libRocket.git
  cd libRocket

Building
^^^^^^^^

.. code-block:: bash

  cd Build
  cmake . -DBUILD_LUA_BINDINGS=1 -DUSE_LUAJIT=1
  make -j4
  sudo make install

Building Engine
---------------

Dependencies
^^^^^^^^^^^^

.. code-block:: bash

  brew install luajit glew --universal

Building
^^^^^^^^

If dependencies are not installed by brew :code:`<path_with_dependencies>` should point to folder, downloaded here :ref:`ogre-deps-label`.

.. code-block:: bash

  export OGRE_HOME=<ogre_home_directory>
  mkdir build
  cd build
  OGRE_DEPENDENCIES_DIR=<path_with_dependencies> cmake ../ -DCMAKE_OSX_ARCHITECTURES=x86_64
  make -j $(sysctl -n hw.ncpu)

If you want to build editor, you should install Qt5 and define :code:`CMAKE_PREFIX_PATH` variable in the PATH to the Qt cmake modules.

.. important::
  Qt5 editor starts on mac, but FBO rendering does not work with QT for some reason yet.
