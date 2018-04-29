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
#include "lua.hpp"

namespace Gsage {

  const std::string LuaScriptSystem::ID = "lua";

  LuaScriptSystem::LuaScriptSystem()
    : mState(0)
    , mWorkdir(".")
  {
    mSystemInfo.put("type", LuaScriptSystem::ID);
  }

  LuaScriptSystem::~LuaScriptSystem()
  {
  }

  bool LuaScriptSystem::initialize(const DataProxy& settings) {
    mWorkdir = mEngine->env().get("workdir", ".");
    EngineSystem::initialize(settings);
    return true;
  }

  void LuaScriptSystem::configUpdated() {
    EngineSystem::configUpdated();
    std::pair<DataProxy, bool> hooks = mConfig.get<DataProxy>("hooks");
    if(hooks.second) {
      for(auto& pair : hooks.first) {
        std::string hook = pair.second.getValueOptional<std::string>("");
        if(hook.empty()) {
          LOG(WARNING) << "Got empty hook " << pair.first;
          continue;
        }

        if(!runScript(hook))
        {
          LOG(ERROR) << "Startup hook run failed";
        }
      }
    }
  }

  bool LuaScriptSystem::fillComponentData(ScriptComponent* component, const DataProxy& data)
  {
    bool createdComponent = ComponentStorage<ScriptComponent>::fillComponentData(component, data);
    if(!createdComponent)
      return false;

    if(component->getBehavior().empty())
    {
      sol::table t = mState->create_table();
      component->setData(t);
      return true;
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

    for(Listener& listener : mUpdateListeners)
    {
      auto res = listener.function(time);
      if(!res.valid()) {
        sol::error err = res;
        removeUpdateListener(listener.function);
        LOG(ERROR) << "Failed to call update listener: " << err.what() << "\n\t" << lua_tostring(mState->lua_state(), -1) << ", force unsubscribe";
      }
    }

    ComponentStorage<ScriptComponent>::update(time);
  }

  void LuaScriptSystem::updateComponent(ScriptComponent* component, Entity* entity, const double& time)
  {
    if(!component->getSetupExecuted())
    {
      runFunction(component, component->getSetupFunction());
      LOG(INFO) << entity->getId() << " executing script";
      runScript(component, component->getSetupScript());
      component->setSetupExecuted(true);
    }

    if(!component->getBehavior().empty() && component->getBtree() == sol::lua_nil) {
      sol::function initializeBtree = (*mState)["btree"]["initialize"].get<sol::function>();
      sol::table btree = initializeBtree(component->getOwner()->getId(), component->getBehavior());
      component->setBtree(btree);
      sol::table context = btree["context"];

      component->setData(context);

      LOG(INFO) << "Successfuly registered script component for entity " << component->getOwner()->getId();
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
      sol::function stopBtree = (*mState)["btree"]["deinitialize"].get<sol::function>();
      auto res = stopBtree(id);
      if(!res.valid()) {
        sol::error err = res;
        LOG(ERROR) << "Failed to stop btree " << err.what();
      }


      if(!runScript(component, component->getTearDownScript()))
      {
        LOG(ERROR) << "Failed to tear down script component (tear down script)";
        stopped = false;
      }
    }

    bool res = ComponentStorage<ScriptComponent>::removeComponent(component);
    return res && stopped;
  }

  bool LuaScriptSystem::runFunction(ScriptComponent* component, sol::function func)
  {
    if(func.valid()) {
      std::string id = component->getOwner()->getId();
      LOG(INFO) << id << " executing function";
      try {
        auto res = func(component->getOwner());
        return res.valid();
      } catch(sol::error e) {
        LOG(ERROR) << "Failed to run function " << e.what();
        return false;
      }
    }
    return true;
  }

  bool LuaScriptSystem::runScript(ScriptComponent* component, const std::string& script)
  {
    try {
      auto res = mState->script(getScriptData(script).c_str());
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

      // If script returns a function, it means that it wants to have a sandbox

      sol::function callback = r;
      auto callResult = callback(component->getOwner());
      if(!callResult.valid()) {
        sol::error err = callResult;
        LOG(ERROR) << "Failed to execute lua script " << err.what();
        return false;
      }
    } catch(sol::error e) {
      LOG(ERROR) << "Failed to execute lua script " << e.what();
      return false;
    } catch(...) {
      LOG(ERROR) << "Failed to execute lua script: unknown error";
      return false;
    }

    return true;
  }

  std::string LuaScriptSystem::getScriptData(const std::string& data)
  {
    std::vector<std::string> parts = split(data, ':');
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
		    return "";
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
        LOG(ERROR) << "Failed to read script file: " << data;
        stream.close();
		    return "";
      }

      stream.close();
    } else {
      scriptData = data;
    }
    return scriptData;
  }

  bool LuaScriptSystem::runScript(const std::string& script)
  {
    try {
      auto res = mState->script(getScriptData(script).c_str());
      if(!res.valid()) {
        return false;
      }
    } catch(sol::error e) {
      LOG(ERROR) << "Failed to execute lua script " << e.what();
      return false;
    }

    return true;
  }

  bool LuaScriptSystem::addUpdateListener(const sol::object& function, bool global)
  {
    if(std::find_if(mUpdateListeners.begin(), mUpdateListeners.end(), [function] (Listener l) { return l.function == function; } ) != mUpdateListeners.end())
      return true;

    if(function.get_type() == sol::type::function) {
      mUpdateListeners.emplace_back(function.as<sol::function>(), global);
    } else {
      LOG(WARNING) << "Tried to add update listener object of unsupported type";
      return false;
    }

    return true;
  }

  bool LuaScriptSystem::removeUpdateListener(const sol::object& object)
  {
    sol::function function = object.as<sol::function>();

    UpdateListeners::iterator element = std::find_if(mUpdateListeners.begin(), mUpdateListeners.end(), [function] (Listener l) { return l.function == function; } );
    if(element == mUpdateListeners.end())
      return false;

    mUpdateListeners.erase(element);
    return true;
  }

  void LuaScriptSystem::unloadComponents()
  {
    setEnabled(false);
    for (auto i = mUpdateListeners.begin(); i != mUpdateListeners.end();) {
      if(i->global) {
        ++i;
        continue;
      }

      i = mUpdateListeners.erase(i);
    }
    ComponentStorage<ScriptComponent>::unloadComponents();
    setEnabled(true);
  }
}
