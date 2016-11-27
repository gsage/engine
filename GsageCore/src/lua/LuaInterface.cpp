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

#include "lua/LuaInterface.h"

#include "GsageFacade.h"

#include "GameDataManager.h"
#include "EngineEvent.h"

#include "components/StatsComponent.h"
#include "components/ScriptComponent.h"

#include "systems/CombatSystem.h"
#include "systems/LuaScriptSystem.h"

#include "input/OisInputListener.h"

#include "lua/LuaEventProxy.h"

#include "Dictionary.h"

#include "sol.hpp"


namespace Gsage {

  void fillRecoursively(const sol::object& t, Dictionary& dest) {
    if(t.get_type() == sol::type::table) {
      for(auto pair : t.as<sol::table>()) {
        Dictionary d;
        fillRecoursively(pair.second, d);

        if(pair.first.get_type() == sol::type::number) {
          dest.push(d);
        } else {
          dest.put(pair.first.as<std::string>(), d);
        }
      }
    } else {
      dest.set(t.as<std::string>());
    }
  }

  std::shared_ptr<Dictionary> wrapObject(const sol::object& t)
  {
    Dictionary* d = new Dictionary();
    fillRecoursively(t, *d);
    return std::shared_ptr<Dictionary>(d);
  }

  DataManagerProxy::DataManagerProxy(GameDataManager* instance)
    : mInstance(instance)
  {
  }

  DataManagerProxy::~DataManagerProxy()
  {
  }

  Entity* DataManagerProxy::createEntity(const std::string& templateFile, Dictionary params)
  {
    return mInstance->createEntity(templateFile, params);
  }

  Entity* DataManagerProxy::createEntity(const std::string& json)
  {
    return mInstance->createEntity(json);
  }

  EntityProxy* EngineProxy::getEntity(const std::string& id)
  {
    return new EntityProxy(id, mEngine);
  }

  EntityProxy::EntityProxy(const std::string& entityId, Engine* engine)
    : mEngine(engine)
    , mEntityId(entityId)
  {
    // TODO: create entity remove event and subscribe to it
  }

  EntityProxy::~EntityProxy()
  {
  }

  const std::string& EntityProxy::getId()
  {
    return mEntityId;
  }

  bool EntityProxy::isValid()
  {
    return mEngine->getEntity(mEntityId) != 0;
  }

  LuaInterface::LuaInterface(GsageFacade* instance, const std::string& resourcePath)
    : mInstance(instance)
    , mDataManagerProxy(0)
    , mEngineProxy(0)
    , mEventProxy(new LuaEventProxy())
    , mState(0)
    , mResourcePath(resourcePath)
    , mStateView(0)
    , mStateCreated(false)
  {
  }

  LuaInterface::~LuaInterface()
  {
    if(mDataManagerProxy)
      delete mDataManagerProxy;

    if(mEventProxy)
      delete mEventProxy;

    if(mEngineProxy)
      delete mEngineProxy;

    if(mStateView)
      delete mStateView;

    closeLuaState();
  }

  void LuaInterface::closeLuaState()
  {
    if(mStateCreated) {
      lua_close(mState);
    }
  }

