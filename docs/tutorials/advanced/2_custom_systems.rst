.. _custom-systems-label:
Custom Systems
==============

Writing a Component
-----------------

It is better to start from defining a component.
Defining a component is a really simple task. You should create new class, which is derived from
:cpp:class:`Gsage::EntityComponent`, then define static :code:`SYSTEM` field in the created component class. This field will
define the id which will be used for this component and the system.

Then you are free to define all kinds of properties, methods and other stuff.

The component is derived from the :cpp:class:`Gsage::Serializable`, so it can be easily configured to
read and write it's state into :code:`json/mspack`. More information about serialization :ref:`serializable-label`.

If everything is defined properly, :cpp:class:`Gsage::GameDataManager` will handle state saving and restoring for
the newly created component.

Engine System
-------------

Each Gsage Engine system should implement at least :cpp:class:`Gsage::EngineSystem` interface.

To make system work, you should implement several methods.

Abstract Methods
^^^^^^^^^^^^^^^^
  * :cpp:func:`Gsage::EngineSystem::update` - called on each engine update.
  * :cpp:func:`Gsage::EngineSystem::createComponent` - create component in the system.
  * :cpp:func:`Gsage::EngineSystem::removeComponent` - remove component from the system.
  * :cpp:func:`Gsage::EngineSystem::unloadComponents` - unload all components from the system.

This interface should be convenient to use for all systems that have components.
It's not if you want system without components.

For that, you can use :cpp:class:`Gsage::UpdateListener` interface.

If you want to add which does not need :code:`update` but has components, then you'll have to make stub
implementation for the :cpp:func:`Gsage::EngineSystem::update` method.

It would be nice to have a separate interface & pool for such systems so there is `a task <https://www.pivotaltracker.com/story/show/135001339>`_ for that.

Also, each system must have static field :code:`ID`, which defines it's string identifier for the :cpp:class:`Gsage::SystemManager`.

Optional Methods
^^^^^^^^^^^^^^^^
  * :cpp:func:`Gsage::EngineSystem::initialize` - handle initial configs in this method.
  * :cpp:func:`Gsage::EngineSystem::configure` - can be called if any level has different configs.

Don't forget to call base class implementation in each override, otherwise :cpp:var:`Gsage::EngineSystem::mConfig`
will be unset.

Fields
^^^^^^

  * :cpp:var:`Gsage::EngineSystem::mEngine` - engine instance.
  * :cpp:var:`Gsage::EngineSystem::mConfig` - dictionary the current system configs.

Component Storage
-----------------

There is also another class, which can be used as a base class for the system: :cpp:class:`ComponentStorage`.

This class helps you to handle component allocation, iteration, initialization.

It has only one pure virtual method :cpp:func:`Gsage::ComponentStorage::updateComponent`. This method is
called for each component in the system.

Optional Methods
^^^^^^^^^^^^^^^^

  * :cpp:func:`Gsage::ComponentStorage::prepareComponent` - call it for some precondition logic handling.
  * :cpp:func:`Gsage::ComponentStorage::fillComponentData` - this method can be used to configure the component.

Registering a New System
------------------------

Newly created system can be registered in the facade by a simple call.
Just call :cpp:func:`Gsage::GsageFacade::addSystem` with the new system.
You can do it at any time and engine will initialize this system properly.

Example:

.. code-block:: cpp

  facade.addSystem<Gsage::LuaScriptSystem>();


There is also another way to register new type of the system by using `Gsage::GsageFacade::registerSystemFactory`.

.. code-block:: cpp

  facade.registerSystemFactory<Gsage::LuaScriptSystem>("luaSystem");

After registering system this way, it will be possible to tell engine to create it using game config :code:`systems` field:

.. code-block:: javascript

  ...
  "systems": ["luaSystem"]
  ...

.. important::

  This solution is more flexible as it allows engine to create such systems at the runtime.


Further steps
-------------

* After you've created the new system, you may want to expose some methods to the lua. See :ref:`lua-bindings-label`, :ref:`bind-engine-systems-label` and :ref:`bind-entity-components-label` for more details.
* You may also want to wrap this new system into a plugin. See :ref:`plugins-label` for more details.
