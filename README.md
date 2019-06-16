Graphics System Agnostic Game Engine
====================================

[![Documentation Status](https://readthedocs.org/projects/engine/badge/?version=latest)](https://engine.readthedocs.io/en/latest/?badge=latest)
[![Build status](https://ci.appveyor.com/api/projects/status/vf3oobbg4ofld3of?svg=true)](https://ci.appveyor.com/project/Unix4ever/engine)
[![Build Status](https://travis-ci.org/gsage/engine.svg?branch=master)](https://travis-ci.org/gsage/engine)
[![Gitter](https://badges.gitter.im/gsage/engine.svg)](https://gitter.im/gsage/engine?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

## Downloads

Binary downloads, created by CI are available on Github releases.
[Get latest release](https://github.com/gsage/engine/releases/latest) (MSI, DMG and DEB are available).

## Showcase

![Editor](/docs/images/editor.v0.3.png)
![ExampleGame](/docs/images/game2.png)
See [documentation](http://engine.readthedocs.io/) for more examples.

## About

This project moves away from the paradigm of having Swiss Army Knife
engine that can do anything and can be too bulky for some small projects.
Instead, it aims to be configurable and easily expandable by plug-ins.
If some plug-ins are not needed they can be removed from the project.

Thus, the Engine has the Core, which is static itself, but has many
customization traits, like:

* UI library interface `UIManager`.
* Input handling interface `InputManager`.
* Window handling interface `WindowManager`.
* Generic plugin interface `IPlugin`.

So by using these interfaces, one can easily implement new input system, add support for any UI library and so on.

As this engine is based on [ECS](https://en.wikipedia.org/wiki/Entity_component_system) architecture,
adding new kind of game logic can be achieved by [writing](http://engine.readthedocs.io/en/latest/tutorials/advanced/2_custom_systems.html) new EngineSystem and Component.

The Core also provides a set of convenient tools:
* [serialization](http://engine.readthedocs.io/en/latest/tutorials/advanced/6_serializable.html).
* lua bindings which can be expanded in any plug-in.
* implementation agnostic events for Mouse and Keyboard.
* executable fully configurable via json configuration file.
* logging.
* dynamic plugin loader.
* delegates, with support of event propagation to lua.
* some generic systems bundled into the core (script, movement).

You can read more documentation on [http://engine.readthedocs.io/](http://engine.readthedocs.io/).

This engine supports three OS at the moment:
* Windows, Visual Studio 2017 and later.
* OSX, ci is using Xcode 9.1.
* Linux, ci is using Ubuntu 16.04 with GCC 5.4.

There are plans to support iOS and Android.

Use [build instructions](http://engine.readthedocs.io/en/latest/tutorials/build/conan.html) to build the engine.

## Dependencies

Most of dependencies are vendored by Conan.

### Main

* [Luajit](http://luajit.org/) Scripting.
* [jsoncpp](https://github.com/open-source-parsers/jsoncpp) config/level
  data parsing.
* [msgpackc](https://github.com/msgpack/msgpack-c) used for released
  game package resources (levels/characters).
* [easylogging](https://muflihun.github.io/easyloggingpp/).
* [sol2](https://github.com/ThePhD/sol2) Lua bindings.
* [sole](https://github.com/r-lyeh-archived/sole) UUID generator.
* [inja](https://github.com/pantor/inja) Jinja2 like template renderer.
* [POCO](https://github.com/pocoproject/poco) Networking, threading,
  filesystem.

### Optional

* [libRocket](https://github.com/libRocket/libRocket) RocketUI interface.
* [ogre 1.9/2.1](http://www.ogre3d.org/) powered rendering system.
* OIS input from ogre dependencies (only for OGRE 1.9).
* [SDL2](https://www.libsdl.org/download-2.0.php) SDL2 windowing
  and input.
* [recastnavigation](https://github.com/recastnavigation/recastnavigation) navmesh building/pathfinding.
* [dear imgui](https://github.com/ocornut/imgui) interface.
* [CEF](https://github.com/chromiumembedded/cef/).

## Contributing

Fork the repo on GitHub
1. Clone the project to your own machine
2. Commit changes to your own branch
3. Push your work back up to your fork
4. Submit a Pull request with a single commit preferably

NOTE: Be sure to merge the latest from "upstream" before making a pull request!

## LICENSE

This project is licensed under MIT license.

## Other Plugins
- [Audio system plugin based on SFML](https://github.com/gsage/SFMLAudioSystemPlugin)

## Issue Tracker
- [Pivotal Tracker](https://www.pivotaltracker.com/n/projects/963480)
