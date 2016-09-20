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
#include "ObjectPool.h"
#include "DynLib.h"

#include "Engine.h"
#include "EventSubscriber.h"
#include "FileLoader.h"

// forward declarations ------

struct lua_State;

namespace Ogre
{
  class Timer;
}

namespace Gsage
{
  class EngineSystem;
  class UIManager;
  class GameDataManager;
  class OgreRenderSystem;
  class RecastMovementSystem;
  class CombatSystem;
  class OisInputListener;
  class LuaInterface;
  class IPlugin;

  /**
   * Update listener interface
   */
  class UpdateListener
  {
    public:
      /**
       * Abstract update method that should be implemented in descendants
       * @param time Time in seconds
       */
      virtual void update(const float& time) = 0;
  };

  // ---------------------------

  /**
   * One class to initialize all systems and parts of the engine
   */
  class GsageFacade : public EventSubscriber<GsageFacade>
  {
    public:
      typedef bool(*INSTALL_PLUGIN)(GsageFacade*);
      typedef bool(*UNINSTALL_PLUGIN)(GsageFacade*);

      static const std::string LOAD;
      static const std::string RESET;

      GsageFacade();
      virtual ~GsageFacade();
      /**
       * Initializes engine, configures systems
       *
       * @param gsageConfigPath Path to the file with facade settings
       */
      virtual bool initialize(const std::string& gsageConfigPath, const std::string& resourcePath, Dictionary* configOverride = 0, FileLoader::Encoding configEncoding = FileLoader::Json);
      /**
       * Register new system in the engine
       */
      template<typename T>
      T* addSystem()
      {
        return mEngine.addSystem<T>();
      }
      /**
       * Register new system in the engine
       *
       * @param id System id
       * @param system System pointer
       */
      void addSystem(const std::string& id, EngineSystem* system);
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
       * Sets UIManager to handle UI related logic
       *
       * @param value Abstract UI manager
       */
      void setUIManager(UIManager* value);
      /**
       * Stops the engine
       */
      void halt();
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
       */
      void setLuaState(lua_State* L);

      /**
       * Check if facade was stopped
       */
      bool isStopped() const { return mStopped; }

    protected:
      bool onEngineHalt(EventDispatcher* sender, const Event& event);

      bool mStarted;
      bool mStartupScriptRun;
      std::string mStartupScript;
      std::atomic<bool> mStopped;

      Dictionary mConfig;

      Ogre::Timer* mTimer;
      unsigned long mTimeSinceLastUpdate;

      Engine mEngine;
      GameDataManager* mGameDataManager;
      UIManager* mUIManager;
      LuaInterface* mLuaInterface;
      lua_State* mLuaState;

      typedef std::map<std::string, DynLib*> Libraries;
      Libraries mLibraries;

      typedef std::map<std::string, IPlugin*> Plugins;
      Plugins mInstalledPlugins;

      typedef std::vector<UpdateListener*> UpdateListeners;
      UpdateListeners mUpdateListeners;
  };
}
#endif
