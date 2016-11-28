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

#include "CameraFactory.h"

#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>

#include "components/RenderComponent.h"
#include "systems/OgreRenderSystem.h"
#include "ogre/SceneNodeWrapper.h"

#include "Engine.h"
#include "EngineEvent.h"
#include "MouseEvent.h"
#include "KeyboardEvent.h"
#include "CollisionTools.h"

namespace Gsage {

  std::string IsometricCameraController::TYPE = "isometric";
  std::string WASDCameraController::TYPE = "wasd";
  std::string LuaCameraController::TYPE = "lua";

  CameraController::CameraController()
    : mTarget("")
    , mCamera(0)
    , mContinuousRaycast(false)
  {
    BIND_PROPERTY("target", &mTarget);
    BIND_ACCESSOR("id", &CameraController::createCamera, &CameraController::getId);
    BIND_ACCESSOR("bgColour", &CameraController::setBgColour, &CameraController::getBgColour);
    BIND_ACCESSOR("clipDistance", &CameraController::setClipDistance, &CameraController::getClipDistance);
  }

  CameraController::~CameraController()
  {
    mSceneManager->destroyCamera(mCamera);
  }

  void CameraController::initialize(OgreRenderSystem* renderSystem, Engine* engine)
  {
    mViewport = renderSystem->getViewport();
    mSceneManager = renderSystem->getSceneManager();
    mEngine = engine;

    addEventListener(mEngine, MouseEvent::MOUSE_DOWN, &CameraController::onMouseButton);
    addEventListener(mEngine, MouseEvent::MOUSE_UP, &CameraController::onMouseButton);
    addEventListener(mEngine, MouseEvent::MOUSE_MOVE, &CameraController::onMouseMove);
    addEventListener(mEngine, KeyboardEvent::KEY_UP, &CameraController::handleKeyboardEvent);
    addEventListener(mEngine, KeyboardEvent::KEY_DOWN, &CameraController::handleKeyboardEvent);
  }

  bool CameraController::onMouseMove(EventDispatcher* sender, const Event& event)
  {
    const MouseEvent& e = (MouseEvent&) event;
    Ogre::Vector3 delta;

    mMousePosition = Ogre::Vector3(e.mouseX, e.mouseY, e.mouseZ);
    return true;
  }

  bool CameraController::setTarget(const std::string& id)
  {
    if(!mEngine->getComponent<RenderComponent>(id))
    {
      LOG(INFO) << "Failed to attach to entity with id " << id << ": entity does not exist or does not have required component";
      return false;
    }

    mTarget = id;
    return true;
  }

  Ogre::Camera* CameraController::getCamera()
  {
    return mCamera;
  }

  const std::string& CameraController::getId() const
  {
    return mId;
  }

  void CameraController::setBgColour(const Ogre::ColourValue& value)
  {
    mViewport->setBackgroundColour(value);
  }

  const Ogre::ColourValue& CameraController::getBgColour() const
  {
    return mViewport->getBackgroundColour();
  }

  void CameraController::setClipDistance(const float& value)
  {
    mCamera->setNearClipDistance(value);
  }

  float CameraController::getClipDistance()
  {
    return mCamera->getNearClipDistance();
  }


  IsometricCameraController::IsometricCameraController()
    : mMoveCamera(false)
    , mCenter(Ogre::Vector3::ZERO)
    , mMaxDistance(100)
    , mMinDistance(1)
    , mMaxAngle(80)
    , mMinAngle(10)
    , mUAngle(45)
    , mVAngle(45)
    , mZoomStepMultiplier(0.1f)
    , mCameraOffset(Ogre::Vector3::ZERO)
    , mDistance(10)
  {

    BIND_PROPERTY("maxDistance", &mMaxDistance);
    BIND_PROPERTY("minDistance", &mMinDistance);
    BIND_PROPERTY("maxAngle", &mMaxAngle);
    BIND_PROPERTY("minAngle", &mMinAngle);

    BIND_PROPERTY("uAngle", &mUAngle);
    BIND_PROPERTY("vAngle", &mVAngle);
    BIND_PROPERTY("distance", &mDistance);
    BIND_PROPERTY("zoomStepMultiplier", &mZoomStepMultiplier);

    BIND_READONLY_PROPERTY("type", &IsometricCameraController::TYPE);

    BIND_PROPERTY("cameraOffset", &mCameraOffset);
  }

