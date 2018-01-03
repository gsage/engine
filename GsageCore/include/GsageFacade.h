/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2016 Artem Chernyshev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef _GsageFacade_H_
#define _GsageFacade_H_

#include <atomic>
#include <chrono>
#include <list>
#include "ObjectPool.h"
#include "DynLib.h"

#include "UpdateListener.h"
#include "Engine.h"
#include "EventSubscriber.h"
#include "FileLoader.h"
#include "input/InputFactory.h"
#include "systems/SystemManager.h"
#include "WindowManager.h"
#include "WindowManagerFactory.h"

// forward declarations ------

struct lua_State;

namespace Gsage
{
  class EngineSystem;
  class UIManager;
  class GameDataManager;
  class CombatSystem;
  class OisInputListener;
  class LuaInterface;
  class IPlugin;

  // ---------------------------

  /**
   * One class to initialize all systems and parts of the engine
   */
  class GsageFacade : public EventSubscriber<GsageFacade>
  {
    public:
      typedef bool(*INSTALL_PLUGIN)(GsageFacade*);
      typedef bool(*UNINSTALL_PLUGIN)(GsageFacade*);

      /**
       * Event id dispatched when level is loaded
       */
      static const Event::Type LOAD;

      /**
       * Event id dispatched when level is unloaded
       */
      static const Event::Type RESET;

      /**
       * Event id dispatched before level unload
       */
      static const Event::Type BEFORE_RESET;

      GsageFacade();
      virtual ~GsageFacade();
      /**
       * Initializes engine, configures systems
       *
       * @param gsageConfigPath Path to the file with facade settings
       */
      virtual bool initialize(const std::string& gsageConfigPath, const std::string& resourcePath, DataProxy* configOverride = 0, FileLoader::Encoding configEncoding = FileLoader::Json);
      /**
       * Add new system in the engine
       */
      template<typename T>
      T* addSystem()
      {
        return mEngine.addSystem<T>();
      }
      /**
       * Add new system in the engine
       *
       * @param id System id
       * @param system System pointer
       */
      void addSystem(const std::string& id, EngineSystem* system);

      /**
       * @copydoc Gsage::SystemManager::registerSystem
       */
      template<typename T>
      bool registerSystemFactory(const std::string& id)
      {
        return mSystemManager.registerSystem<T>(id);
      }

      /**
       * @copydoc Gsage::SystemManager::registerSystem
       */
      template<typename T>
      bool registerSystemFactory()
      {
        return mSystemManager.registerSystem<T>();
      }

      /**
       * @copydoc Gsage::SystemManager::removeSystem
       */
      bool removeSystemFactory(const std::string& id)
      {
        return mSystemManager.removeSystem(id);
      }

      /**
       * @copydoc Gsage::SystemManager::removeSystem
       */
      template<class S>
      bool removeSystemFactory()
      {
        return mSystemManager.removeSystem<S>();
      }

      /**
       * Updates engine and all it's systems
       */
      virtual bool update();
      /**
       * Dumps game save
       *
       * @param name Save file name
       */
      bool dumpSave(const std::string& name);
      /**
       * Adds UIManager to handle UI related logic.
       * Engine can have several UI managers at the same time.
       *
       * @param value Abstract UI manager
       * @returns int handle of UIManager. Use it to remove UI manager from the system
       */
      int addUIManager(UIManager* value);
      /**
       * Remove UIManager by it's handle
       *
       * @param handle int handle returned from addUIManager call
       * @returns true if removed
       */
      bool removeUIManager(int handle);
      /**
       * Stops the engine
       */
      void shutdown(int exitCode = 0);
      /**
       * Get shutdown exit code
       */
      int getExitCode() const;
      /**
       * Unloads current scene from engine
       */
      virtual void reset();
      /**
       * Loads game save
       *
       * @param name Save file name
       */
      virtual bool loadSave(const std::string& name);
      /**
       * Loads game area, no character initialization will be done in this case
       *
       * @param name Area file name
       */
      virtual bool loadArea(const std::string& name);
      /**
       * Gets engine instance
       */
      Engine* getEngine() { return &mEngine; }
      /**
       * Gets save data manager
       */
      GameDataManager* getGameDataManager() { return mGameDataManager; }

      /**
       * Load plugin from dynamic library
       * @param path Path to the plugin
       */
      bool loadPlugin(const std::string& path);

      /**
       * Unload plugin
       * @param path Path to the plugin
       */
      bool unloadPlugin(const std::string& path);

      /**
       * Install plugin
       * @param plugin Plugin instance
       */
      bool installPlugin(IPlugin* plugin);

      /**
       * Remove plugin
       * @param plugin Plugin instance
       */
      bool uninstallPlugin(IPlugin* plugin);

      /**
       * Add object that is dependent of timer updates
       * @param listener Pointer to the UpdateListener
       */
      void addUpdateListener(UpdateListener* listener);

      /**
       * Set lua state to use in the lua interface and systems
       * @param lua_State Lua state pointer
       * @param initialize initialize lua interface and update ui managers
       */
      void setLuaState(lua_State* L, bool initialize = true);

      /**
       * Get lua state
       * @returns lua_State
       */
      lua_State* getLuaState();

      /**
       * Check if facade was stopped
       */
      bool isStopped() const { return mStopped; }

      /**
       * Register input factory
       *
       * @param id Unique id of the factory
       */
      template<class T>
      T* registerInputFactory(const std::string& id)
      {
        return mInputManager.registerFactory<T>(id);
      }

      /**
       * Remove input factory
       *
       * @param id Unique id of the factory
       */
      void removeInputFactory(const std::string& id);

      /**
       * Create system using system manager
       * @param systemID system ID
       */
      bool createSystem(const std::string& systemID);

      /**
       * Register new window manager factory
       * @param id factory name
       */
      template<class C>
      void registerWindowManager(const std::string& id)
      {
        mWindowManagerFactory.registerWindowManager<C>(id);
      }

      /**
       * Unregister window manager factory
       * @param id factory name
       * @returns true if the factory was removed
       */
      bool removeWindowManager(const std::string& id);

      /**
       * Get active window manager
       * @returns current window manager pointer
       */
      WindowManagerPtr getWindowManager();

      /**
       * Handle window manager events, proxy them to the engine
       * @param sender event sender
       * @param event event to proxy
       */
      bool handleWindowManagerEvent(EventDispatcher* sender, const Event& event);
    protected:
      bool onEngineShutdown(EventDispatcher* sender, const Event& event);

      bool mStarted;
      bool mStartupScriptRun;
      std::string mStartupScript;
      std::atomic<bool> mStopped;

      DataProxy mConfig;

      std::chrono::high_resolution_clock::time_point mPreviousUpdateTime;

      Engine mEngine;
      InputManager mInputManager;
      SystemManager mSystemManager;
      GameDataManager* mGameDataManager;
      WindowManagerPtr mWindowManager;
      WindowManagerFactory mWindowManagerFactory;

      LuaInterface* mLuaInterface;
      lua_State* mLuaState;

      typedef std::map<std::string, DynLib*> Libraries;
      Libraries mLibraries;

      typedef std::list<std::string> PluginOrder;
      PluginOrder mPluginOrder;

      typedef std::map<std::string, IPlugin*> Plugins;
      Plugins mInstalledPlugins;

      typedef std::vector<UpdateListener*> UpdateListeners;
      UpdateListeners mUpdateListeners;

      typedef std::map<long, UIManager*> UIManagers;
      UIManagers mUIManagers;

      int mExitCode;
  };
}
#endif
