.. _plugins-label:

Plugins
=======

Gsage Engine supports plugins, which can be installed at the runtime.
Each plugin can be wrapped into dynamically linked library or shared library (.so/.dynlib/.dll)

Plugin loader is borrowed from the Ogre project :code:`DynLib.h`.

It is also possible to install plugins if they are build statically.
All plugins installed into the system should have unique id.

* :cpp:func:`Gsage::GsageFacade::loadPlugin` can be used to install plugin from a shared library.
* :cpp:func:`Gsage::GsageFacade::installPlugin` can be called with instantiated plugin object.
* :cpp:func:`Gsage::GsageFacade::uninstallPlugin` removes installed plugin.
* :cpp:func:`Gsage::GsageFacade::unloadPlugin` unloads shared library.


Writing a Plugin
----------------

Each plugin should implement :cpp:class:`IPlugin` interface.

Abstract Methods
^^^^^^^^^^^^^^^^

* :cpp:func:`Gsage::IPlugin::getName` - this method should return unique string id of the plugin.
* :cpp:func:`Gsage::IPlugin::installImpl` - should contain plugin initialization logic.
* :cpp:func:`Gsage::IPlugin::uninstallImpl` - should contain plugin deinitialization logic.

:code:`installImpl` can manipulate facade in any way as a :code:`IPlugin` has pointer to it.
:code:`uninstallImpl` should properly uninstall plugin from the facade, for example remove the system.

.. important::
  Though it is possible to define bindings in the :code:`installImpl` method, there is another method for this: :cpp:func:`Gsage::IPlugin::setupLuaBindings`.
  Bindings may work properly if defined in :code:`installImpl`, but if Lua state will be recreated by the :cpp:class:`LuaInterface`, bindings will be lost.

While implementing a plugin, you should also add :code:`extern "C"` methods, which will be called from the :code:`GsageFacade` after plugin
is loaded:

* :code:`dllStartPlugin`.
* :code:`dllStopPlugin`.

Example:

.. code-block:: cpp

  // from OgrePlugin.h

  ...

  #if GSAGE_PLATFORM == GSAGE_WIN32
  #ifdef PLUGIN_EXPORT
  #define PluginExport __declspec (dllexport)
  #else
  #define PluginExport __declspec (dllimport)
  #endif
  #else
  #define PluginExport
  #endif

  ...

.. code-block:: cpp

  // from OgrePlugin.cpp
  ...

  bool OgrePlugin::installImpl()
  {
    mEngine->addSystem<OgreRenderSystem>();
    mEngine->addSystem<RecastMovementSystem>();
    return true;
  }

  void OgrePlugin::uninstallImpl()
  {
    mEngine->removeSystem("render");
    mEngine->removeSystem("movement");
  }

  OgrePlugin* ogrePlugin = NULL;

  extern "C" bool PluginExport dllStartPlugin(GsageFacade* facade)
  {
    if(ogrePlugin != NULL)
    {
      return false;
    }
    ogrePlugin = new OgrePlugin();
    return facade->installPlugin(ogrePlugin);
  }

  extern "C" bool PluginExport dllStopPlugin(GsageFacade* facade)
  {
    if(ogrePlugin == NULL)
      return true;

    bool res = facade->uninstallPlugin(ogrePlugin);
    if(!res)
      return false;
    delete ogrePlugin;
    ogrePlugin = NULL;
    return true;
  }

  ...

Set Up Lua Bindings
^^^^^^^^^^^^^^^^^^^

* :cpp:func:`Gsage::IPlugin::setupLuaBindings` - should contain all Lua bindings. This method will be called again if :code:`lua_State` will be recreated.

Example:

.. code-block:: cpp

  ...

  void OgrePlugin::setupLuaBindings() {
    if (mLuaInterface && mLuaInterface->getState())
    {
      sol::state_view lua = *mLuaInterface->getSolState();

      // Ogre Wrappers

      lua.new_usertype<OgreObject>("OgreObject",
          "type", sol::property(&OgreObject::getType)
      );

      ...

      lua.new_usertype<Ogre::Quaternion>("Quaternion",
          sol::constructors<sol::types<const Ogre::Real&, const Ogre::Real&, const Ogre::Real&, const Ogre::Real&>, sol::types<const Ogre::Radian&, const Ogre::Vector3&>>(),
          "w", &Ogre::Quaternion::w,
          "x", &Ogre::Quaternion::x,
          "y", &Ogre::Quaternion::y,
          "z", &Ogre::Quaternion::z,
          sol::meta_function::multiplication, (Ogre::Quaternion(Ogre::Quaternion::*)(const Ogre::Quaternion&)const)  &Ogre::Quaternion::operator*
      );

      lua.new_usertype<Ogre::Radian>("Radian",
          sol::constructors<sol::types<float>>()
      );

      lua.new_usertype<OgreSelectEvent>("OgreSelectEvent",
          sol::base_classes, sol::bases<Event, SelectEvent>(),
          "intersection", sol::property(&OgreSelectEvent::getIntersection),
          "cast", cast<const Event&, const OgreSelectEvent&>
      );

      lua["Engine"]["render"] = &Engine::getSystem<OgreRenderSystem>;
      lua["Engine"]["movement"] = &Engine::getSystem<RecastMovementSystem>;

      lua["Entity"]["render"] = &Entity::getComponent<RenderComponent>;
      lua["Entity"]["movement"] = &Entity::getComponent<MovementComponent>;

      ...

    }
  }

  ...