  IsometricCameraController::~IsometricCameraController()
  {
  }

  void IsometricCameraController::createCamera(const std::string& id)
  {
    LOG(TRACE) << "Create camera with id " << id;
    mId = id;
    mCamera = mSceneManager->hasCamera(id) ? mSceneManager->getCamera(id) : mSceneManager->createCamera(id);
    mViewport->setCamera(mCamera);
    mCamera->setAspectRatio(
        float(mViewport->getActualWidth()) / float(mViewport->getActualHeight()));
  }

  void IsometricCameraController::update(const double& time)
  {
    if(mTarget.empty())
      return;

    RenderComponent* render = mEngine->getComponent<RenderComponent>(mTarget);
    if(render)
      mCenter = render->getPosition() + mCameraOffset;

    mUAngle = std::max(mUAngle, mMinAngle);
    mUAngle = std::min(mUAngle, mMaxAngle);
    mDistance = std::max(mDistance, mMinDistance);
    mDistance = std::min(mDistance, mMaxDistance);

    Ogre::Real teta = mUAngle.valueRadians();
    Ogre::Real phi = mVAngle.valueRadians();

    Ogre::Vector3 position(
      mCenter.x + mDistance * sin(teta) * cos(phi),
      mCenter.y + mDistance * cos(teta),
      mCenter.z + mDistance * sin(teta) * sin(phi)
    );

    if(mCamera->getPosition() == position)
      return;

    mCamera->setPosition(position);
    mCamera->lookAt(mCenter);
  }

  bool IsometricCameraController::onMouseButton(EventDispatcher* sender, const Event& event)
  {
    bool res = CameraController::onMouseButton(sender, event);
    const MouseEvent& e = (MouseEvent&) event;
    if(e.button == MouseEvent::Right)
    {
      mMoveCamera = e.getType() == MouseEvent::MOUSE_DOWN;
    }
    return res;
  }

  bool IsometricCameraController::onMouseMove(EventDispatcher* sender, const Event& event)
  {
    const MouseEvent& e = (MouseEvent&) event;
    Ogre::Vector3 delta;

    delta.x = e.mouseX - mMousePosition.x;
    delta.y = e.mouseY - mMousePosition.y;
    if(e.relativeZ != 0)
      mDistance += e.relativeZ * mZoomStepMultiplier;
    bool res = CameraController::onMouseMove(sender, event);

    if(!mMoveCamera)
      return res;

    mUAngle -= Ogre::Degree(delta.y);
    mVAngle += Ogre::Degree(delta.x);
    return res;
  }

  WASDCameraController::WASDCameraController()
    : mMoveSpeed(1)
    , mMoveVector(Ogre::Vector3::ZERO)
    , mPitchAngle(0)
    , mYawAngle(0)
    , mRotate(false)
  {
    BIND_PROPERTY("position", &mPosition);
    BIND_PROPERTY("moveSpeed", &mMoveSpeed);
  }

  WASDCameraController::~WASDCameraController()
  {
  }

  void WASDCameraController::createCamera(const std::string& id)
  {
    LOG(TRACE) << "Create camera with id " << id;
    mId = id;
    mCamera = mSceneManager->hasCamera(id) ? mSceneManager->getCamera(id) : mSceneManager->createCamera(id);
    mCamera->setAspectRatio(
        float(mViewport->getActualWidth()) / float(mViewport->getActualHeight()));
    mViewport->setCamera(mCamera);
  }

  void WASDCameraController::update(const double& time)
  {
    mPosition += mMoveVector;
    mCamera->setPosition(mPosition);

    if(mYawAngle != 0)
    {
      mCamera->yaw(Ogre::Degree(mYawAngle));
      mYawAngle = 0;
    }

    if(mPitchAngle != 0)
    {
      mCamera->pitch(Ogre::Degree(mPitchAngle));
      mPitchAngle = 0;
    }
  }

  bool WASDCameraController::onMouseButton(EventDispatcher* sender, const Event& event)
  {
    bool res = CameraController::onMouseButton(sender, event);
    const MouseEvent& e = (MouseEvent&) event;
    mMousePosition = Ogre::Vector3(e.mouseX, e.mouseY, 0);
    if(e.button == MouseEvent::Right)
    {
      mRotate = e.getType() == MouseEvent::MOUSE_DOWN;
    }
    return res;
  }

