Saving and Loading Game State
================================

Level File Format
---------------------

Static scene objects are defined by :code:`render` component of an entity.
Only difference for static object is the **static** flag, defined in the entity.
Static scene object can have any other components defined as well, such as script or sound.

Level is described by simple json or msgpack:

.. code-block:: javascript

   {
     "settings": {
       "render": {
         "resources": {
           "levelID": [
             "Zip:models/packs/levelResourcePack.zip"
           ]
         },
         "colourBackground": "0xc0c0c",
         "colourAmbient": "0x7F7F7F",
         "colourDiffuse": "0xc0c0c"
       },
       "script": {
         "hooks": {
           "camera": "setOrbitalCam()",
           "damageEmitter": "emitter.create('damage')"
         }
       }
     },
     "entities": [{
         "id": "castle",
         "render": {
           "root": {
             "position": "0,0,0",
             "rotation": "1,0,-1,0",
             "scale": "1,1,1",
             "children": [{
               "type": "model",
               "mesh": "castle.mesh",
               "name": "castle",
               "castShadows": true
             }]
           }
         }
       }
     ]
   }

:code:`"settings"` section is used to reconfigure all engine systems
for the particular level. It works in the similar way as the global config.
In this example, :code:`"render"` and :code:`script` systems are configured, and have the following settings:

* :code:`"resources"` contains list of resources, that are required for
  the level. Each resource list is identified by unique id to make shared
  resources reusable between different levels.

* :code:`"script"` has some level startup script hooks defined.

:code:`"entities"` section describes entity list to be used as static scene elements.

Loading Levels
------------------

All levels are stored in the same folder. Level can be loaded by calling :cpp:func:`Gsage::GameDataManager::loadArea`.
This will initialize all static entities and reconfigure systems.

By default, level folder path is :code:`resources/levels`.
Game data manager is configured in the `gameConfig.json` file, see :ref:`game-datamanager-settings-label` for more information.

Levels information should be considered as constant. Game data loader is designed with thoughts that
levels information will be stored in the single archive, which won't be changed.

To provide ability to modify levels information and save dynamic entities states, save files are used.

.. node::
    Dynamic entities are player, enemies and any movable scene elements.

.. note::
    Currently save file copies the whole level information. There is plan to save only difference between initial data
    and save

To load save file :cpp:func:`Gsage::GameDataManager::loadSave` function can be used.
Save file format differs from level file format a bit:

* save file saves level information in the field :code:`area`.

Several new Fields:

* :code:`characters` saves all characters information.
* :code:`placement` saves positions of all characters on all locations.
* :code:`settings` saves global configs for systems (can be used for render system settings customizations).

.. note::
    Save file format might change in the future. :code:`settings` is likely to be removed.

Saving Levels
-----------------

Each engine component and system supports serialization.
All settings, that can be read from configs also can be dumped to config files.

This allows :cpp:class:`Gsage::GameDataManager` to iterate all systems and entities and dump their state to file.

Save file can be created by calling :cpp:func:`Gsage::GameDataManager::dumpSave`.

.. note::
    There is no method to modify level file itself yet. It will be created for editor purposes later.

Lua Bindings
----------------

* :code:`game:loadSave("save1234")`
* :code:`game:dumpSave("save1234")`
