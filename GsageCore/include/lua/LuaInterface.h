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

#include "EventSubscriber.h"
#include "GsageDefinitions.h"
#include "DataProxy.h"
#include "Engine.h"
#include "sol.hpp"

struct lua_State;

namespace Ogre
{
  class Vector3;
}

namespace sol
{
  // Gsage::DataProxy can be converted to the lua table in a very simple way
  template <>
  struct lua_type_of<Gsage::DataProxy> : std::integral_constant<sol::type, sol::type::table> {};

  // Now, specialize various stack structures
  namespace stack {

    template <>
    struct checker<Gsage::DataProxy> {
      template <typename Handler>
        static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
          // indices can be negative to count backwards from the top of the stack,
          // rather than the bottom up
          // to deal with this, we adjust the index to
          // its absolute position using the lua_absindex function
          int absolute_index = lua_absindex(L, index);
          // Check first and second second index for being the proper types
          bool success = stack::check<sol::table>(L, absolute_index, handler);
          tracking.use(1);
          return success;
        }
    };

    template <>
    struct getter<Gsage::DataProxy> {
      static Gsage::DataProxy get(lua_State* L, int index, record& tracking) {
        int absolute_index = lua_absindex(L, index);
        sol::table t = stack::get<sol::table>(L, absolute_index);
        Gsage::DataProxy dp = Gsage::DataProxy::create(t);
        tracking.use(1);
        return dp;
      }
    };

    template <>
    struct pusher<Gsage::DataProxy> {
      static int push(lua_State* L, const Gsage::DataProxy& value) {
        sol::table t = sol::state_view(L).create_table();
        Gsage::DataProxy dp = Gsage::DataProxy::wrap(t);
        value.dump(dp);
        t.push();
        return 1;
      }
    };
  }
} // sol

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
       * @see GameDataManager::createEntity(const DataProxy& params)
       */
      Entity* createEntity(DataProxy params);
      /**
       * @see GameDataManager::createEntity(const std::string&, const DataProxy& params)
       */
      Entity* createEntity(const std::string& templateFile, DataProxy params);
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

      /**
       * Register new event type
       *
       * @param name Lua event class name
       * @param handler Name of handler to create
       * @param ut Bindings
       */
      template<class T>
      void registerEvent(const std::string& name, const std::string& handler, sol::usertype<T> ut);
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
