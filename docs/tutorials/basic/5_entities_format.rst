Entity
======

.. _entity-format-label:

Entity Format
-------------

As Gsage uses ECS architecture, it operates entites.
Each entity consists of several components.

Each component can communicate with adjacent component through entity.
Each component belongs to the separate system and stores state of the
entity for that system.

For example, for render system:

.. code-block:: javascript

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

The component stores information about nodes: :code:`"root"` is always root node of the component.
It has :code:`"position"`, :code:`"rotation"` and other props, typical for render system.
:code:`"children"` here can store the list of various visual children:

* models.
* billboards.
* particle systems.
* and others.

There are different kinds of systems.
Script component data can look like this:

.. code-block::

  "script":
  {
    "setupScript": "@File:characters/scripts/ninja.lua",
    "behavior": "dumbMonster"
  }

The engine is highly extensible, so it's easy to add any new kind of system.
Each system is installed in the engine with the unique string id. The same id is used
to match the system where the entity should be created.

That way, engine will find system by the name :code:`"script"` and will create new component there using
parameters, which are defined for it in the entity description.

For example for the **script** component type it will call
:cpp:func:`Gsage::Engine::createComponent` with :code:`type = "script"` and dict:

.. code-block:: javascript

  {
    "setupScript": "@File:characters/scripts/ninja.lua",
    "behavior": "dumbMonster"
  }

Then it will add the pointer to the created component to the entity object.

Lifecycle
---------

On each update, each component state can be updated.
Engine iterates and updates all the systems and each system iterates and updates all it's components.

Each component can be altered during engine operation.
For example, render component can update it's position or change model.
Movement can change speed.

Entity add Flow
---------------


