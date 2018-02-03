Ogre Plugin
===========

Plugin Name: :code:`OgrePlugin`

.. note::

  This page is mostly a stub for OGRE Plugin documentation. It will be more useful after adding support for OGRE 2.1.

Ogre Plugin is going to support different versions of OGRE rendering library.
1.9.0 and 2.1.0 are pretty different feature wise and 2.1.0 does not have backward
compatibility with 1.9.0.

Common Features
^^^^^^^^^^^^^^^

TBD: only 1.9.0 is supported currently.

1.9.0
^^^^^

Supported rendering subsystems:

* Direct3D9 Rendering Subsystem
* OpenGL Rendering Subsystem

Custom pipeline setup is not there yet. Editor starts, but it crashes when trying to load the scene:

* Direct3D11 Rendering Subsystem

1.10.0
^^^^^^

TBD: when ogre 1.10 is supported.

2.1.0
^^^^^

Legacy API still can be used by defining that explicitly in render component entities and nodes.

