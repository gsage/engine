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

#ifndef _LuaScriptSystem_H_
#define _LuaScriptSystem_H_

#include "ComponentStorage.h"
#include "components/ScriptComponent.h"
#include "systems/SystemFactory.h"
#include "lua/LuaInterface.h"
#include "Engine.h"
#include "sol_forward.hpp"

struct lua_State;

namespace Gsage
{
  class EntityComponent;
  class Entity;
  class ScriptComponent;

  class LuaScriptSystem : public ComponentStorage<ScriptComponent>
  {
    public:
      static const std::string ID;
      LuaScriptSystem();
      virtual ~LuaScriptSystem();

      /**
       * @copydoc EngineSystem::initialize()
       */
      virtual bool initialize(const DataProxy& settings);

      /**
       * @copydoc EngineSystem::configure()
       */
      virtual void configUpdated();

      /**
       * Registers the script in the script system
       * @param component ScriptComponent that contains all stuff
       * @param data DataProxy with the script serialized data
       */
      bool fillComponentData(ScriptComponent* component, const DataProxy& data);

      /**
       * Initialize script system with the specified lua state
       * Should be called only once
       *
       * @param L lua state
       */
      bool setLuaState(lua_State* L);

      /**
       * Update the system
       *
       * @param time Elapsed time
       */
      void update(const double& time);

      /**
       * Override script component update logic
       *
       * @param component ScriptComponent
       * @param entity Entity that owns the component
       * @param time Elapsed time
       */
      void updateComponent(ScriptComponent* component, Entity* entity, const double& time);

      /**
       * Called on component remove
       *
       * @param component ScriptComponent to remove
       */
      bool removeComponent(ScriptComponent* component);

      /**
       * Run script in the LuaScriptSystem
       *
       * @param component Target component
       * @param script Script or file with it
       * @returns run result, false if error happened
       */
      bool runScript(ScriptComponent* component, const std::string& script);

      /**
       * Run script in the LuaScriptSystem
       *
       * @param script Script string or file to run
       * @returns True if succeed
       */
      bool runScript(const std::string& script);

      /**
       * Run function in the LuaScriptSystem for the component
       *
       * @param component Target component
       * @param func Function to run
       * @returns true if succeed
       */
      bool runFunction(ScriptComponent* component, sol::function func);

      /**
       * Add lua function which will be called on each update of the system
       * @param function Sol function object, only LUA_TFUNCTION will be added as listener
       * @param global Flag to tell script system that the callback is a global function so it won't be
       *               removed on LuaScriptSystem::unloadComponents call
       * @returns true if passed type is a function
       */
      bool addUpdateListener(const sol::object& function, bool global = false);

      /**
       * Remove lua function from update listeners list
       * @param function Sol function object
       */
      bool removeUpdateListener(const sol::object& function);

      /**
       * Unload components.
       */
      void unloadComponents();
    private:
      struct Listener
      {
        Listener(sol::function function, bool global)
          : function(function)
          , global(global)
        {}

        sol::function function;
        bool global;
      };

      std::string getScriptData(const std::string& data);

      sol::state_view* mState;

      typedef std::vector<Listener> UpdateListeners;
      UpdateListeners mUpdateListeners;

      std::string mWorkdir;
  };

  class LuaScriptSystemFactory : public SystemFactory
  {
    public:
      LuaScriptSystemFactory(LuaInterface* luaInterface)
        : mLuaInterface(luaInterface)
      {}

      /**
       * @copydoc Gsage::SystemFactory::create
       */
      EngineSystem* create(Engine* engine, bool configure = true)
      {
        lua_State* s = mLuaInterface->getState();
        if(s == 0) {
          return 0;
        }

        LuaScriptSystem* script = engine->addSystem<LuaScriptSystem>(configure);
        if(!script){
          return 0;
        }
        script->setKind(LuaScriptSystem::ID);
        script->setLuaState(s);
        return script;
      }

      const std::string& getSystemType() const
      {
        return LuaScriptSystem::type::SYSTEM;
      }
    private:
      LuaInterface* mLuaInterface;
  };
}
#endif
