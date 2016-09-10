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

#include <luabind/adopt_policy.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/operator.hpp>

#include "GsageFacade.h"

#include "GameDataManager.h"
#include "EngineEvent.h"
#include "CameraFactory.h"

#include "components/MovementComponent.h"
#include "components/RenderComponent.h"
#include "components/StatsComponent.h"
#include "components/ScriptComponent.h"

#include "systems/OgreRenderSystem.h"
#include "systems/RecastMovementSystem.h"
#include "systems/CombatSystem.h"
#include "systems/LuaScriptSystem.h"

#include "input/OisInputListener.h"

#include "lua/LuaConverters.h"
#include "lua/LuaEventProxy.h"

#include "ogre/SceneNodeWrapper.h"
#include "ogre/EntityWrapper.h"
#include "ogre/LightWrapper.h"
#include "ogre/ParticleSystemWrapper.h"
#include "ogre/CameraWrapper.h"

using namespace luabind;

namespace luabind {
  template<>
  struct default_converter<DataNode> : native_converter_base<DataNode> {
    static int compute_score(lua_State* L, int index) {
      return lua_type(L, index) == LUA_TTABLE ? 0 : -1;
    }

    static void iterate(object lua_object, DataNode& node) {
      for(iterator i(lua_object), end; i != end; i++)
      {
        object val = *i;
        if (type(val) == LUA_TTABLE) 
        {
          DataNode child;
          iterate(val, child);
          if(type(i.key()) == LUA_TSTRING)
            node.put_child(object_cast<std::string>(i.key()), child);
          else
            node.push_back(std::make_pair("", child));
        }
        else
        {
          std::string value;
          if(type(val) == LUA_TNUMBER)
            value = std::to_string(object_cast<float>(val));
          else
            value = object_cast<std::string>(val);

          if(type(i.key()) == LUA_TSTRING)
            node.put(object_cast<std::string>(i.key()), value);
          else
            node.push_back(std::make_pair("", value));
        }
      }
    }

    DataNode from(lua_State* L, int index) {
      DataNode node;
      iterate(object(from_stack(L, index)), node);
      return node;
    }

    void to(lua_State* L, const DataNode& l) {
      luabind::object table = luabind::newtable(L);

      // TODO: this is not that simple, implement it later
      /*for (auto pair& : node) {
        table[pair.first] = pair.second.get();
        }*/

      table.push(L);
    }
  };
}

namespace Gsage {

  DataManagerProxy::DataManagerProxy(GameDataManager* instance)
    : mInstance(instance)
  {
  }

  DataManagerProxy::~DataManagerProxy()
  {
  }

  Entity* DataManagerProxy::createEntity(const std::string& templateFile, const luabind::object& params)
  {
    GameDataManager::TemplateParameters internalParams;
    for(iterator i(params), end; i != end; i++)
    {
      std::string key = object_cast<std::string>(i.key());
      luabind::object val = *i;
      if (type(val) == LUA_TSTRING)
      {
        internalParams[key] = object_cast<std::string>(val);
      }
    }
    return mInstance->createEntity(templateFile, internalParams);
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

  template<class C>
  C* EntityProxy::getComponent()
  {
    return mEngine->getComponent<C>(mEntityId);
  }

  const std::string& EntityProxy::getId()
  {
    return mEntityId;
  }

  bool EntityProxy::isValid()
  {
    return mEngine->getEntity(mEntityId) != 0;
  }

  LuaInterface::LuaInterface(GsageFacade* instance, const std::string& resourceDir)
    : mInstance(instance)
    , mDataManagerProxy(0)
    , mEngineProxy(0)
    , mEventProxy(new LuaEventProxy())
    , mResourceDir(resourceDir)
    , mState(0)
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
  }

