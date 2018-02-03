/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Artem Chernyshev

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

#include "RenderTarget.h"
#include <OgreRoot.h>
#include <OgreRenderQueueInvocation.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreMovableObject.h>
#include <OgreVector3.h>

#include <RenderSystems/GL/OgreGLTexture.h>
#include <RenderSystems/GL/OgreGLFrameBufferObject.h>
#include <RenderSystems/GL/OgreGLFBORenderTexture.h>

#include "OgreSelectEvent.h"
#include "MouseEvent.h"
#include "Engine.h"

#include "ogre/SceneNodeWrapper.h"

namespace Gsage {

  RenderTarget::RenderTarget(const std::string& name, Type type, const DataProxy& parameters, Engine* engine)
    : mName(name)
    , mParameters(parameters)
    , mCurrentCamera(0)
    , mAutoUpdate(mParameters.get("autoUpdated", true))
    , mType(type)
    , mX(0)
    , mY(0)
    , mWidth(parameters.get("width", 320))
    , mHeight(parameters.get("height", 240))
    , mRenderQueueSequenceCreated(false)
    , mSamples(parameters.get("samples", 0))
    , mHasQueueSequence(false)
    , mEngine(engine)
    , mSceneManager(0)
    , mContinuousRaycast(false)
    , mMousePosition(Ogre::Vector2(0, 0))
    , mMouseOver(false)
  {
    subscribe();
  }

  RenderTarget::RenderTarget(
      const std::string& name,
      Ogre::RenderTarget* wrapped, const DataProxy& parameters, Engine* engine)
    : mName(name)
    , mParameters(parameters)
    , mCurrentCamera(0)
    , mAutoUpdate(mParameters.get("autoUpdated", true))
    , mType(Generic)
    , mX(0)
    , mY(0)
    , mWidth(parameters.get("width", 320))
    , mHeight(parameters.get("height", 240))
    , mRenderQueueSequenceCreated(false)
    , mSamples(parameters.get("samples", 0))
    , mWrappedTarget(wrapped)
    , mHasQueueSequence(false)
    , mEngine(engine)
    , mSceneManager(0)
    , mContinuousRaycast(false)
    , mMousePosition(Ogre::Vector2(0, 0))
    , mMouseOver(false)
  {
    subscribe();
  }

  RenderTarget::~RenderTarget()
  {
  }

  void RenderTarget::subscribe()
  {
    int priority = mType == RenderTarget::Rtt ? -200 : -199;

    addEventListener(mEngine, MouseEvent::MOUSE_DOWN, &RenderTarget::handleMouseEvent, priority);
    addEventListener(mEngine, MouseEvent::MOUSE_UP, &RenderTarget::handleMouseEvent, priority);
    addEventListener(mEngine, MouseEvent::MOUSE_MOVE, &RenderTarget::handleMouseEvent, priority);

    addEventListener(mEngine, MouseEvent::MOUSE_DOWN, &RenderTarget::handleRaycast, 0);
    addEventListener(mEngine, MouseEvent::MOUSE_UP, &RenderTarget::handleRaycast, 0);
    addEventListener(mEngine, MouseEvent::MOUSE_MOVE, &RenderTarget::handleRaycast, 0);
  }

  bool RenderTarget::handleRaycast(EventDispatcher* sender, const Event& event) {
    const MouseEvent& e = static_cast<const MouseEvent&>(event);

    if(e.dispatcher != mName) {
      return true;
    }

    if(e.button == MouseEvent::Left)
    {
      if(e.getType() == MouseEvent::MOUSE_DOWN) {
        doRaycasting(e.mouseX, e.mouseY, 0xFFFF, true);
      } else if(e.getType() == MouseEvent::MOUSE_UP) {
        mContinuousRaycast = false;
      }
    }
    return true;
  }

