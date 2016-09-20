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

#include "systems/LuaScriptSystem.h"
#include "components/ScriptComponent.h"
#include "Entity.h"
#include "Logger.h"
#include "Engine.h"

extern "C" {
  #include "lua.h"
  #include "lauxlib.h"
  #include "lualib.h"
}

#include <luabind/adopt_policy.hpp>
#include <luabind/object.hpp>

namespace Gsage {

  LuaScriptSystem::LuaScriptSystem()
    : mState(0)
    , mWorkdir(".")
  {
  }

  LuaScriptSystem::~LuaScriptSystem()
  {
  }

  bool LuaScriptSystem::initialize(const Dictionary& settings) {
    mWorkdir = mEngine->env().get("workdir", ".");
    EngineSystem::initialize(settings);
    return true;
  }

  bool LuaScriptSystem::fillComponentData(ScriptComponent* component, const Dictionary& data)
  {
    bool createdComponent = ComponentStorage<ScriptComponent>::fillComponentData(component, data);
    if(!createdComponent)
      return false;

    if(component->getBehavior().empty())
      return true;

    if(mState)
    {
      luabind::object initializeBtree = luabind::globals(mState)["btree"]["initialize"];
      if(initializeBtree.is_valid())
      {
        luabind::object btree = initializeBtree(component->getOwner()->getId(), component->getBehavior());
        component->setBtree(btree);
        luabind::object context = btree["context"];
        component->setData(context);
      }
      else
      {
        LOG(ERROR) << "Failed to initialize behavior tree, check that resources/scripts/behaviors.lua was loaded";
        return false;
      }
      LOG(INFO) << "Successfuly registered script component for entity " << component->getOwner()->getId();
    }
    else
    {
      LOG(ERROR) << "Failed to register script";
      return false;
    }
    return true;
  }

  bool LuaScriptSystem::setLuaState(lua_State* L)
  {
    if(mState == L)
      return true;

    mState = L;
    return true;
  }

  void LuaScriptSystem::update(const double& time)
  {
    if(!mState)
      return;

    for(luabind::object& function : mUpdateListeners)
    {
      try
      {
        if(!function.is_valid())
        {
          LOG(WARNING) << "Attempt to call invalid function, function removed from listeners";
          removeUpdateListener(function);
        }
        else
        {
          function(time);
        }
      }
      catch(luabind::error e)
      {
        removeUpdateListener(function);
        LOG(ERROR) << "Failed to call update listener: " << e.what() << "\n\t" << lua_tostring(mState, -1) << ", force unsubscribe";
      }
    }

    ComponentStorage<ScriptComponent>::update(time);
  }

  void LuaScriptSystem::updateComponent(ScriptComponent* component, Entity* entity, const double& time)
  {
    if(!component->getSetupExecuted())
    {
      LOG(INFO) << entity->getId() << " executing script";
      runScript(component->getSetupScript());
      component->setSetupExecuted(true);
    }

    if(component->hasBehavior())
    {
      luabind::object& btree = component->getBtree();
      btree["update"](btree, time);
    }
  }

  bool LuaScriptSystem::removeComponent(ScriptComponent* component)
  {
    const std::string id = component->getOwner()->getId();
    bool stopped = true;
    if(mState && component->hasBehavior())
    {
      luabind::object stopBtree = luabind::globals(mState)["btree"]["deinitialize"];
      if(!stopBtree.is_valid() || !runScript(component->getTearDownScript()))
      {
        LOG(ERROR) << "Failed to tear down script component";
        stopped = false;
      }
      else
        stopBtree(id);
    }

    bool res = ComponentStorage<ScriptComponent>::removeComponent(component);
    return res && stopped;
  }

  bool LuaScriptSystem::runScript(const std::string& script)
  {
    std::vector<std::string> parts = split(script, ':');
    std::string scriptData;
    if (parts.size() == 2 && parts[0] == "@File")
    {
      std::string filename = mWorkdir + GSAGE_PATH_SEPARATOR + parts[1];
      std::ifstream stream(filename);
      stream.seekg(0, std::ios::end);
      long int size = stream.tellg();
      if(size == -1)
      {
        LOG(ERROR) << "Failed to read script file: " << filename;
        return false;
      }

      scriptData.reserve(size);
      stream.seekg(0, std::ios::beg);
      try
      {
        scriptData.assign((std::istreambuf_iterator<char>(stream)),
            std::istreambuf_iterator<char>());
      }
      catch(std::ifstream::failure e)
      {
        LOG(ERROR) << "Failed to read script file: " << script;
        stream.close();
        return false;
      }

      stream.close();
    } else {
      scriptData = script;
    }

    int res = luaL_dostring(mState, scriptData.c_str());

    if(res != 0)
    {
      LOG(ERROR) << "Failed to execute lua script:\n" << lua_tostring(mState, -1);
      return false;
    }
    return true;
  }

  bool LuaScriptSystem::addUpdateListener(const luabind::object& function)
  {
    if(std::find(mUpdateListeners.begin(), mUpdateListeners.end(), function) != mUpdateListeners.end())
      return true;

    if(luabind::type(function) == LUA_TFUNCTION)
    {
      mUpdateListeners.push_back(function);
    }
    else
    {
      LOG(WARNING) << "Tried to add update listener object of unsupported type";
      return false;
    }

    return true;
  }

  bool LuaScriptSystem::removeUpdateListener(const luabind::object& function)
  {
    UpdateListeners::iterator element = std::find(mUpdateListeners.begin(), mUpdateListeners.end(), function);
    if(element == mUpdateListeners.end())
      return false;

    mUpdateListeners.erase(element);
    return true;
  }
}
