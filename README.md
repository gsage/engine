Graphics System Agnostic Game Engine
====================================

[![Documentation Status](https://readthedocs.org/projects/engine/badge/?version=latest)](https://engine.readthedocs.io/en/latest/?badge=latest) ![Build](https://travis-ci.org/gsage/engine.svg?branch=master)

# The Purpose

This project provides a flexible game engine core, which can be easily
extended by any plugins.

For example:

* use any UI library you like by writing a plugin, based
  on UIManager interface.
* [add any type of system](http://engine.readthedocs.io/en/latest/tutorials/advanced/2_custom_systems.html)
  to the engine, customize rendering, add physics.

Along with plugin architecture, this engine provides a set of convenient
cpp classes, which can simplify new plugins creation.

For example for [serialization](http://engine.readthedocs.io/en/latest/tutorials/advanced/6_serializable.html).

This engine is based on [ECS](https://en.wikipedia.org/wiki/Entity_component_system) architecture.
It operates [entities](http://engine.readthedocs.io/en/latest/tutorials/basic/5_entities_format.html) which can consist of any amount of components.

You can read more documentation on [http://engine.readthedocs.io/](http://engine.readthedocs.io/).

This engine supports three OS systems at the moment:

* [Linux Build Instructions](http://engine.readthedocs.io/en/latest/tutorials/build/linux.html)
* [OSX Build Instructions](http://engine.readthedocs.io/en/latest/tutorials/build/mac.html)
* [Windows Build Instructions](http://engine.readthedocs.io/en/latest/tutorials/build/windows.html)

# Minimal Build Requirements

* GCC 4.9 + on linux
* Clang 3.5+
* Visual Studio 2015 Community (Visual C++ 14.0)+
* Cmake 3.1+

# Dependencies

### Required

* [luajit](http://luajit.org/)
* [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
* [msgpackc](https://github.com/msgpack/msgpack-c)

### Optional

* [libRocket](https://github.com/libRocket/libRocket) -- RocketUI.
* [ogre 1.9](http://www.ogre3d.org/) -- OgreBundle1.9 plugin.
* [Particle Universe](https://github.com/scrawl/particleuniverse) --
  ParticleUniverse plugin.
* OIS -- from ogre dependencies.

# Contributing

1. Fork this repo
2. Create feature branch and push it to your fork
3. Create PR basing on your branch

Please make your PR have only one commit, so squash them.

# LICENSE

This project is licensed under MIT

# Plugins
- [Audio system plugin based on SFML](https://github.com/gsage/SFMLAudioSystemPlugin)

# Issue Tracker
- [Pivotal Tracker](https://www.pivotaltracker.com/n/projects/963480)
