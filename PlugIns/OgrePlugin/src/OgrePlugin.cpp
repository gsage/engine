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

#include "OgrePlugin.h"
#include "CameraFactory.h"
#include "GsageFacade.h"
#include "EngineEvent.h"
#include "OgreSelectEvent.h"

#include "lua/LuaInterface.h"

#include "components/MovementComponent.h"
#include "components/RenderComponent.h"

#include "systems/OgreRenderSystem.h"
#include "systems/RecastMovementSystem.h"

#include "ogre/SceneNodeWrapper.h"
#include "ogre/OgreObject.h"
#include "ogre/EntityWrapper.h"
#include "ogre/LightWrapper.h"
#include "ogre/ParticleSystemWrapper.h"
#include "ogre/CameraWrapper.h"

#include "sol.hpp"

namespace Gsage {

  const std::string PLUGIN_NAME = "OgreBundle";

  OgrePlugin::OgrePlugin()
  {
  }

  OgrePlugin::~OgrePlugin()
  {
  }

  const std::string& OgrePlugin::getName() const
  {
    return PLUGIN_NAME;
  }

  void OgrePlugin::setupLuaBindings() {
    if (mLuaInterface && mLuaInterface->getState())
    {
      sol::state_view lua = *mLuaInterface->getSolState();

      // Ogre Wrappers

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

      // Systems

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

      // Ogre Types

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

      lua.new_usertype<OgreSelectEvent>("OgreSelectEvent",
          sol::base_classes, sol::bases<Event, SelectEvent>(),
          "intersection", sol::property(&OgreSelectEvent::getIntersection),
          "cast", cast<const Event&, const OgreSelectEvent&>
      );

      lua["Engine"]["render"] = &Engine::getSystem<OgreRenderSystem>;
      lua["Engine"]["movement"] = &Engine::getSystem<RecastMovementSystem>;

      lua["EntityProxy"]["render"] = &EntityProxy::getComponent<RenderComponent>;
      lua["EntityProxy"]["movement"] = &EntityProxy::getComponent<MovementComponent>;
      LOG(INFO) << "Registered lua bindings for " << PLUGIN_NAME;
    }
    else
    {
      LOG(WARNING) << "Lua bindings for ogre plugin were not registered: lua state is nil";
    }
  }

  bool OgrePlugin::installImpl()
  {
    mFacade->registerSystemFactory<OgreRenderSystem>("ogre");
    mFacade->registerSystemFactory<RecastMovementSystem>("recast");
    return true;
  }

  void OgrePlugin::uninstallImpl()
  {
    if (mLuaInterface && mLuaInterface->getState())
    {
      sol::state_view lua = *mLuaInterface->getSolState();

      lua["Engine"]["render"] = sol::nil;
      lua["Engine"]["movement"] = sol::nil;

      lua["EntityProxy"]["render"] = sol::nil;
      lua["EntityProxy"]["movement"] = sol::nil;
    }

    mFacade->getEngine()->removeSystem("render");
    mFacade->getEngine()->removeSystem("movement");

  }

  OgrePlugin* ogrePlugin = NULL;

  extern "C" bool PluginExport dllStartPlugin(GsageFacade* facade)
  {
    if(ogrePlugin != NULL)
    {
      return false;
    }
    ogrePlugin = new OgrePlugin();
    return facade->installPlugin(ogrePlugin);
  }

  extern "C" bool PluginExport dllStopPlugin(GsageFacade* facade)
  {
    if(ogrePlugin == NULL)
      return true;

    bool res = facade->uninstallPlugin(ogrePlugin);
    if(!res)
      return false;
    delete ogrePlugin;
    ogrePlugin = NULL;
    return true;
  }
}
