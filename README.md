Graphics System Agnostic Game Engine
====================================

[![Documentation Status](https://readthedocs.org/projects/engine/badge/?version=latest)](https://engine.readthedocs.io/en/latest/?badge=latest)
[![Build status](https://ci.appveyor.com/api/projects/status/vf3oobbg4ofld3of?svg=true)](https://ci.appveyor.com/project/Unix4ever/engine)
[![Build Status](https://travis-ci.org/gsage/engine.svg?branch=master)](https://travis-ci.org/gsage/engine)
[![Gitter](https://badges.gitter.im/gsage/engine.svg)](https://gitter.im/gsage/engine?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

## Showcase

**GsageExe example is not working right now, that's known issue. This
example will be replaced by proper template project that will be
available from the editor**

![Editor](/docs/images/editor.png)
![ExampleGame](/docs/images/game2.png)
See [documentation](http://engine.readthedocs.io/) for more examples.

## About

This project moves away from the paradigm of having Swiss Army Knife
engine that can do anything and can be too bulky for some small projects.
Instead, it aims to be configurable and easily expandable by plug-ins.
If some plug-ins are not needed they can be removed from the project.

Thus, the Engine has the Core, which is static itself, but has many
customization traits, like:

* UIManager.
* InputManager.
* WindowManager.
* IPlugin.

So by using these interfaces, one can easily implement new input system,
or add support for any UI library and so on.

As this engine is based on [ECS](https://en.wikipedia.org/wiki/Entity_component_system) architecture,
adding new kind of game logic can be achieved by [writing](http://engine.readthedocs.io/en/latest/tutorials/advanced/2_custom_systems.html) new EngineSystem and Component.

The Core also provides a set of convenient tools:
* [serialization](http://engine.readthedocs.io/en/latest/tutorials/advanced/6_serializable.html).
* lua bindings which can be expanded in any plug-in.
* implementation agnostic events for Mouse and Keyboard.
* executable fully configurable via json configuration file.
* logging.
* has built in lua script component and system.
* dynamic plugin loader.
* delegates, with support of event propagation to lua.

You can read more documentation on [http://engine.readthedocs.io/](http://engine.readthedocs.io/).

This engine supports three OS systems at the moment:
* Windows, Visual Studio 2015 and later.
* OSX, ci is using Xcode 7.3.
* Linux, ci is using Ubuntu 14.04 with GCC 4.9.

There are plans to support iOS and Android.

Use [build instructions](http://engine.readthedocs.io/en/latest/tutorials/build/conan.html) to build the engine.

## Dependencies

Most of dependencies are vendored by Conan.

### Main

* [luajit](http://luajit.org/).
* [jsoncpp](https://github.com/open-source-parsers/jsoncpp).
* [msgpackc](https://github.com/msgpack/msgpack-c).
* [easylogging](https://muflihun.github.io/easyloggingpp/).
* [sol2](https://github.com/ThePhD/sol2).
* [sole](https://github.com/r-lyeh-archived/sole).
* [CEF](https://bitbucket.org/chromiumembedded/cef).
* [inja](https://github.com/pantor/inja).

### Optional

* [libRocket](https://github.com/libRocket/libRocket) -- RocketUI.
* [ogre 1.9/2.1](http://www.ogre3d.org/) -- Ogre plugin (2.1 is still
  unstable).
* [Particle Universe](https://github.com/scrawl/particleuniverse) --
  ParticleUniverse plugin.
* OIS -- from ogre dependencies.
* [SDL2](https://www.libsdl.org/download-2.0.php) -- SDL2 windowing
  and input.
* [recastnavigation](https://github.com/recastnavigation/recastnavigation).
* [dear imgui](https://github.com/ocornut/imgui).

## Contributing

1. Fork this repo
2. Create feature branch and push it to your fork
3. Create PR basing on your branch

Please make your PR have only one commit, so squash them.

## LICENSE

This project is licensed under MIT

## Other Plugins
- [Audio system plugin based on SFML](https://github.com/gsage/SFMLAudioSystemPlugin)

## Issue Tracker
- [Pivotal Tracker](https://www.pivotaltracker.com/n/projects/963480)