  bool RenderTarget::handleMouseEvent(EventDispatcher* sender, const Event& event)
  {
    const MouseEvent& e = static_cast<const MouseEvent&>(event);

    if(e.dispatcher == mName || e.dispatcher != "main") {
      return true;
    }

    mMouseOver = e.mouseX > mX &&
       e.mouseY > mY &&
       e.mouseX < mX + mWidth &&
       e.mouseY < mY + mHeight;

    if(mMouseOver) {
      // fire new mouse event with another coordinates defined
      MouseEvent alteredEvent(e.getType(), mWidth, mHeight, e.button, mName);
      alteredEvent.setAbsolutePosition((e.mouseX - mX), (e.mouseY - mY), e.mouseZ);
      alteredEvent.setRelativePosition(e.relativeX, e.relativeY, e.relativeZ);
      mEngine->fireEvent(alteredEvent);
      mMousePosition = Ogre::Vector2(alteredEvent.mouseX, alteredEvent.mouseY);
    }

    return true;
  }

  void RenderTarget::doRaycasting(const float& offsetX, const float& offsetY, unsigned int flags, bool select)
  {
    Ogre::MovableObject* target;
    Ogre::Vector3 result;
    float closestDistance = 0.1f;

    if(!mCurrentCamera)
    {
      return;
    }

    bool hitObject = mCollisionTools->raycastFromCamera(mWidth, mHeight, mCurrentCamera, Ogre::Vector2(offsetX, offsetY), result, target, closestDistance, flags);

    if(!hitObject)
    {
      if(!mRolledOverObject.empty())
      {
        mEngine->fireEvent(OgreSelectEvent(SelectEvent::ROLL_OUT, 0x00, mRolledOverObject, result));
        mRolledOverObject.clear();
      }
      return;
    }

    const Ogre::Any& entityId = target->getUserObjectBindings().getUserAny("entity");
    if(entityId.isEmpty())
    {
      return;
    }

    if((target->getQueryFlags() & SceneNodeWrapper::STATIC) == SceneNodeWrapper::STATIC && select)
       mContinuousRaycast = true;

    const std::string& id = entityId.get<const std::string>();
    if(mRolledOverObject != id)
    {
      mEngine->fireEvent(OgreSelectEvent(OgreSelectEvent::ROLL_OUT, target->getQueryFlags(), mRolledOverObject, result));
      mRolledOverObject = id;
      mEngine->fireEvent(OgreSelectEvent(OgreSelectEvent::ROLL_OVER, target->getQueryFlags(), id, result));
    }

    if(select)
      mEngine->fireEvent(OgreSelectEvent(OgreSelectEvent::OBJECT_SELECTED, target->getQueryFlags(), id, result));
  }

  const std::string& RenderTarget::getName() const
  {
    return mName;
  }

  RenderTarget::Type RenderTarget::getType() const
  {
    return mType;
  }

  void RenderTarget::initialize(Ogre::SceneManager* sceneManager)
  {
    if(mSceneManager != 0) {
      return;
    }

    // create default camera
    auto name = mParameters.get("defaultCamera.name", "");
    if(!name.empty()) {
      setCamera(sceneManager->createCamera(name));
      LOG(INFO) << "[" << mName << "] " << "Created default camera " << name;
    }

    mSceneManager = sceneManager;
    mCollisionTools = std::make_shared<MOC::CollisionTools>(mSceneManager);
  }

  int RenderTarget::getWidth() const
  {
    return mWidth;
  }

  void RenderTarget::setWidth(int width)
  {
    setDimensions(width, mHeight);
  }

  int RenderTarget::getHeight() const
  {
    return mHeight;
  }

  void RenderTarget::setHeight(int height)
  {
    setDimensions(mWidth, height);
  }

  void RenderTarget::setDimensions(int width, int height)
  {
    if(mWidth == width && mHeight == height) {
      return;
    }

    mWidth = width;
    mHeight = height;

    updateCameraAspectRatio();
  }

  void RenderTarget::setPosition(int x, int y)
  {
    mX = x;
    mY = y;
  }

