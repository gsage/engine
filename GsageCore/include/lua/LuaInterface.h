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

#ifndef _LuaInterface_H_
#define _LuaInterface_H_

extern "C" {
  #include "lua.h"
  #include "lauxlib.h"
  #include "lualib.h"
}

#include "EventSubscriber.h"
#include "GsageDefinitions.h"
#include "Dictionary.h"
#include "Engine.h"

namespace sol
{
  class state_view;
}

namespace Ogre
{
  class Vector3;
}

namespace Gsage
{

  template<class From, class To>
  static To cast(From f) {
    return static_cast<To>(f);
  }

  class Entity;

  class GsageFacade;
  class GameDataManager;
  class LuaEventProxy;

  class EntityProxy;

  class EngineProxy
  {
    public:
      EngineProxy(Engine* engine) : mEngine(engine) {}
      virtual ~EngineProxy() {}

      /**
       * Get entity by id
       * @param id Entity id
       */
      EntityProxy* getEntity(const std::string& id);
    private:
      Engine* mEngine;
  };

  /**
   * Lua entity wrapper, not to be created in C++
   */
  class EntityProxy
  {
    public:
      EntityProxy(const std::string& entityId, Engine* engine);
      virtual ~EntityProxy();
      /**
       * Get component
       */
      template<class C>
      C* getComponent()
      {
        return mEngine->getComponent<C>(mEntityId);
      }
      /**
       * Get id of underlying entity
       */
      const std::string& getId();
      /**
       * Checks if there is entity instance with specified id
       */
      bool isValid();
    private:
      Engine* mEngine;
      std::string mEntityId;
  };

  /**
   * Lua data manager wrapper
   */
  class DataManagerProxy
  {
    public:
      DataManagerProxy(GameDataManager* instance);
      virtual ~DataManagerProxy();
      /**
       * @see GameDataManager::createEntity(const std::string&, const Dictionary& params)
       */
      Entity* createEntity(const std::string& templateFile, Dictionary params);
      /**
       * @see GameDataManager::createEntity(const std::string&)
       */
      Entity* createEntity(const std::string& json);
    private:
      GameDataManager* mInstance;
  };

  /**
   * Contains all lua bindings
   */
  class LuaInterface
  {
    public:
      LuaInterface(GsageFacade* instance, const std::string& resourcePath = "./resources");
      virtual ~LuaInterface();
      /**
       * Attaching to/creating the lua state, does all class bindings
       *
       * @param L State to use. Will be created if not defined as a parameter
       */
      bool initialize(lua_State* L = 0);

      /**
       * Get lua state to which this interface is attached
       */
      lua_State* getState();
      /**
       * Run lua script
       *
       * @param path Path to the file with script
       */
      bool runScript(const std::string& path);

      /**
       * Returns sol2 state view
       * @returns pointer to the state, NULL if is not initialized
       */
      sol::state_view* getSolState();

      /**
       * Set resource path, which is helps lua to find all the scripts
       *
       * @param path Path to folder with resources
       */
      void setResourcePath(const std::string& path);
    private:
      void closeLuaState();
      GsageFacade* mInstance;
      std::string mResourcePath;

      EngineProxy* mEngineProxy;
      DataManagerProxy* mDataManagerProxy;
      LuaEventProxy* mEventProxy;

      lua_State* mState;
      sol::state_view* mStateView;
      bool mStateCreated;
  };
}

#endif
