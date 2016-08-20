# Graphics system agnostic game engine

The main idea of this engine is to be a core for a game, which brings together render, sound, physics engines and allows to script everything in lua.
This engine is based on [ECS](https://en.wikipedia.org/wiki/Entity_component_system) architecture. 

Each entity defined in the engine is combined from 1 to N components, for example:
```
{
  "id": "%name%",
  "flags": ["dynamic"],
  "render":
  {
    "root":
    {
      "name": "%name%",
      "position": "-100,0,10",
      "scale":"0.07,0.07,0.07",
      "rotation":"1,0,0.5,0",
      "orientationVector": "0,0,-1",
      "children":
      [
        {
          "type": "model",
          "query": "dynamic",
          "name": "%name%",
          "mesh": "model.mesh",
          "castShadows": true
        }
      ]
    }
  },
  "movement":
  {
    "speed": 4,
    "moveAnimation": "walk",
    "animSpeedRatio": 0.3
  },
}
```
It means that entity has movement component, which is managed by installed movement system and also has render component which means that it is displayed on the scene.

During the game loop, engine iterates all installed systems and calls update for each.
Each systems updates all it's component on each game loop.

# Linux build instructions

#### Getting ogre package

##### install mercurial:
```bash
  sudo apt-get install mercurial
```
##### download ogre sources:
```bash
  hg clone https://bitbucket.org/sinbad/ogre
  cd ogre
  hg update v1-9
```
##### install ogre dependencies:
```bash
  sudo apt-get install libfreetype6-dev libboost-date-time-dev \
  libboost-thread-dev nvidia-cg-toolkit libfreeimage-dev \
  zlib1g-dev libzzip-dev libois-dev libcppunit-dev doxygen \
  libxt-dev libxaw7-dev libxxf86vm-dev libxrandr-dev libglu-dev \
  libboost-dev cmake libx11-dev g++
```

##### Building ogre

```bash
  mkdir build
  cd build
  cmake ../
  make -j4
  sudo make install
```

#### Getting libRocket package

##### Downloading sources:
```bash
  git clone https://github.com/libRocket/libRocket.git
  cd libRocket
```

##### Building:
```bash
  cd Build
  cmake . -DBUILD_LUA_BINDINGS=1
  make -j4
  sudo make install
```

#### Building engine

##### Dependencies:
```bash
  sudo apt-get install libluabind-dev libois-dev libluajit-5.1-dev
```
##### Building:
```bash
  mkdir build
  cd build
  cmake ../
  make -j4
```

If you want to build editor, you should install Qt5 and define
`CMAKE_PREFIX_PATH`
variable in the PATH to the Qt cmake modules

##### It should look like this

```bash
  /home/<user>/Qt/<version>/gcc_64/lib/cmake
  # for example
  /home/artem/Qt/5.2.0/gcc_64/lib/cmake
```

##### Running an example

```bash
cd build/bin
./game
```

#### Notes:

- plugins.cfg has a ParticleUniverse plugin defined, you can remove it,
  if you don't have one.

#### Instructions TODO:
- write Windows step by step instructions.
- describe how to build and install ParticleUniverse plugin (it doesn't
  have install step, so maybe I'll fork it and add this functional).

# Contributing

1. Fork this repo
2. Create feature branch and push it to your fork
3. Create PR basing on your branch

Note that I want you to have only one commit in your PR. So squash it if you have several commits there.

# LICENSE

This project is licensed under MIT

# Plugins
- [Audio system plugin based on SFML](https://github.com/gsage/SFMLAudioSystemPlugin)
