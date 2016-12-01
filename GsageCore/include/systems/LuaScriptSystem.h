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
#include <sol.hpp>
#include "systems/SystemFactory.h"
#include "lua/LuaInterface.h"
#include "Engine.h"

struct lua_State;

namespace Gsage
{
  class EntityComponent;
  class Entity;
  class ScriptComponent;

  class LuaScriptSystem : public ComponentStorage<ScriptComponent>
  {
    public:
      LuaScriptSystem();
      virtual ~LuaScriptSystem();

      /**
       * @copydoc EngineSystem::initialize()
       */
      virtual bool initialize(const Dictionary& settings);
      /**
       * Registers the script in the script system
       * @param component ScriptComponent that contains all stuff
       * @param data Dictionary with the script serialized data
       */
      bool fillComponentData(ScriptComponent* component, const Dictionary& data);

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
       * Add lua function which will be called on each update of the system
       * @param function Sol function object, only LUA_TFUNCTION will be added as listener
       */
      bool addUpdateListener(const sol::object& function);

      /**
       * Remove lua function from update listeners list
       * @param function Sol function object
       */
      bool removeUpdateListener(const sol::object& function);

    private:

      sol::state_view* mState;

      typedef std::vector<sol::protected_function> UpdateListeners;
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
      EngineSystem* create(Engine* engine)
      {
        lua_State* s = mLuaInterface->getState();
        if(s == 0) {
          return 0;
        }

        LuaScriptSystem* script = engine->addSystem<LuaScriptSystem>();
        if(!script){
          return 0;
        }

        script->setLuaState(s);
        return script;
      }
    private:
      LuaInterface* mLuaInterface;
  };
}
#endif
