.. _windows-build:

Windows Build Instructions
======================

.. important::
  Haven't built on windows for some time, so this won't work most likely.
  This instruction should work with Visual Studio 14.0 and windows 7+.

Getting Ogre
^^^^^^^^^^^^^

Get it there http://ogre3d.org/forums/viewtopic.php?t=69274

Getting libRocket package
-------------------------

Downloading Sources
^^^^^^^^^^^^^^^^^^^


1. Clone https://github.com/libRocket/libRocket.git
2. Run cmake with :code:`-DBUILD_LUA_BINDINGS=1` flag on.
3. Build with VS.

Building Engine
---------------

1. Get/Build luajit 5.1.
2. Get/Build msgpackc.
3. You should define the following env variables to make engine find dependencies:
   * :code:`LIBROCKET_DIR` for librocket.
   * :code:`LUAJIT_DIR` for luajit.
   * :code:`MSGPACK_DIR` for msgpackc.
   * :code:`OGRE_DEPENDENCIES_DIR` for OIS, boost and other ogre dependencies.
4. Run cmake and build with VS 14.

.. important::
  QT5 editor was never build on windows yet.
