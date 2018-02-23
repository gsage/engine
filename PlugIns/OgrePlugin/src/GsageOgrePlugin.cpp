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

#include "GsageOgrePlugin.h"
#include "GsageFacade.h"
#include "EngineEvent.h"
#include "OgreSelectEvent.h"
#include "RenderTarget.h"
#include "RenderTargetTypes.h"

#include "components/OgreRenderComponent.h"

#include "systems/OgreRenderSystem.h"

#include "ogre/SceneNodeWrapper.h"
#include "ogre/OgreObject.h"
#include "ogre/EntityWrapper.h"
#include "ogre/LightWrapper.h"
#include "ogre/ParticleSystemWrapper.h"
#include "ogre/CameraWrapper.h"

namespace Gsage {

  const std::string PLUGIN_NAME = "OgreBundle";

  GsageOgrePlugin::GsageOgrePlugin()
  {
  }

  GsageOgrePlugin::~GsageOgrePlugin()
  {
  }

  const std::string& GsageOgrePlugin::getName() const
  {
    return PLUGIN_NAME;
  }

  void GsageOgrePlugin::setupLuaBindings() {
    if (mLuaInterface && mLuaInterface->getState())
    {
      // Override select event
      mLuaInterface->registerEvent<OgreSelectEvent>("OgreSelectEvent",
        "onOgreSelect",
        sol::base_classes, sol::bases<Event, SelectEvent>(),
        "intersection", sol::property(&OgreSelectEvent::getIntersection)
      );

      sol::state_view lua(mLuaInterface->getState());

      // Ogre Wrappers

      lua.new_usertype<OgreObject>("OgreObject",
          sol::base_classes, sol::bases<Reflection>(),
          "type", sol::property(&OgreObject::getType),
          "name", sol::property(&OgreObject::getObjectId)
      );

      lua.new_usertype<SceneNodeWrapper>(
          "OgreSceneNode",
          sol::base_classes, sol::bases<OgreObject>(),
          "orientation", sol::property(&SceneNodeWrapper::setOrientation, &SceneNodeWrapper::getOrientation),
          "scale", sol::property(&SceneNodeWrapper::setScale, &SceneNodeWrapper::getScale),
          "position", sol::property(&SceneNodeWrapper::setPosition, &SceneNodeWrapper::getPosition),
          "getChild", &SceneNodeWrapper::getChild,
          "getSceneNode", &SceneNodeWrapper::getChildOfType<SceneNodeWrapper>,
          "getEntity", &SceneNodeWrapper::getChildOfType<EntityWrapper>,
          "getParticleSystem", &SceneNodeWrapper::getChildOfType<ParticleSystemWrapper>,
          "getCamera", &SceneNodeWrapper::getChildOfType<CameraWrapper>,
          "rotate", &SceneNodeWrapper::rotate,
          "pitch", &SceneNodeWrapper::pitch,
          "yaw", &SceneNodeWrapper::yaw,
          "roll", &SceneNodeWrapper::roll,
          "translate", &SceneNodeWrapper::translate,
          "lookAt", &SceneNodeWrapper::lookAt,
          "children", sol::property(&SceneNodeWrapper::writeChildren)
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
          "attach", sol::overload(
            (void(CameraWrapper::*)(Ogre::Viewport* viewport))&CameraWrapper::attach,
            (void(CameraWrapper::*)(RenderTargetPtr renderTarget))&CameraWrapper::attach
          ),
          "isActive", &CameraWrapper::isActive,
          "getProjectionMatrix", &CameraWrapper::getProjectionMatrix,
          "getViewMatrix", &CameraWrapper::getViewMatrix,
          "getCamera", (Ogre::Camera*(CameraWrapper::*)())&CameraWrapper::getCamera
      );

      lua.new_usertype<Ogre::Camera>("Camera");

      lua.new_usertype<RenderTarget>("RenderTarget",
          "setCamera", &RenderTarget::setCamera,
          "getCamera", &RenderTarget::getCamera,
          "name", sol::property(&RenderTarget::getName),

          "Rtt", sol::var(RenderTargetType::Rtt),
          "Window", sol::var(RenderTargetType::Window)
      );

      lua.new_usertype<WindowRenderTarget>("WindowRenderTarget",
          sol::base_classes, sol::bases<RenderTarget>()
      );

      lua.new_usertype<RttRenderTarget>("RttRenderTarget",
          sol::base_classes, sol::bases<RenderTarget>()
      );

      // Systems

      lua.new_usertype<OgreRenderSystem>("OgreRenderSystem",
          sol::base_classes, sol::bases<EngineSystem, RenderSystem>(),
          "configure", &OgreRenderSystem::configure,
          "getObjectsInRadius", &OgreRenderSystem::getObjectsInRadius,
          "createRenderTarget", &OgreRenderSystem::createRenderTarget,
          "renderCameraToTarget", sol::overload(
            (void(OgreRenderSystem::*)(const std::string&, const std::string&)) &OgreRenderSystem::renderCameraToTarget,
            (void(OgreRenderSystem::*)(Ogre::Camera*, const std::string&)) &OgreRenderSystem::renderCameraToTarget
          ),
          "mainRenderTarget", sol::property(&OgreRenderSystem::getMainRenderTarget),
          "getRenderTarget", &OgreRenderSystem::getRenderTarget,
          "getGeometry", sol::overload(
            (GeomPtr(OgreRenderSystem::*)(const BoundingBox&, int)) &OgreRenderSystem::getGeometry,
            (GeomPtr(OgreRenderSystem::*)(std::vector<std::string>)) &OgreRenderSystem::getGeometry
          )
      );

      lua["ogre"] = lua.create_table_with(
          "PF_R8G8B8A8", Ogre::PF_R8G8B8A8,
          "RENDER_QUEUE_BACKGROUND", Ogre::RENDER_QUEUE_BACKGROUND,
          "RENDER_QUEUE_SKIES_EARLY", Ogre::RENDER_QUEUE_SKIES_EARLY,
          "RENDER_QUEUE_MAIN", Ogre::RENDER_QUEUE_MAIN,
          "RENDER_QUEUE_OVERLAY", Ogre::RENDER_QUEUE_OVERLAY,
          "OT_POINT_LIST", Ogre::RenderOperation::OT_POINT_LIST,
          "OT_LINE_LIST", Ogre::RenderOperation::OT_LINE_LIST,
          "OT_LINE_STRIP", Ogre::RenderOperation::OT_LINE_STRIP,
          "OT_TRIANGLE_LIST", Ogre::RenderOperation::OT_TRIANGLE_LIST,
          "OT_TRIANGLE_STRIP", Ogre::RenderOperation::OT_TRIANGLE_STRIP,
          "OT_TRIANGLE_FAN", Ogre::RenderOperation::OT_TRIANGLE_FAN
      );

      // Components

      lua.new_usertype<OgreRenderComponent>("OgreRenderComponent",
          sol::base_classes, sol::bases<EventDispatcher, Reflection>(),
          "props", sol::property(&OgreRenderComponent::getProps, &OgreRenderComponent::setProps),
          "position", sol::property((void(OgreRenderComponent::*)(const Ogre::Vector3&))&OgreRenderComponent::setPosition, &OgreRenderComponent::getOgrePosition),
          "root", sol::property(&OgreRenderComponent::getRoot),
          "direction", sol::property(&OgreRenderComponent::getOgreDirection),
          "orientation", sol::property(
            (void(OgreRenderComponent::*)(const Ogre::Quaternion&))&OgreRenderComponent::setOrientation,
            &OgreRenderComponent::getOgreOrientation
          ),
          "facingOrientation", sol::property(&OgreRenderComponent::getOgreFaceOrientation),
          "lookAt", sol::overload(
            (void(OgreRenderComponent::*)(const Ogre::Vector3&, const Geometry::RotationAxis, Geometry::TransformSpace))&OgreRenderComponent::lookAt,
            (void(OgreRenderComponent::*)(const Ogre::Vector3&))&OgreRenderComponent::lookAt
          ),
          "rotate", sol::overload(
            (void(OgreRenderComponent::*)(const Ogre::Quaternion&))&OgreRenderComponent::rotate,
            (void(OgreRenderComponent::*)(const Gsage::Quaternion&))&OgreRenderComponent::rotate
          ),
          "playAnimation", &OgreRenderComponent::playAnimation,
          "resetAnimation", &OgreRenderComponent::resetAnimationState,
          "setAnimationState", &OgreRenderComponent::setAnimationState,
          "adjustAnimationSpeed", &OgreRenderComponent::adjustAnimationStateSpeed,

          "POSITION_CHANGE", sol::var(OgreRenderComponent::POSITION_CHANGE)
      );

      // Ogre Types
      auto parseVector = [] (const std::string& value) -> std::tuple<Ogre::Vector3, bool> {
        Ogre::Vector3 vector;
        bool succeed = typename TranslatorBetween<std::string, Ogre::Vector3>::type().to(value, vector);
        return std::make_tuple(vector, succeed);
      };

      lua.new_usertype<Ogre::Vector3>("Vector3",
          sol::constructors<sol::types<const Ogre::Real&, const Ogre::Real&, const Ogre::Real&>>(),
          "parse", sol::factories(parseVector),
          "x", &Ogre::Vector3::x,
          "y", &Ogre::Vector3::y,
          "z", &Ogre::Vector3::z,
          "squaredDistance", &Ogre::Vector3::squaredDistance,
          "crossProduct", &Ogre::Vector3::crossProduct,
          "ZERO", sol::property([] () -> Ogre::Vector3 { return Ogre::Vector3::ZERO; }),
          "UNIT_X", sol::property([] () -> Ogre::Vector3 { return Ogre::Vector3::UNIT_X; }),
          "UNIT_Y", sol::property([] () -> Ogre::Vector3 { return Ogre::Vector3::UNIT_Y; }),
          "UNIT_Z", sol::property([] () -> Ogre::Vector3 { return Ogre::Vector3::UNIT_Z; }),
          "NEGATIVE_UNIT_X", sol::property([] () -> Ogre::Vector3 { return Ogre::Vector3::NEGATIVE_UNIT_X; }),
          "NEGATIVE_UNIT_Y", sol::property([] () -> Ogre::Vector3 { return Ogre::Vector3::NEGATIVE_UNIT_Y; }),
          "NEGATIVE_UNIT_Z", sol::property([] () -> Ogre::Vector3 { return Ogre::Vector3::NEGATIVE_UNIT_Z; }),
          "UNIT_SCALE", sol::property([] () -> Ogre::Vector3 { return Ogre::Vector3::UNIT_SCALE; }),
          sol::meta_function::multiplication, sol::overload(
            (Ogre::Vector3(Ogre::Vector3::*)(const Ogre::Vector3&)const)&Ogre::Vector3::operator*,
            (Ogre::Vector3(Ogre::Vector3::*)(const Ogre::Real)const)&Ogre::Vector3::operator*
          ),
          sol::meta_function::addition, (Ogre::Vector3(Ogre::Vector3::*)(const Ogre::Vector3&)const)&Ogre::Vector3::operator+,
          sol::meta_function::equal_to, (Ogre::Vector3(Ogre::Vector3::*)(const Ogre::Vector3&)const)&Ogre::Vector3::operator==
      );

      lua.new_usertype<Ogre::Node>("OgreNode",
          "TS_LOCAL", sol::var(Ogre::Node::TS_LOCAL),
          "TS_WORLD", sol::var(Ogre::Node::TS_WORLD)
      );

      lua.new_usertype<Ogre::Quaternion>("Quaternion",
          sol::constructors<sol::types<const Ogre::Real&, const Ogre::Real&, const Ogre::Real&, const Ogre::Real&>, sol::types<const Ogre::Radian&, const Ogre::Vector3&>>(),
          "w", &Ogre::Quaternion::w,
          "x", &Ogre::Quaternion::x,
          "y", &Ogre::Quaternion::y,
          "z", &Ogre::Quaternion::z,
          "getPitch", &Ogre::Quaternion::getPitch,
          "getYaw", &Ogre::Quaternion::getYaw,
          "getRoll", &Ogre::Quaternion::getRoll,
          sol::meta_function::multiplication, sol::overload(
            (Ogre::Quaternion(Ogre::Quaternion::*)(const Ogre::Quaternion&)const)  &Ogre::Quaternion::operator*,
            (Ogre::Vector3(Ogre::Quaternion::*)(const Ogre::Vector3&)const)  &Ogre::Quaternion::operator*
          )
      );

      lua.new_usertype<Ogre::Radian>("Radian",
          sol::constructors<sol::types<float>>(),
          "degrees", sol::property(&Ogre::Radian::valueDegrees),
          "radians", sol::property(&Ogre::Radian::valueRadians)
      );

      lua.new_usertype<Ogre::Degree>("Degree",
          sol::constructors<sol::types<float>>(),
          "degrees", sol::property(&Ogre::Degree::valueDegrees),
          "radians", sol::property(&Ogre::Degree::valueRadians)
      );

      lua.new_usertype<Ogre::Matrix4>("Matrix4",
          sol::constructors<sol::types<float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float>>(),
          "row", [] (const Ogre::Matrix4& value, size_t row) -> const std::vector<float> {
            const float* r = value[row];
            return std::vector<float>(r, r + 4);
          }
      );

      lua["Entity"]["render"] = &Entity::getComponent<OgreRenderComponent>;
      lua["Engine"]["render"] = &Engine::getSystem<OgreRenderSystem>;
      LOG(INFO) << "Registered lua bindings for " << PLUGIN_NAME;
    }
    else
    {
      LOG(WARNING) << "Lua bindings for ogre plugin were not registered: lua state is nil";
    }
  }

  bool GsageOgrePlugin::installImpl()
  {
    mFacade->registerSystemFactory<OgreRenderSystem>();
    return true;
  }

  void GsageOgrePlugin::uninstallImpl()
  {
    if (mLuaInterface && mLuaInterface->getState())
    {
      sol::state_view& lua = *mLuaInterface->getSolState();

      lua["Engine"]["render"] = sol::lua_nil;
      lua["Entity"]["render"] = sol::lua_nil;
    }

    mFacade->removeSystemFactory<OgreRenderSystem>();
  }
}

Gsage::GsageOgrePlugin* ogrePlugin = NULL;

extern "C" bool PluginExport dllStartPlugin(Gsage::GsageFacade* facade)
{
  if(ogrePlugin != NULL)
  {
    return false;
  }
  ogrePlugin = new Gsage::GsageOgrePlugin();
  return facade->installPlugin(ogrePlugin);
}

extern "C" bool PluginExport dllStopPlugin(Gsage::GsageFacade* facade)
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
