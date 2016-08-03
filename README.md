
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
  sudo apt-get install libluabind-dev libois-dev
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

#### Notes:

- plugins.cfg has a ParticleUniverse plugin defined, you can remove it,
  if you don't have one.

#### Instructions TODO:
- write Windows step by step instructions.
- describe how to build and install ParticleUniverse plugin (it doesn't
  have install step, so maybe I'll fork it and add this functional).
