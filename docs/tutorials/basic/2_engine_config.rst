Game Configuration
==================

Gsage engine has one global configuration file.
It is loaded by :cpp:func:`Gsage::GsageFacade::initialize`.

This file has initial configs for all engine systems, plugins list, startup script path and data manager configuration.

It can be encoded in json and msgpack and can look like this:

.. code-block:: javascript

  {
    "dataManager":
    {
      "extension": "json",
      "charactersFolder": "characters",
      "levelsFolder": "levels",
      "savesFolder": "templates"
    },
    "startupScript": "scripts/start.lua",

    "plugins":
    [
      "PlugIns/Plugin_ParticleUniverseFactory"
    ],

    "render":
    {
      "pluginsFile": "plugins.cfg",
      "configFile": "ogreConfig.cfg",
      "globalResources":
      {
        "General":
        [
          "FileSystem:materials/",
          "FileSystem:programs/",
          "FileSystem:particles/PU",
          "FileSystem:particles/Ogre"
        ],
        "Rocket":
        [
          "FileSystem:fonts/",
          "FileSystem:ui/"
        ]
      },

      "window":
      {
        "name": "Game",
        "width": 1280,
        "height": 800,
        "params":
        {
          "FSAA": 4,
          "displayFrequency": 50,
          "vsync": false
        }
      }
    }
  }

.. _game-datamanager-settings-label:

:cpp:class:`Gsage::GameDataManager` Config
--------------------------------------------

All settings are in :code:`"dataManager"` section.
It has 4 types of parameters:

* :code:`"extension"` extension of all data files. :code:`"json"` is recommended.
* :code:`"charactersFolder"` folder where to search characters construction data.
* :code:`"levelsFolder"` folder where to search levels data.
* :code:`"savesFolder"` folder where to keep saves.

Plugins List
------------

:code:`"plugins"` stores list of plugins to be loaded on engine startup.
Plugins are c++ dynamic libraries: :code:`*.so/*.dylib/*.dll`.

.. note::
    Should be specified without extension. Engine will add appropriate extension for each platform itself.

Each defined plugin will be installed in the order defined in the list.

Systems Configs
---------------

Each system has it's own configs.