  void RenderTarget::setCamera(Ogre::Camera* camera)
  {
    mCurrentCamera = camera;
    mWrappedTarget->removeAllViewports();
    if(!camera) {
      return;
    }

    Ogre::Viewport* vp = mWrappedTarget->addViewport(mCurrentCamera);
    configureViewport(vp);
    updateCameraAspectRatio();
  }

  Ogre::Camera* RenderTarget::getCamera() {
    return mCurrentCamera;
  }

  Ogre::Viewport* RenderTarget::getViewport() {
    if(mWrappedTarget == 0) {
      return NULL;
    }

    return mWrappedTarget->getViewport(0);
  }

  void RenderTarget::update()
  {
    if(mWrappedTarget) {
      mWrappedTarget->update();
    }

    if(mContinuousRaycast)
      doRaycasting(mMousePosition.x, mMousePosition.y, SceneNodeWrapper::STATIC, true);
    else
      doRaycasting(mMousePosition.x, mMousePosition.y);
  }

  void RenderTarget::configureViewport(Ogre::Viewport* vp)
  {

    if(!mRenderQueueSequenceCreated) {
      mRenderQueueSequenceCreated = true;
      auto pair = mParameters.get<DataProxy>("viewport.renderQueueSequence");
      if (pair.second) {
        Ogre::RenderQueueInvocationSequence* queueSequence = Ogre::Root::getSingletonPtr()->createRenderQueueInvocationSequence(mName);
        for(auto queue : pair.first) {
          auto p = queue.second.get<int>("id");
          bool suppressShadows = false;
          int id = -1;
          if(p.second) {
            // try as DataProxy
            id = p.first;
            suppressShadows = queue.second.get("suppressShadows", false);
          } else {
            id = queue.second.as<int>();
          }

          if(id == -1) {
            LOG(ERROR) << "Malformed queue sequence definition";
            continue;
          }

          if(id > 0xFF) {
            LOG(ERROR) << "Can't use queue ID higher than " << 0xFF;
            continue;
          }
          Ogre::RenderQueueInvocation* rqi = new Ogre::RenderQueueInvocation(Ogre::uint8(id), mName);
          rqi->setSuppressShadows(suppressShadows);
          queueSequence->add(rqi);
        }
        LOG(INFO) << "Created RenderQueueInvocationSequence " << mName;
        mHasQueueSequence = true;
      }
    }

    if(mHasQueueSequence) {
      vp->setRenderQueueInvocationSequenceName(mName);
    }
    vp->setBackgroundColour(mParameters.get<Ogre::ColourValue>("viewport.backgroundColor", Ogre::ColourValue::Black));
    mWrappedTarget->setAutoUpdated(mAutoUpdate);
  }

  Ogre::RenderTarget* RenderTarget::getOgreRenderTarget()
  {
    return mWrappedTarget;
  }

  void RenderTarget::updateCameraAspectRatio()
  {
    if(!mCurrentCamera) {
      return;
    }
    Ogre::Real aspectRatio = Ogre::Real(mWidth) / Ogre::Real(mHeight);
    mCurrentCamera->setAspectRatio(aspectRatio);
  }

  bool RenderTarget::isAutoUpdated() const
  {
    return mAutoUpdate;
  }

  bool RenderTarget::isMouseOver() const
  {
    return mMouseOver;
  }

  RttRenderTarget::RttRenderTarget(const std::string& name, const DataProxy& parameters, Engine* engine)
    : RenderTarget(name, RenderTarget::Rtt, parameters, engine)
  {
    createTexture(
        mName,
        mWidth,
        mHeight,
        mSamples,
        Ogre::PF_R8G8B8A8
    );
  }

  RttRenderTarget::~RttRenderTarget()
  {
  }

  unsigned int RttRenderTarget::getGLID() const {
    Ogre::GLTexture *nativeTexture = static_cast<Ogre::GLTexture*>(mTexture.get());
    return nativeTexture->getGLID();
  }