  bool LuaInterface::initialize(lua_State* L)
  {
    if(L != 0 && L == mState) {
      return true;
    }

    if(mStateView)
      delete mStateView;

    if(mState) {
      closeLuaState();
    }

    if(L == 0) {
      mState = lua_open();
      mStateCreated = true;
    } else {
      mState = L;
      mStateCreated = false;
    }

    luaL_openlibs(mState);
    mStateView = new sol::state_view(mState);
    sol::state_view lua = *mStateView;

    lua.new_usertype<GsageFacade>("Facade",
        "new", sol::no_constructor,
        "halt", &GsageFacade::halt,
        "reset", &GsageFacade::reset,
        "loadSave", &GsageFacade::loadSave,
        "dumpSave", &GsageFacade::dumpSave,
        "loadPlugin", &GsageFacade::loadPlugin,
        "unloadPlugin", &GsageFacade::unloadPlugin
    );

    lua.new_usertype<EngineProxy>("EngineProxy",
        "get", &EngineProxy::getEntity
    );

    lua.new_simple_usertype<EntityProxy>("EntityProxy",
        sol::constructors<sol::types<const std::string&, Engine*>>(),
        "id", sol::property(&EntityProxy::getId),
        "valid", sol::property(&EntityProxy::isValid)
    );

    lua["EntityProxy"]["stats"] = &EntityProxy::getComponent<StatsComponent>;
    lua["EntityProxy"]["script"] = &EntityProxy::getComponent<ScriptComponent>;

    // --------------------------------------------------------------------------------
    // Systems

    lua.new_usertype<EngineSystem>("EngineSystem",
        "enabled", sol::property(&EngineSystem::isEnabled, &EngineSystem::setEnabled)
    );

    lua.new_usertype<LuaScriptSystem>("ScriptSystem",
        sol::base_classes, sol::bases<EngineSystem>(),
        "addUpdateListener", &LuaScriptSystem::addUpdateListener,
        "removeUpdateListener", &LuaScriptSystem::removeUpdateListener
    );

    // --------------------------------------------------------------------------------
    // Components

    lua.new_usertype<StatsComponent>("StatsComponent",
        sol::base_classes, sol::bases<EventDispatcher>(),
        "hasStat", &StatsComponent::hasStat,
        "getBool", sol::overload(
          (const bool(StatsComponent::*)(const std::string&))&StatsComponent::getStat<bool>,
          (const bool(StatsComponent::*)(const std::string&, const bool&))&StatsComponent::getStat<bool>
        ),
        "getString", sol::overload(
          (const std::string(StatsComponent::*)(const std::string&))&StatsComponent::getStat<std::string>,
          (const std::string(StatsComponent::*)(const std::string&, const std::string&))&StatsComponent::getStat<std::string>
        ),
        "getNumber", sol::overload(
          (const float(StatsComponent::*)(const std::string&)) &StatsComponent::getStat<float>,
          (const float(StatsComponent::*)(const std::string&, const float&)) &StatsComponent::getStat<float>
        ),
        "set", sol::overload(
          &StatsComponent::setStat<bool>,
          &StatsComponent::setStat<std::string>,
          &StatsComponent::setStat<float>
        ),
        "increase", &StatsComponent::increase
    );

    lua.new_usertype<ScriptComponent>("ScriptComponent",
        "state", sol::property(&ScriptComponent::getData)
    );

    lua.new_usertype<Entity>("Entity",
        "id", sol::property(&Entity::getId)
    );

    lua.new_simple_usertype<Engine>("Engine",
        sol::base_classes, sol::bases<EventDispatcher>()
    );

    lua["Engine"]["removeEntity"] = (bool(Engine::*)(const std::string& id))&Engine::removeEntity;
    lua["Engine"]["getEntity"] = &Engine::getEntity;
    lua["Engine"]["getSystem"] = (EngineSystem*(Engine::*)(const std::string& name))&Engine::getSystem;
    lua["Engine"]["script"] = &Engine::getSystem<LuaScriptSystem>;

    lua.new_usertype<DataManagerProxy>("DataManager",
        "createEntity", sol::overload(
          (Entity*(DataManagerProxy::*)(const std::string&))&DataManagerProxy::createEntity,
          (Entity*(DataManagerProxy::*)(const std::string&, Dictionary))&DataManagerProxy::createEntity
        )
    );

    lua.new_usertype<LuaEventProxy>("LuaEventProxy",
        "bind", &LuaEventProxy::addEventListener,
        "unbind", &LuaEventProxy::removeEventListener
    );

    // events

    lua.new_usertype<Event>("BaseEvent",
        "type", sol::property(&Event::getType)
    );

    lua.new_usertype<SelectEvent>("SelectEvent",
        sol::base_classes, sol::bases<Event>(),
        "hasFlags", &SelectEvent::hasFlags,
        "entity", sol::property(&SelectEvent::getEntityId),
        "cast", cast<const Event&, const SelectEvent&>
    );

    lua.new_usertype<StatEvent>("StatEvent",
        sol::base_classes, sol::bases<Event>(),
        "id", sol::property(&StatEvent::getId),
        "cast", cast<const Event&, const StatEvent&>
    );

    lua.new_usertype<Dictionary>("Dictionary",
        sol::meta_function::construct, sol::factories(wrapObject)
    );

    mDataManagerProxy = new DataManagerProxy(mInstance->getGameDataManager());
    mEngineProxy = new EngineProxy(mInstance->getEngine());

    mInstance->getEngine()->fireEvent(EngineEvent(EngineEvent::LUA_STATE_CHANGE));

    lua["resourcePath"] = mResourcePath;
    lua.script("function getResourcePath(path) return resourcePath .. '/' .. path; end");
    lua["game"] = mInstance;
    lua["engine"] = mEngineProxy;
    lua["core"] = mInstance->getEngine();
    lua["data"] = mDataManagerProxy;
    lua["event"] = mEventProxy;

    return true;
  }

  void LuaInterface::setResourcePath(const std::string& path)
  {
    mResourcePath = path;
    if(mStateView) {
      (*mStateView)["resourcePath"] = mResourcePath;
      mStateView->script("function getResourcePath(path) return resourcePath .. '/' .. path; end");
    }
  }

  sol::state_view* LuaInterface::getSolState() {
    return mStateView;
  }

  lua_State* LuaInterface::getState()
  {
    return mState;
  }

  bool LuaInterface::runScript(const std::string& script)
  {
    if(!mStateView)
      return false;

    try {
      LOG(INFO) << "Executing script " << script;
      auto res = mStateView->script_file(script);
      if(!res.valid()) {
        sol::error err = res;
        throw err;
      }
    } catch(sol::error& err) {
      LOG(ERROR) << "Failed to execute lua script: " << err.what();
      return false;
    } catch(...) {
      LOG(ERROR) << "Unknown script execution error";
      return false;
    }

    return true;
  }
}
