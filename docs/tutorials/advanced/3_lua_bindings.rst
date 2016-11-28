.. _lua-bindings-label:

Lua Bindings
============

:cpp:func:`Gsage::GsageFacade::initialize` initializes Lua state in the engine.
This is a single global Lua state for the whole engine (At least for now it's single).

To handle Lua related logic there is :cpp:class:`Gsage::LuaInterface` class, that handles some initial Lua bindings, like:

* bindings for entity and engine wrappers.
* :cpp:class:`Gsage::GsageFacade` bindings like :cpp:func:`Gsage::GsageFacade::halt`.
* :cpp:class:`Gsage::GameDataManager` bindings.
* and others.

`Sol2 <http://sol2.readthedocs.io/en/latest/>`_ library is used for Lua bindings.

Besides bindings, this class creates :code:`lua_State` and initializes global variables:

* :code:`core` - :cpp:class:`Gsage::Engine` instance.
* :code:`game` - :cpp:class:`Gsage::GsageFacade` instance.
* :code:`data` - :cpp:class:`Gsage::GameDataManager` instance.
* :code:`event` - :cpp:class:`Gsage::LuaEventProxy` instance.

Then any script can be executed by calling :cpp:func:`Gsage::LuaInterface::runScript`.
If :code:`startupScript` is defined in the game config :cpp:class:`Gsage::GsageFacade` will call
:cpp:func:`Gsage::LuaInterface::runScript` using it in the :code:`initialize` method.

Bindings Guidelines
-------------------

There are several guidelines for bindings to keep Gsage Lua API consistent.

Before you start manipulating Lua state, you should get it.
If it's plugin, then you should have :code:`LuaInterface` pointer there.

Then, there is method :cpp:func:`Gsage::LuaInterface::getSolState`, which will return pointer to Sol2 :code:`state_view`.

.. note::

  Note that the pointer to Sol2 state can be :code:`0` if the :code:`LuaInterface` was not initialized by calling :cpp:func:`Gsage::LuaInterface::initialize`.

.. _bind-engine-systems-label:

Bind Engine Systems
^^^^^^^^^^^^^^^^^^^

If you create a new engine system, you may want to access it from Lua.
Systems can be added dynamically at any point, so Lua state has :cpp:class:`Gsage::Engine` defined
as `simple usertype <http://sol2.readthedocs.io/en/latest/api/simple_usertype.html>`_.

This allows extending this binding at the runtime, like this:

.. code-block:: cpp

    lua["Engine"]["removeEntity"] = (bool(Engine::*)(const std::string& id))&Engine::removeEntity;
    lua["Engine"]["getEntity"] = &Engine::getEntity;
    lua["Engine"]["getSystem"] = (EngineSystem*(Engine::*)(const std::string& name))&Engine::getSystem;
    lua["Engine"]["script"] = &Engine::getSystem<LuaScriptSystem>;

So, if you add a new system, you will need to create a new binding like this:

.. code-block:: cpp

    lua.new_usertype<KittySystem>("KittySystem"
      "getKitten", &KittySystem::getKitten
    );

    lua["Engine"]["kitty"] = &Engine::getSystem<KittySystem>;



After you make this binding, you will be able to work with the system like this:

.. code-block:: lua

    s = core:kitty()
    s:getKitten()

.. _bind-entity-components-label:

Bind Entity Components
^^^^^^^^^^^^^^^^^^^^^^

It's very likely that you will want to access system components.
It's almost the same as for the system, but instead of :code:`Engine` binding, you should use :code:`EntityProxy`:

.. code-block:: cpp

    lua.new_usertype<KittyComponent>("KittyComponent"
      "meow", &KittyComponent::meow
    );

    lua["EntityProxy"]["kitty"] = &EntityProxy::getComponent<KittyComponent>;

And then you will be able to work with this component from Lua:

.. code-block:: lua

    e = entity.get("cat")
    cat:kitty():meow()

Bind Events
^^^^^^^^^^^

As Sol2 does not support upcasting out of the box, when you bind an event, you
should also implement :code:`cast` method there.

As :cpp:class:`Gsage::LuaEventProxy` passes all events to the callback as :cpp:class:`Gsage::Event` base class,
it is required to cast it to any particular at runtime.

Example:

.. code-block:: cpp

    static To cast(From f) {
      return static_cast<To>(f);
    }
...

    lua.new_usertype<SelectEvent>("SelectEvent",
        sol::base_classes, sol::bases<Event>(),
        "hasFlags", &SelectEvent::hasFlags,
        "entity", sol::property(&SelectEvent::getEntityId),
        "cast", cast<const Event&, const SelectEvent&>
    );

:cpp:func:`Gsage::cast` is used in this example.

Then in Lua event should be handled like this:

.. code-block:: lua

  local onSelect = function(event)
    local e = SelectEvent.cast(event)
    -- then you will be able to access derived class methods
    print(e:hasFlags(OgreSceneNode.DYNAMIC))
  end

  event:bind(core, "objectSelected", onSelect)
