.. _lua-bindings-label:

Lua Bindings
============

:cpp:func:`Gsage::GsageFacade::initialize` initializes Lua state in the engine.
This is a single global Lua state for the whole engine (At least for now it's single).

:cpp:class:`Gsage::LuaInterface` binds the Core bindings:

* bindings for entity and engine wrappers.
* :cpp:class:`Gsage::GsageFacade` bindings like :cpp:func:`Gsage::GsageFacade::shutdown`.
* :cpp:class:`Gsage::GameDataManager` bindings.
* and others.

`Sol2 <http://sol2.readthedocs.io/en/latest/>`_ library is used for Lua bindings.

Besides bindings, :code:`LuaInterface` initializes global variables:

* :code:`core` - :cpp:class:`Gsage::Engine` instance.
* :code:`game` - :cpp:class:`Gsage::GsageFacade` instance.
* :code:`data` - :cpp:class:`Gsage::GameDataManager` instance.
* :code:`log`  - provides easylogging++ API to lua.

Any script can be executed by calling :cpp:func:`Gsage::LuaInterface::runScript`.
:code:`startupScript` configuration variable can be used to define script which will be executed after
:cpp:class:`Gsage::GsageFacade` initialization.

:code:`LuaInterface` also runs packager script.

Bindings Guidelines
-------------------

All Core bindings are located in the :code:`LuaInterface` file.
If any plug-in needs to add some Lua bindings, it should override :cpp:class:`Gsage::IPlugin::setupLuaBindings` method.
Plug-in can access Lua state by using :code:`mLuaInterface->getSolState()` function. It will return pointer to
:code:`sol::state_view`.

.. note::

  Note that the pointer to Sol2 state can be :code:`0` if the :code:`LuaInterface` was not initialized by calling :cpp:func:`Gsage::LuaInterface::initialize`.

.. _bind-engine-systems-label:

Bind Engine Systems
^^^^^^^^^^^^^^^^^^^

If you create a new engine system, you may want to access it from Lua.
Systems can be added dynamically at any point and :cpp:class:`Gsage::Engine` functions should be updated in the runtime:

.. code-block:: cpp

    lua["Engine"]["script"] = &Engine::getSystem<LuaScriptSystem>;

This way, when a plugin registers a new system, it will also update :cpp:class:`Gsage::Engine` to have getter 
for this system: :code:`core:script()`.

So, if you add a new system, you will need to create a new binding like this:

.. code-block:: cpp

    lua.new_usertype<KittySystem>("KittySystem"
      "getKitten", &KittySystem::getKitten
    );

    lua["Engine"]["kitty"] = &Engine::getSystem<KittySystem>;



After you make this binding, you will be able to get the system instance:

.. code-block:: lua

    s = core:kitty()
    s:getKitten()

.. _bind-entity-components-label:

Bind Entity Components
^^^^^^^^^^^^^^^^^^^^^^

When registering a new Component, you should also update :cpp:class:`Gsage::Entity` functions
and add the getter for the new Component in the same way as for the new System, but instead of :code:`Engine` binding, you should use :code:`Entity`:

.. code-block:: cpp

    lua.new_usertype<KittyComponent>("KittyComponent"
      "meow", &KittyComponent::meow
    );

    lua["Entity"]["kitty"] = &Entity::getComponent<KittyComponent>;

After that it will be possible to get Component from Entity instance by using newly registered getter:

.. code-block:: lua

    e = eal:getEntity("cat")
    cat.kitty:meow()

Bind Events
^^^^^^^^^^^

Events can be handled in Lua script in two ways: 
* :code:`event:bind(...)` will bind generic callback. You can use it if you do not need upcasting from :cpp:class:`Gsage::Event` to derived event type.
* :code:`event:<handlerID>(...)` will bind callback specifically for some concrete type of event.

If you use bind, you will not be able to access derived class methods or variables:

.. code-block:: lua

  local onSelect = function(event)
    -- then you will be able to access derived class methods
    print(e.hasFlags) -- prints nil
  end

  event:bind(core, "objectSelected", onSelect)

:code:`handlerID` is defined when binding a new event type:

Example:

.. code-block:: cpp

    registerEvent<SelectEvent>("SelectEvent",
        "onSelect", // <-- handlerID
        sol::base_classes, sol::bases<Event>(),
        "hasFlags", &SelectEvent::hasFlags,
        "entity", sol::property(&SelectEvent::getEntityId),
    );

To handle :cpp:class:`Gsage::SelectEvent` in Lua:

.. code-block:: lua

  local onSelect = function(event)
    -- you will be able to access derived class methods
    print(e:hasFlags(OgreSceneNode.DYNAMIC))
  end

  event:onSelect(core, "objectSelected", onSelect)
