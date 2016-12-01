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
    "inputHandler": "ois",
    "systems": ["ogre"],

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

Systems can be either registered statically, by calling :cpp:func:`Gsage::GsageFacade::addSystem` or
they can be constructed by :code:`GsageFacade` during :code:`initialize`, if the factory was registered for the system.

To construct systems using factory :code:`systems` field is used. For example:

.. code-block:: javascript

  ...
  "systems": ["ogre", "lua", "dynamicStats"]
  ...

* :code:`lua` and :code:`dynamicStats` are preinstalled systems.
* :code:`ogre` and :code:`recast` are registered by the OgrePlugin.

Each system has it's own unique id (:code:`render`, :code:`movement`, etc...).
It is possible to configure the system in the global config and in the level config.

System config should be placed under it's id in the root of the configuration object.
For example:

.. code-block:: javascript

  {
  ...
    "movement": {
      "cacheFolder": "./"
    }
    "coolSystem": {
      "setMeUP": 1
    }
  ...
  }

When the engine starts or if the system with id :code:`coolSystem` is added to the already running engine
it will be configured. It will get all configs as a :cpp:class:`Gsage::Dictionary` object, then it will be it's
responsibility to read all configs from that object.

See :ref:`custom-systems-label` for more information how to add new types of systems into Gsage engine.

Input
-----

Input is configured by :code:`inputHandler` field.
It should have string identifier of input factory, which is installed into the Gsage Facade.

Currently the only supported input handler type is :code:`ois`.
You can implement your own input handler and install it into the Gsage Facade.
See :ref:`custom-input-handler-label` to get more info how to implement your own input handler.
