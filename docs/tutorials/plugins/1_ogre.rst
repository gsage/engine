Ogre Plugin
===========

Plugin Name: :code:`OgrePlugin`

Only one version of OGRE can be built simultaneously. E.g. you can't build both 2.1 and 1.9 plugins at the same time.

Common Features
^^^^^^^^^^^^^^^

1.9.0
^^^^^

This version is still used by default, no additional build parameters are required to select this version.

Supported rendering subsystems:

* Direct3D9 Rendering Subsystem
* OpenGL Rendering Subsystem

Custom pipeline setup is not there yet. Editor starts, but it crashes when trying to load the scene:

* Direct3D11 Rendering Subsystem

1.10.0
^^^^^^

Will be used instead of 1.9.0 in future. 1.9.0 support will be dropped.

2.1.0
^^^^^

To build against OGRE 2.1.

Using makefile:

.. code-block:: bash

   # bash
   OGRE_VERSION=2.1.0 make build

   # windows shell
   set OGRE_VERSION=2.1.0
   make.exe build

Conan commands:

.. code-block:: bash

	conan install -g cmake -s build_type=Release -o gsage:with_ogre=2.1.0 --build=outdated .
	conan build .

Supported rendering subsystems:

* OpenGL 3+ Rendering Subsystem
* Metal Rendering Subsystem (OSX only)