  bool LuaInterface::attachToState(lua_State* L)
  {
    mState = L;

    #define LUA_CONST_START( class ) { object g = globals(L); object table = g[#class]; 
    #define LUA_CONST( class, name ) table[#name] = class::name 
    #define LUA_CONST_END } 

    luaL_openlibs(L);
    open(L);
    module(L)
    [
      class_<EventDispatcher>("EventDispatcher"),

      class_<EngineProxy>("EngineProxy")
        .def("get", &EngineProxy::getEntity, adopt(result)),

      class_<EntityProxy>("EntityProxy")
        .def(constructor<const std::string&, Engine*>())
        .property("render", &EntityProxy::getComponent<RenderComponent>)
        .property("movement", &EntityProxy::getComponent<MovementComponent>)
        .property("stats", &EntityProxy::getComponent<StatsComponent>)
        .property("script", &EntityProxy::getComponent<ScriptComponent>)
        .property("id", &EntityProxy::getId)
        .property("valid", &EntityProxy::isValid),

      class_<CameraController>("CameraController")
        .def("setTarget", &CameraController::setTarget),

      class_<GsageFacade>("Facade")
        .def("reset", &GsageFacade::reset)
        .def("halt", &GsageFacade::halt)
        .def("loadSave", &GsageFacade::loadSave)
        .def("dumpSave", &GsageFacade::dumpSave)
        .def("loadPlugin", &GsageFacade::loadPlugin)
        .def("unloadPlugin", &GsageFacade::unloadPlugin),

      // --------------------------------------------------------------------------------
      // Ogre objects

      class_<OgreObject>("Object")
        .property("type", &OgreObject::getType),

      class_<SceneNodeWrapper, OgreObject>("SceneNode")
        .def("getChild", &SceneNodeWrapper::getChild)
        .enum_("flags")
        [
          value("STATIC", SceneNodeWrapper::STATIC),
          value("DYNAMIC", SceneNodeWrapper::DYNAMIC)
        ],

      class_<EntityWrapper, OgreObject>("Entity")
        .def("attachToBone", &EntityWrapper::attachToBone),
      class_<LightWrapper, OgreObject>("Light"),
      class_<ParticleSystemWrapper, OgreObject>("OgreParticleSystem")
        .def("createParticle", (void(ParticleSystemWrapper::*)(unsigned short, const std::string&)) &ParticleSystemWrapper::createParticle)
        .def("createParticle", (void(ParticleSystemWrapper::*)(unsigned short, const std::string&, const std::string&)) &ParticleSystemWrapper::createParticle)
        .def("createParticle", (void(ParticleSystemWrapper::*)(unsigned short, const std::string&, const std::string&, const Ogre::Quaternion&)) &ParticleSystemWrapper::createParticle),

      class_<CameraWrapper, OgreObject>("CameraWrapper")
        .def("attach", &CameraWrapper::attach),

      // --------------------------------------------------------------------------------
      // Systems

      class_<EngineSystem>("EngineSystem")
        .property("enabled", &EngineSystem::isEnabled, &EngineSystem::setEnabled),

      class_<OgreRenderSystem, EngineSystem>("RenderSystem")
        .property("camera", &OgreRenderSystem::getCamera)
        .property("viewport", &OgreRenderSystem::getViewport)
        .def("getObjectsInRadius", &OgreRenderSystem::getObjectsInRadius),

      class_<RecastMovementSystem, EngineSystem>("MovementSystem")
        .def("showNavMesh", &RecastMovementSystem::showNavMesh)
        .def("setControlledEntity", &RecastMovementSystem::setControlledEntity)
        .def("resetControlledEntity", &RecastMovementSystem::resetControlledEntity),

      class_<LuaScriptSystem, EngineSystem>("ScriptSystem")
        .def("addUpdateListener", &LuaScriptSystem::addUpdateListener)
        .def("removeUpdateListener", &LuaScriptSystem::removeUpdateListener),

      // --------------------------------------------------------------------------------
      // Components

      class_<RenderComponent>("RenderComponent")
        .property("position", &RenderComponent::getPosition)
        .property("root", &RenderComponent::getRoot)
        .property("direction", &RenderComponent::getDirection)
        .property("orientation", &RenderComponent::getOrientation)
        .property("facingOrientation", &RenderComponent::getFaceOrientation)
        .def("lookAt", &RenderComponent::lookAt)
        .def("rotate", &RenderComponent::rotate)
        .def("playAnimation", &RenderComponent::playAnimation)
        .def("resetAnimation", &RenderComponent::resetAnimationState)
        .def("setAnimationState", &RenderComponent::setAnimationState)
        .def("adjustAnimationSpeed", &RenderComponent::adjustAnimationStateSpeed),

      class_<MovementComponent>("MovementComponent")
        .def("go", (void(MovementComponent::*)(const Ogre::Vector3&))&MovementComponent::setTarget)
        .def("go", (void(MovementComponent::*)(const float&, const float&, const float&))&MovementComponent::setTarget)
        .def("stop", &MovementComponent::resetTarget)
        .property("speed", &MovementComponent::getSpeed, &MovementComponent::setSpeed)
        .property("currentTarget", &MovementComponent::getCurrentTarget)
        .property("target", &MovementComponent::getFinalTarget)
        .property("hasTarget", &MovementComponent::hasTarget),

      class_<StatsComponent, EventDispatcher>("StatsComponent")
        .def("hasStat", &StatsComponent::hasStat)
        .def("getBool", (const bool(StatsComponent::*)(const std::string&))&StatsComponent::getStat<bool>)
        .def("getBool", (const bool(StatsComponent::*)(const std::string&, const bool&))&StatsComponent::getStat<bool>)
        .def("getString", (const std::string(StatsComponent::*)(const std::string&))&StatsComponent::getStat<std::string>)
        .def("getString", (const std::string(StatsComponent::*)(const std::string&, const std::string&))&StatsComponent::getStat<std::string>)
        .def("getNumber", (const float(StatsComponent::*)(const std::string&)) &StatsComponent::getStat<float>)
        .def("getNumber", (const float(StatsComponent::*)(const std::string&, const float&)) &StatsComponent::getStat<float>)
        .def("set", &StatsComponent::setStat<bool>)
        .def("set", &StatsComponent::setStat<std::string>)
        .def("set", &StatsComponent::setStat<float>)
        .def("increase", &StatsComponent::increase),

      class_<ScriptComponent>("ScriptComponent")
        .property("state", &ScriptComponent::getData),

      class_<Entity>("Entity")
        .property("id", &Entity::getId),

      class_<Engine, EventDispatcher>("Engine")
        .def("removeEntity", (bool(Engine::*)(const std::string& id))&Engine::removeEntity)
        .def("getEntity", &Engine::getEntity)
        .def("getSystem", (EngineSystem*(Engine::*)(const std::string& name))&Engine::getSystem)
        .property("render", &Engine::getSystem<OgreRenderSystem>)
        .property("movement", &Engine::getSystem<RecastMovementSystem>)
        .property("script", &Engine::getSystem<LuaScriptSystem>),

      class_<DataManagerProxy>("DataManager")
        .def("createEntity", (Entity*(DataManagerProxy::*)(const std::string&))&DataManagerProxy::createEntity)
        .def("createEntity", (Entity*(DataManagerProxy::*)(const std::string&, const object&))&DataManagerProxy::createEntity),

      class_<LuaEventProxy>("LuaEventProxy")
        .def("bind", &LuaEventProxy::addEventListener)
        .def("unbind", &LuaEventProxy::removeEventListener),

      class_<Event>("BaseEvent")
        .property("type", &Event::getType),

      class_<SelectEvent, Event>("SelectEvent")
        .def("hasFlags", &SelectEvent::hasFlags)
        .def_readonly("intersection", &SelectEvent::mIntersection)
        .property("entity", &SelectEvent::getEntityId),

      class_<StatEvent, Event>("StatEvent")
        .property("id", &StatEvent::getId),

      class_<Ogre::Vector3>("Vector3")
        .def(constructor<const Ogre::Real&, const Ogre::Real&, const Ogre::Real&>())
        .def_readwrite("x", &Ogre::Vector3::x)
        .def_readwrite("y", &Ogre::Vector3::y)
        .def_readwrite("z", &Ogre::Vector3::z)
        .def("squaredDistance", &Ogre::Vector3::squaredDistance),

      class_<Ogre::Quaternion>("Quaternion")
        .def(constructor<const Ogre::Real&, const Ogre::Real&, const Ogre::Real&, const Ogre::Real&>())
        .def(constructor<const Ogre::Radian&, const Ogre::Vector3&>())
        .def_readwrite("w", &Ogre::Quaternion::w)
        .def_readwrite("x", &Ogre::Quaternion::x)
        .def_readwrite("y", &Ogre::Quaternion::y)
        .def_readwrite("z", &Ogre::Quaternion::z)
        .def(self * other<Ogre::Quaternion>()),

      class_<Ogre::Radian>("Radian")
        .def(constructor<float>()),

      class_<Ogre::Viewport>("Viewport")
    ];

    mDataManagerProxy = new DataManagerProxy(mInstance->getGameDataManager());
    mEngineProxy = new EngineProxy(mInstance->getEngine());

    globals(L)["resourceDir"] = mResourceDir;
    luaL_dostring(L, "function getResourcePath(path) return resourceDir .. '/' .. path; end");

    globals(L)["game"] = mInstance;
    globals(L)["engine"] = mEngineProxy;
    globals(L)["core"] = mInstance->getEngine();
    globals(L)["data"] = mDataManagerProxy;
    globals(L)["event"] = mEventProxy;
    LUA_CONST_START( Vector3 )
      LUA_CONST( Ogre::Vector3, ZERO);
      LUA_CONST( Ogre::Vector3, UNIT_X);
      LUA_CONST( Ogre::Vector3, UNIT_Y);
      LUA_CONST( Ogre::Vector3, UNIT_Z);
      LUA_CONST( Ogre::Vector3, NEGATIVE_UNIT_X);
      LUA_CONST( Ogre::Vector3, NEGATIVE_UNIT_Y);
      LUA_CONST( Ogre::Vector3, NEGATIVE_UNIT_Z);
      LUA_CONST( Ogre::Vector3, UNIT_SCALE);
    LUA_CONST_END;

    return true;
  }

  lua_State* LuaInterface::getState()
  {
    return mState;
  }

  bool LuaInterface::runScript(const std::string& script)
  {
    if(!mState)
      return false;

    int res = luaL_dofile(mState, script.c_str());
    if(res != 0)
    {
      LOG(ERROR) << "Failed to execute lua script:\n" << lua_tostring(mState, -1);
      return false;
    }

    return true;
  }
}