  void RttRenderTarget::createTexture(const std::string& name,
      unsigned int width,
      unsigned int height,
      unsigned int samples,
      Ogre::PixelFormat pixelFormat) {

    Ogre::TextureManager& texManager = Ogre::TextureManager::getSingleton();
    if(texManager.resourceExists(name)) {
      texManager.remove(name);
    }

    mTexture = texManager.createManual(name,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D,
        width,
        height,
        0,
        pixelFormat,
        Ogre::TU_RENDERTARGET,
        0,
        false,
        samples);
    mWrappedTarget = mTexture->getBuffer()->getRenderTarget();
    if(mCurrentCamera) {
      // reattach camera to the new render target
      setCamera(mCurrentCamera);
    }
#if GSAGE_PLATFORM != GSAGE_WIN32
    // for some reason it does not play well on Windows
    mWrappedTarget->update();
#endif

    LOG(DEBUG) << "Created RTT texture \"" << name << "\", size: " << width << "x" << height << ", samples: " << samples << ", pixel format: " << pixelFormat;
  }

  void RttRenderTarget::setDimensions(int width, int height) {
    createTexture(
        mName,
        width,
        height,
        mSamples,
        Ogre::PF_R8G8B8A8
    );
    RenderTarget::setDimensions(width, height);
  }

  WindowRenderTarget::WindowRenderTarget(const std::string& name, const DataProxy& parameters, Engine* engine)
    : RenderTarget(name, RenderTarget::Window, parameters, engine)
  {
    Ogre::NameValuePairList params;
    auto paramNode = parameters.get<DataProxy>("params");
    if(paramNode.second)
    {
      for(auto& pair : paramNode.first)
      {
        params[pair.first] = pair.second.as<std::string>();
      }
    }
#if GSAGE_PLATFORM == GSAGE_APPLE
    params["macAPI"] = "cocoa";
#endif
    if(parameters.get("useWindowManager", false)) {
      std::string handle = parameters.get<std::string>("windowHandle", "");
      std::string glContext = parameters.get<std::string>("glContext", "");
#if GSAGE_PLATFORM == GSAGE_APPLE
      params["macAPICocoaUseNSView"] = "true";
      params["externalWindowHandle"] = handle;
#else
#if GSAGE_PLATFORM == GSAGE_WIN32
      if(!glContext.empty()) {
        params["externalGLContext"] = glContext;
        params["externalGLControl"] = "True";
      }
      params["externalWindowHandle"] = handle;
#endif
      params["parentWindowHandle"] = handle;
#endif
    }
    Ogre::RenderWindow* window = Ogre::Root::getSingletonPtr()->createRenderWindow(
        parameters.get("name", "default"),
        parameters.get("width", 1024),
        parameters.get("height", 786),
        parameters.get("fullscreen", false),
        &params);

    mWidth = window->getWidth();
    mHeight = window->getHeight();

    if(parameters.get("useWindowManager", false)) {
      window->setVisible(false);
    } else {
      window->setVisible(parameters.get("visible", true));
    }
    mWrappedTarget = window;
  }

  WindowRenderTarget::~WindowRenderTarget()
  {
  }

  void WindowRenderTarget::setDimensions(int width, int height)
  {
    Ogre::RenderWindow* window = static_cast<Ogre::RenderWindow*>(mWrappedTarget);
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
    window->resize(width, height);
#endif
    window->windowMovedOrResized();
    RenderTarget::setDimensions(width, height);
  }

  RenderTargetPtr RenderTargetFactory::create(const std::string& name, RenderTarget::Type type, const DataProxy& parameters, Engine* engine)
  {
    RenderTarget* target;
    switch(type) {
      case RenderTarget::Rtt:
        target = new RttRenderTarget(name, parameters, engine);
        break;
      case RenderTarget::Window:
        target = new WindowRenderTarget(name, parameters, engine);
        break;
      default:
        target = nullptr;
        break;
    }

    return target;
  }

  RenderTargetPtr wrap(Ogre::RenderTarget* renderTarget, const std::string& name, const DataProxy& parameters, Engine* engine)
  {
    return new RenderTarget(name, renderTarget, parameters, engine);
  }
}
