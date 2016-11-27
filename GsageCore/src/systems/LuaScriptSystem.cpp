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
#include "lua/LuaInterface.h"

extern "C" {
  #include "lua.h"
  #include "lauxlib.h"
  #include "lualib.h"
}


namespace Gsage {

  LuaScriptSystem::LuaScriptSystem()
    : mState(0)
    , mWorkdir(".")
  {
  }

  LuaScriptSystem::~LuaScriptSystem()
  {
    if(mState != 0)
    {
      delete mState;
    }
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
      sol::function initializeBtree = (*mState)["btree"]["initialize"].get<sol::function>();
      sol::table btree = initializeBtree(component->getOwner()->getId(), component->getBehavior());
      component->setBtree(btree);
      sol::table context = btree["context"];
      component->setData(context);

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
    if(mState && mState->lua_state() == L)
      return true;

    mState = new sol::state_view(L);
    return true;
  }

  void LuaScriptSystem::update(const double& time)
  {
    if(!mState)
      return;

    for(sol::protected_function& function : mUpdateListeners)
    {
      auto res = function(time);
      if(!res.valid()) {
        sol::error err = res;
        removeUpdateListener(function);
        LOG(ERROR) << "Failed to call update listener: " << err.what() << "\n\t" << lua_tostring(mState->lua_state(), -1) << ", force unsubscribe";
      }
    }

    ComponentStorage<ScriptComponent>::update(time);
  }

  void LuaScriptSystem::updateComponent(ScriptComponent* component, Entity* entity, const double& time)
  {
    if(!component->getSetupExecuted())
    {
      LOG(INFO) << entity->getId() << " executing script";
      runScript(component, component->getSetupScript());
      component->setSetupExecuted(true);
    }

    if(component->hasBehavior())
    {
      sol::table& btree = component->getBtree();
      btree["update"](btree, time);
    }
  }

  bool LuaScriptSystem::removeComponent(ScriptComponent* component)
  {
    const std::string id = component->getOwner()->getId();
    bool stopped = true;
    if(mState && component->hasBehavior())
    {
      sol::function stopBtree = (*mState)["btree"]["deinitialize"].get<sol::protected_function>();
      if(!runScript(component, component->getTearDownScript()))
      {
        LOG(ERROR) << "Failed to tear down script component";
        stopped = false;
      }
      else
      {
        auto res = stopBtree(id);
        if(!res.valid()) {
          sol::error err = res;
          LOG(ERROR) << "Failed to stop btree " << err.what();
        }
      }
    }

    bool res = ComponentStorage<ScriptComponent>::removeComponent(component);
    return res && stopped;
  }

  bool LuaScriptSystem::runScript(ScriptComponent* component, const std::string& script)
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

    try {
      auto res = mState->script(scriptData.c_str());
      if(!res.valid())
      {
        sol::error err = res;
        LOG(ERROR) << "Failed to execute lua script " << err.what();
        return false;
      }

      sol::object r = res.get<sol::object>();
      if(r.get_type() != sol::type::function) {
        return true;
      }

      // If script returns a function, it means that it want's to have a sandbox

      sol::protected_function callback = r;
      auto callResult = callback(EntityProxy(component->getOwner()->getId(), mEngine));
      if(!callResult.valid()) {
        sol::error err = callResult;
        LOG(ERROR) << "Failed to execute lua script " << err.what();
        return false;
      }
    } catch(sol::error e) {
      LOG(ERROR) << "Failed to execute lua script " << e.what();
      return false;
    }

    return true;
  }

  bool LuaScriptSystem::addUpdateListener(const sol::object& function)
  {
    if(std::find(mUpdateListeners.begin(), mUpdateListeners.end(), function) != mUpdateListeners.end())
      return true;

    if(function.get_type() == sol::type::function) {
      mUpdateListeners.push_back(function.as<sol::protected_function>());
    } else {
      LOG(WARNING) << "Tried to add update listener object of unsupported type";
      return false;
    }

    return true;
  }

  bool LuaScriptSystem::removeUpdateListener(const sol::object& object)
  {
    sol::protected_function function = object.as<sol::protected_function>();
    UpdateListeners::iterator element = std::find(mUpdateListeners.begin(), mUpdateListeners.end(), function);
    if(element == mUpdateListeners.end())
      return false;

    mUpdateListeners.erase(element);
    return true;
  }
}