  bool WASDCameraController::onMouseMove(EventDispatcher* sender, const Event& event)
  {
    bool res = CameraController::onMouseMove(sender, event);
    if (!mRotate)
    {
      return res;
    }
    const MouseEvent& e = (MouseEvent&) event;
    Ogre::Vector3 delta;
    delta.x = e.mouseX - mMousePosition.x;
    delta.y = e.mouseY - mMousePosition.y;
    if(mRotate) {
      mYawAngle = -delta.x/2;
      mPitchAngle = -delta.y/2;
    }
    mMousePosition = Ogre::Vector3(e.mouseX, e.mouseY, 0);

    return res;
  }

  bool WASDCameraController::handleKeyboardEvent(EventDispatcher* sender, const Event& event) {
    const KeyboardEvent& e = static_cast<const KeyboardEvent&>(event);
    Ogre::Vector3 direction;

    switch(e.key) {
      case KeyboardEvent::KC_W:
        direction = -Ogre::Vector3::UNIT_Z * mMoveSpeed;
        break;
      case KeyboardEvent::KC_A:
        direction = -Ogre::Vector3::UNIT_X * mMoveSpeed;
        break;
      case KeyboardEvent::KC_S:
        direction = Ogre::Vector3::UNIT_Z * mMoveSpeed;
        break;
      case KeyboardEvent::KC_D:
        direction = Ogre::Vector3::UNIT_X * mMoveSpeed;
        break;
      default:
        return true;
    }

    mMoveVector += e.getType() == KeyboardEvent::KEY_UP ? -direction : direction;
    return true;
  }

  LuaCameraController::LuaCameraController()
  {
    BIND_PROPERTY("script", &mScript);
  }

  LuaCameraController::~LuaCameraController()
  {
  }

  bool LuaCameraController::read(const Dictionary& dict)
  {
    // read all all parameters
    bool res = CameraController::read(dict);
    // initialize lua script
    return res;
  }

  void LuaCameraController::createCamera(const std::string& id)
  {
    LOG(TRACE) << "Create camera with id " << id;
    mId = id;
    mCamera = mSceneManager->hasCamera(id) ? mSceneManager->getCamera(id) : mSceneManager->createCamera(id);
    mCamera->setAspectRatio(
        float(mViewport->getActualWidth()) / float(mViewport->getActualHeight()));
    mViewport->setCamera(mCamera);
  }

  void LuaCameraController::update(const double& time)
  {
  }

  bool LuaCameraController::onMouseButton(EventDispatcher* sender, const Event& event)
  {
    return true;
  }

  bool LuaCameraController::onMouseMove(EventDispatcher* sender, const Event& event)
  {
    return true;
  }

  bool LuaCameraController::handleKeyboardEvent(EventDispatcher* sender, const Event& event) {
    return true;
  }

  // ---------------------------------------------------------------------------------------------------------------

  template<typename TController>
  CameraController* ConcreteControllerFactory<TController>::create(const Dictionary& settings, OgreRenderSystem* renderSystem, Engine* engine)
  {
    TController* res = new TController();
    res->initialize(renderSystem, engine);
    res->read(settings);
    return res;
  }

  CameraFactory::CameraFactory()
    : mCameraController(NULL)
  {
    registerControllerType<IsometricCameraController>();
    registerControllerType<WASDCameraController>();
  }

  CameraFactory::~CameraFactory()
  {
    delete mCameraController;

    for(auto& pair : mControllerFactories)
    {
      delete pair.second;
    }

    mControllerFactories.clear();
  }

  template<typename TController>
  void CameraFactory::registerControllerType()
  {
    mControllerFactories[TController::TYPE] = new ConcreteControllerFactory<TController>();
  }

  CameraController* CameraFactory::initializeController(const Dictionary& settings, OgreRenderSystem* renderSystem, Engine* engine)
  {
    auto pair = settings.get<std::string>("type");
    if(!pair.second)
    {
      LOG(ERROR) << "Failed to create camera controller: controller type not found in settings";
      return NULL;
    }

    std::string type = pair.first;
    if(mControllerFactories.count(type) == 0)
    {
      LOG(ERROR) << "Failed to create camera controller: unknown type name: \"" << type << "\"";
      return NULL;
    }

    if(mCameraController != NULL)
      delete mCameraController;

    LOG(INFO) << "Initialize controller of type " << type;
    return mCameraController = mControllerFactories[type]->create(settings, renderSystem, engine);
  }
}
