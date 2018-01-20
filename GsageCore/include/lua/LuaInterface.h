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
#include "sol_forward.hpp"

#include "lua/LuaEventProxy.h"
#include "lua/LuaEventConnection.h"

#include <stdexcept>

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
        sol::table t;
        if(value.getWrappedType() == Gsage::DataWrapper::LUA_TABLE) {
          // access the lua table directly from the wrapper, if it's already a lua table
          t = value.getWrapper<Gsage::DataWrapper::LUA_TABLE>()->getObject();
        } else {
          t = sol::state_view(L).create_table();
          Gsage::DataProxy dp = Gsage::DataProxy::wrap(t);
          value.dump(dp);
        }
        t.push(L);
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

  /**
   * Easylogging++ log listeners manager
   */
  class LogProxy
  {
    public:
      LogProxy();
      virtual ~LogProxy();
      /**
       * Subscribe lua callback to logs.
       * @param name callback name
       * @param function sol::protected_function
       */
      void subscribe(const std::string& name, sol::protected_function function);

      /**
       * Unsubscrube lua callback from logs.
       * @param name callback name
       */
      void unsubscribe(const std::string& name);
    private:
      class LogSubscriber : public el::LogDispatchCallback
      {
        public:
          LogSubscriber() {};
          virtual ~LogSubscriber() {};

          /**
           * Set wrapped function
           * @param wrapped sol::protected_function
           */
          void setWrappedFunction(sol::protected_function wrapped);
        protected:
          void handle(const el::LogDispatchData* data);
        private:
          sol::protected_function mWrapped;
          bool mReady;
      };
  };

  /**
   * Lua property description.
   */
  class PropertyDescription
  {
    public:
      PropertyDescription(const std::string& name, const std::string& docstring);
      virtual ~PropertyDescription() {};

      /**
       * Get name.
       */
      const std::string& getName() const;

      /**
       * Get docstring.
       */
      const std::string& getDocstring() const;
    private:
      std::string mName;
      std::string mDocstring;
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
       * Run lua packager script. It is executed in a separate temporary lua state.
       *
       * @param path packager script path
       * @param dependencies dependencies list
       * @returns true if succeed
       */
      bool runPackager(const std::string& script, const DataProxy& dependencies);
      /**
       * Run lua script
       *
       * @param path Path to the file with script
       * @param state sol::state to run script into
       */
      bool runScript(const std::string& path, sol::state_view* state = 0);

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
      void registerEvent(Event::ConstType name, const std::string& handler, sol::usertype<T> ut)
      {
        if(!std::is_base_of<Event, T>::value)
        {
          LOG(ERROR) << "Event not registered: specified type is not derived from Event";
          return;
        }

        if(!mStateView)
        {
          LOG(ERROR) << "Event not registered: no lua state";
          return;
        }

        sol::state_view& lua = *mStateView;

        lua.set_usertype(name, ut);
        lua["LuaEventProxy"][handler] = &LuaEventProxy::addEventListener<T>;
      }

      template<typename C, typename... Args>
      void registerEvent(Event::ConstType name, const std::string& handler, Args&&... args)
      {
        if(!std::is_base_of<Event, C>::value)
        {
          LOG(ERROR) << "Event not registered: specified type is not derived from Event";
          return;
        }

        if(!mStateView)
        {
          LOG(ERROR) << "Event not registered: no lua state";
          return;
        }

        sol::state_view& lua = *mStateView;
        lua.new_usertype<C>(name, std::forward<Args>(args)...);
        lua["LuaEventConnection"][handler] = &LuaEventConnection::bind<C>;
        lua["LuaEventProxy"][handler] = &LuaEventProxy::addEventListener<C>;
      }
    private:
      void closeLuaState();
      GsageFacade* mInstance;
      std::string mResourcePath;

      LuaEventProxy* mEventProxy;

      lua_State* mState;
      sol::state_view* mStateView;
      bool mStateCreated;
  };
}

#endif
