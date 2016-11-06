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

#include "lua/LuaEventProxy.h"

#include "ogre/SceneNodeWrapper.h"
#include "ogre/EntityWrapper.h"
#include "ogre/LightWrapper.h"
#include "ogre/ParticleSystemWrapper.h"
#include "ogre/CameraWrapper.h"
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

  template<class From, class To>
  To cast(From f) {
    return static_cast<To>(f);
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
    , mStateView(0)
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
  }

  bool LuaInterface::attachToState(lua_State* L)
  {
    mState = L;
    luaL_openlibs(L);
    if(mStateView)
      delete mStateView;
    mStateView = new sol::state_view(L);
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

    lua.new_usertype<EntityProxy>("EntityProxy",
        sol::constructors<sol::types<const std::string&, Engine*>>(),
        "render", sol::property(&EntityProxy::getComponent<RenderComponent>),
        "movement", sol::property(&EntityProxy::getComponent<MovementComponent>),
        "stats", sol::property(&EntityProxy::getComponent<StatsComponent>),
        "script", sol::property(&EntityProxy::getComponent<ScriptComponent>),
        "id", sol::property(&EntityProxy::getId),
        "valid", sol::property(&EntityProxy::isValid)
    );

    lua.new_usertype<CameraController>("CameraController",
        "setTarget", &CameraController::setTarget
    );

    // --------------------------------------------------------------------------------
    // Ogre objects
    lua.new_usertype<OgreObject>("OgreObject",
        "type", sol::property(&OgreObject::getType)
    );

    lua.new_simple_usertype<SceneNodeWrapper>(
        "OgreSceneNode",
        sol::base_classes, sol::bases<OgreObject>(),
        "getChild", &SceneNodeWrapper::getChild,
        "getSceneNode", &SceneNodeWrapper::getChildOfType<SceneNodeWrapper>,
        "getEntity", &SceneNodeWrapper::getChildOfType<EntityWrapper>,
        "getParticleSystem", &SceneNodeWrapper::getChildOfType<ParticleSystemWrapper>,
        "getCamera", &SceneNodeWrapper::getChildOfType<CameraWrapper>,
        "STATIC", sol::var(SceneNodeWrapper::STATIC),
        "DYNAMIC", sol::var(SceneNodeWrapper::DYNAMIC)
    );

    lua.new_usertype<EntityWrapper>("OgreEntity",
        sol::base_classes, sol::bases<OgreObject>(),
        "attachToBone", &EntityWrapper::attachToBone
    );

    lua.new_usertype<ParticleSystemWrapper>("OgreParticleSystem",
        sol::base_classes, sol::bases<OgreObject>(),
        "createParticle", sol::overload(
          (void(ParticleSystemWrapper::*)(unsigned short, const std::string&)) &ParticleSystemWrapper::createParticle,
          (void(ParticleSystemWrapper::*)(unsigned short, const std::string&, const std::string&)) &ParticleSystemWrapper::createParticle,
          (void(ParticleSystemWrapper::*)(unsigned short, const std::string&, const std::string&, const Ogre::Quaternion&)) &ParticleSystemWrapper::createParticle
        )
    );

    lua.new_usertype<CameraWrapper>("CameraWrapper",
        sol::base_classes, sol::bases<OgreObject>(),
        "attach", &CameraWrapper::attach
    );

    // --------------------------------------------------------------------------------
    // Systems

    lua.new_usertype<EngineSystem>("EngineSystem",
        "enabled", sol::property(&EngineSystem::isEnabled, &EngineSystem::setEnabled)
    );

    lua.new_usertype<OgreRenderSystem>("RenderSystem",
        sol::base_classes, sol::bases<EngineSystem>(),
        "camera", sol::property(&OgreRenderSystem::getCamera),
        "viewport", sol::property(&OgreRenderSystem::getViewport),
        "getObjectsInRadius", &OgreRenderSystem::getObjectsInRadius
    );

    lua.new_usertype<RecastMovementSystem>("MovementSystem",
        sol::base_classes, sol::bases<EngineSystem>(),
        "showNavMesh", &RecastMovementSystem::showNavMesh,
        "setControlledEntity", &RecastMovementSystem::setControlledEntity,
        "resetControlledEntity", &RecastMovementSystem::resetControlledEntity
    );

    lua.new_usertype<LuaScriptSystem>("ScriptSystem",
        sol::base_classes, sol::bases<EngineSystem>(),
        "addUpdateListener", &LuaScriptSystem::addUpdateListener,
        "removeUpdateListener", &LuaScriptSystem::removeUpdateListener
    );

    // --------------------------------------------------------------------------------
    // Components

    lua.new_usertype<RenderComponent>("RenderComponent",
        "position", sol::property(&RenderComponent::getPosition),
        "root", sol::property(&RenderComponent::getRoot),
        "direction", sol::property(&RenderComponent::getDirection),
        "orientation", sol::property(&RenderComponent::getOrientation),
        "facingOrientation", sol::property(&RenderComponent::getFaceOrientation),
        "lookAt", &RenderComponent::lookAt,
        "rotate", &RenderComponent::rotate,
        "playAnimation", &RenderComponent::playAnimation,
        "resetAnimation", &RenderComponent::resetAnimationState,
        "setAnimationState", &RenderComponent::setAnimationState,
        "adjustAnimationSpeed", &RenderComponent::adjustAnimationStateSpeed
    );

    lua.new_usertype<MovementComponent>("MovementComponent",
        "go", sol::overload(
          (void(MovementComponent::*)(const Ogre::Vector3&))&MovementComponent::setTarget,
          (void(MovementComponent::*)(const float&, const float&, const float&))&MovementComponent::setTarget
        ),
        "stop", &MovementComponent::resetTarget,
        "speed", sol::property(&MovementComponent::getSpeed, &MovementComponent::setSpeed),
        "currentTarget", sol::property(&MovementComponent::getCurrentTarget),
        "target", sol::property(&MovementComponent::getFinalTarget),
        "hasTarget", sol::property(&MovementComponent::hasTarget)
    );

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

    lua.new_usertype<Engine>("Engine",
        sol::base_classes, sol::bases<EventDispatcher>(),
        "removeEntity", (bool(Engine::*)(const std::string& id))&Engine::removeEntity,
        "getEntity", &Engine::getEntity,
        "getSystem", (EngineSystem*(Engine::*)(const std::string& name))&Engine::getSystem,
        "render", sol::property(&Engine::getSystem<OgreRenderSystem>),
        "movement", sol::property(&Engine::getSystem<RecastMovementSystem>),
        "script", sol::property(&Engine::getSystem<LuaScriptSystem>)
    );

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
        "intersection", sol::readonly(&SelectEvent::mIntersection),
        "entity", sol::property(&SelectEvent::getEntityId),
        "cast", cast<const Event&, const SelectEvent&>
    );

    lua.new_usertype<StatEvent>("StatEvent",
        sol::base_classes, sol::bases<Event>(),
        "id", sol::property(&StatEvent::getId),
        "cast", cast<const Event&, const StatEvent&>
    );

    // Ogre classes

    lua.new_usertype<Ogre::Vector3>("Vector3",
        sol::constructors<sol::types<const Ogre::Real&, const Ogre::Real&, const Ogre::Real&>>(),
        "x", &Ogre::Vector3::x,
        "y", &Ogre::Vector3::y,
        "z", &Ogre::Vector3::z,
        "squaredDistance", &Ogre::Vector3::squaredDistance,
        "ZERO", sol::var(Ogre::Vector3::ZERO),
        "UNIT_X", sol::var(Ogre::Vector3::UNIT_X),
        "UNIT_Y", sol::var(Ogre::Vector3::UNIT_Y),
        "UNIT_Z", sol::var(Ogre::Vector3::UNIT_Z),
        "NEGATIVE_UNIT_X", sol::var(Ogre::Vector3::NEGATIVE_UNIT_X),
        "NEGATIVE_UNIT_Y", sol::var(Ogre::Vector3::NEGATIVE_UNIT_Y),
        "NEGATIVE_UNIT_Z", sol::var(Ogre::Vector3::NEGATIVE_UNIT_Z),
        "UNIT_SCALE", sol::var(Ogre::Vector3::UNIT_SCALE)
    );

    lua.new_usertype<Ogre::Quaternion>("Quaternion",
        sol::constructors<sol::types<const Ogre::Real&, const Ogre::Real&, const Ogre::Real&, const Ogre::Real&>, sol::types<const Ogre::Radian&, const Ogre::Vector3&>>(),
        "w", &Ogre::Quaternion::w,
        "x", &Ogre::Quaternion::x,
        "y", &Ogre::Quaternion::y,
        "z", &Ogre::Quaternion::z,
        sol::meta_function::multiplication, (Ogre::Quaternion(Ogre::Quaternion::*)(const Ogre::Quaternion&)const)  &Ogre::Quaternion::operator*
    );

    lua.new_usertype<Ogre::Radian>("Radian",
        sol::constructors<sol::types<float>>()
    );

    lua.new_usertype<Dictionary>("Dictionary",
        sol::meta_function::construct, sol::factories(wrapObject)
    );

    mDataManagerProxy = new DataManagerProxy(mInstance->getGameDataManager());
    mEngineProxy = new EngineProxy(mInstance->getEngine());

    lua["resourceDir"] = mResourceDir;
    lua.script("function getResourcePath(path) return resourceDir .. '/' .. path; end");
    lua["game"] = mInstance;
    lua["engine"] = mEngineProxy;
    lua["core"] = mInstance->getEngine();
    lua["data"] = mDataManagerProxy;
    lua["event"] = mEventProxy;

    return true;
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
