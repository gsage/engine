.. _eal-label:

Engine Abstraction Layer
========================

Gsage engine is designed to be as flexible as possible, thus it supports plugins.
As systems, that are installed from plugins most definetely will have different set of Lua bindings,
there is a Lua abstraction layer, that can be used to unify interfaces for different kind of systems.

EAL helps to tie Lua code to the engine entities.
Besides that EAL allows extensive code reuse between different objects.

The idea is pretty simple, each engine entity can have defined:

* A single :code:`class` type.
* Several :code:`mixin` s.
* Each component can also trigger EAL to add more Lua logic.

Defining an Extension
^^^^^^^^^^^^^^^^^^^^^

Extension is a function that accepts a class prototype as a first parameter.

.. code-block:: lua

  local function extension(cls)

  end

Then in that function it is possible any amount of additional methods.

.. code-block:: lua

  local function extension(cls)
    function cls:moveToOrigin()
      self.render:setPosition(0, 0, 0)
    end
  end

Besides defining custom methods, it is also possible to define :code:`setup` and :code:`teardown` methods.
They are pretty similar to constructor methods, but there can be more than one :code:`setup` and :code:`teardown`.

.. code-block:: lua

  local function extension(cls)
    ...
    cls.onCreate(function(self)
      self.someVariable = 1
    end)

    cls.onDestroy(function(self)
      print(tostring(self.id) .. " was destroyed")
    end)
    ...
  end

To add this extension to the system, it is required to call :code:`extend` method.

.. code-block:: lua

  eal:extend({mixin = "extension"}, extension)


Now everything combined will look like that:

.. code-block:: lua

  local eal = require 'eal.manager'

  local function extension(cls)
    cls.onCreate(function(self)
      self.someVariable = 1
    end)

    cls.onDestroy(function(self)
      print(tostring(self.id) .. " was destroyed")
    end)

    function cls:moveToOrigin()
      self.render:setPosition(0, 0, 0)
    end
  end

  eal:extend({mixin = "extension"}, extension)

In this example, extension is created as a mixin.
Then, to use this extension for an entity, it is required to modify it's JSON data.

.. code-block:: json

  {
    "id": "something",
    "props": {
      "mixins": ["extension"] // adding a mixin
    }
    "render": {
      "root": {
      }
    }
  }

After all these manipulations you should be able to use this EAL interface:

.. code-block:: lua

  local e = eal:getEntity("something")
  -- our extension method
  e:moveToOrigin()
  -- the variable set in set up should be accessible
  assert(e.someVariable == 1)

Supported Types of Extensions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

System
``````

Allows applying extension for all entities that have a component of a system :code:`system` with subtype :code:`type`.

Extending EAL:

.. code-block:: lua

  eal:extend({system="render", type="ogre"}, extension)

There is no need to modify entity data as this extension will be applied system wide:
each entity with component of system :code:`render` with subtype :code:`ogre` will have this extension applied.

Class
`````

When there is no need to stick to any particular system type, but it's still required to distinguish
different system :code:`subtype`, it is better to use the class extension.
Though it is also possible to define a class without a strict requirement of system type.

.. code-block:: lua

  -- enable this extension only when render system type is "ogre"
  eal:extend({class = {name = "camera", requires = {render = "ogre"}}}, extension)

Using it in the entity data:

.. code-block:: json

  {
    ...
    "props": {
      "class": "camera"
    }
    ...
  }

Mixin
`````

Mixin allows defining multiple different extensions for a single entity that are not tied to any specific system.
It is better to define only the highest level logic in the mixin.
Do not create too many mixins as it may hit the performance.

.. important::
  As it is possible to make a composition of extensions of different kinds, it is necessary to know the order they are applied.
  First go system level extensions.
  Then class extension.
  Then mixins in order, defined in the json array.
