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

    "packager": {
      "deps": [
        "tween"
      ]
    },

    "windowManager": {
      "type": "SDL"
    },

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

:cpp:class:`Gsage::GameDataManager` settings are stored in :code:`"dataManager"` section.
There can be 4 kinds of variables:

* :code:`"extension"` extension of all data files. :code:`"json"` is recommended.
* :code:`"charactersFolder"` folder where to search characters construction data.
* :code:`"levelsFolder"` folder where to search levels data.
* :code:`"savesFolder"` folder where to keep saves.

Plugins List
------------

:code:`"plugins"` stores list of plugins to be loaded on engine startup.
Plugins are c++ dynamic libraries: :code:`*.so/*.dylib/*.dll`.

.. note::
    Plugin should be specified without extension. Engine will add appropriate extension for each platform itself.

Each defined plugin will be installed in the order defined in the list.

Systems Configs
---------------

Systems can be either registered statically, by calling :cpp:func:`Gsage::GsageFacade::addSystem` or
they can be created by :code:`SystemFactory` in the runtime in :code:`GsageFacade::initialize` function.

:code:`SystemFactory` reads :code:`systems` array in the configration file. For example:

.. code-block:: javascript

  ...
  "systems": ["ogre", "lua", "dynamicStats"]
  ...

* :code:`lua` and :code:`dynamicStats` are preinstalled systems.
* :code:`ogre` and :code:`recast` are registered by the OgrePlugin.

Each system has two identifiers:
* **implementation** id.
* **functional** id.

**Implementation** id is used by :code:`SystemFactory` to create a system.
**Functional** id defines system purpose and is used to identify it's components.

For example, there is :code:`render` system that is using :code:`ogre` underneath.

When the system is added to the engine it can read the configuration from the global configuration file.
System configuration must be stored on the root level of global configuration file or scene file under
**functional** id.

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

Engine will inject each system configuration placed under system **functional** id.
The system will get a :cpp:class:`Gsage::DataProxy` object and will get all system specific parameters from it.

See :ref:`custom-systems-label` for more information how to add new types of systems into Gsage engine.

Input
-----

Input is configured by :code:`inputHandler` field.
It should have string identifier of input factory, which is installed into the Gsage Facade.

Currently it supports two kinds of inputHandlers:
* :code:`SDL` (preferred).
* :code:`ois` (may be removed in future releases).

You can implement your own input handler and install it into the Gsage Facade.
See :ref:`custom-input-handler-label` to get more info how to implement your own input handler.

Window Management
-----------------

:code:`windowManager` section can be used to configure window management system.
It has one mandatory field and one optional:

:code:`"type"` is mandatory and defines window manager type to use.
:code:`"windows"` is optional and may contain the list of windows that should be created by the window manager.

Elements of this list should be objects and may vary depending on the implementation fo the window manager.

Log Config
----------

:code:`logConfig` can be used to define path to log configuration file.
Refer to `easylogging++ documentation <https://github.com/muflihun/easyloggingpp#using-configuration-file>`_ for more details.

Packager
--------

This packager can install any lua dependencies using luarocks.
:code:`deps` array should contain the list of dependencies.
Each entry of this array support version pinning and version query operators.

Plug-Ins
--------

Global config file can contain any additional configuration, which are relevant to installed plugins.
