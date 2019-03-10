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
#include <OgreHardwarePixelBuffer.h>
#include <OgreMovableObject.h>
#include <OgreVector3.h>
#include "RenderEvent.h"
#include "Entity.h"

#if OGRE_VERSION_MAJOR == 1
#include <OgreRenderQueueInvocation.h>
#include <RenderSystems/GL/OgreGLTexture.h>
#include <RenderSystems/GL/OgreGLFrameBufferObject.h>
#include <RenderSystems/GL/OgreGLFBORenderTexture.h>
#else
#include <RenderSystems/GL3Plus/OgreGL3PlusTexture.h>
#include <RenderSystems/GL3Plus/OgreGL3PlusFrameBufferObject.h>
#include <RenderSystems/GL3Plus/OgreGL3PlusFBORenderTexture.h>

#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorWorkspace.h>

#include <OgreHlmsUnlitDatablock.h>
#include <OgreHlmsUnlit.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsManager.h>
#include <OgreHlmsTextureManager.h>
#endif

#include "OgreSelectEvent.h"
#include "MouseEvent.h"
#include "Engine.h"

#include "components/RenderComponent.h"

namespace Gsage {

  RenderTarget::RenderTarget(const std::string& name, RenderTargetType::Type type, const DataProxy& parameters, Engine* engine)
    : mName(name)
    , mParameters(parameters)
    , mCurrentCamera(0)
    , mDefaultCamera(0)
    , mAutoUpdate(mParameters.get("autoUpdated", true))
    , mType(type)
    , mX(0)
    , mY(0)
    , mWidth(parameters.get("width", 320))
    , mHeight(parameters.get("height", 240))
    , mSamples(parameters.get("samples", 0))
    , mHasQueueSequence(false)
    , mEngine(engine)
    , mSceneManager(0)
    , mContinuousRaycast(false)
    , mMousePosition(Ogre::Vector2(0, 0))
    , mMouseOver(false)
    , mDestroying(false)
#if OGRE_VERSION >= 0x020100
    , mWorkspace(nullptr)
#else
    , mRenderQueueSequenceCreated(false)
#endif
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
    , mType(RenderTargetType::Generic)
    , mX(0)
    , mY(0)
    , mWidth(parameters.get("width", 320))
    , mHeight(parameters.get("height", 240))
    , mSamples(parameters.get("samples", 0))
    , mWrappedTarget(wrapped)
    , mHasQueueSequence(false)
    , mEngine(engine)
    , mSceneManager(0)
    , mContinuousRaycast(false)
    , mMousePosition(Ogre::Vector2(0, 0))
    , mMouseOver(false)
    , mDestroying(false)
#if OGRE_VERSION >= 0x020100
    , mWorkspace(nullptr)
#else
    , mRenderQueueSequenceCreated(false)
#endif
  {
    subscribe();
  }

  RenderTarget::~RenderTarget()
  {
    mDestroying = true;
#if OGRE_VERSION_MAJOR == 2
    destroyCurrentWorkspace();
#endif
    setCamera(nullptr);
  }

  void RenderTarget::subscribe()
  {
    int priority = mType == RenderTargetType::Rtt ? -200 : -199;

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

  std::tuple<Ogre::Ray, bool> RenderTarget::getRay() const
  {
    Ogre::Ray ray;
    if(!mMouseOver || !mCurrentCamera) {
      return std::make_tuple(ray, false);
    }

    ray = mCurrentCamera->getCameraToViewportRay((Ogre::Real) mMousePosition.x / (Ogre::Real) mWidth, (Ogre::Real) mMousePosition.y / (Ogre::Real) mHeight);
    return std::make_tuple(ray, true);
  }

  std::tuple<Ogre::Vector3, Entity*> RenderTarget::raycast(Ogre::Real defaultDistance, Ogre::Real closestDistance, unsigned int flags) const
  {
    Ogre::Vector3 result = Ogre::Vector3::ZERO;
    if(!mMouseOver || !mCurrentCamera) {
      return std::make_tuple(result, nullptr);
    }

    Ogre::MovableObject* hit = nullptr;
    if(!mCollisionTools->raycastFromCamera(mWidth, mHeight, mCurrentCamera, mMousePosition, result, hit, closestDistance, flags)) {
      bool success = false;
      Ogre::Ray ray;
      std::tie(ray, success) = getRay();
      if(success) {
        result = ray.getPoint(defaultDistance);
      }
    }

    Entity* target = nullptr;

    if(hit) {
      const Ogre::Any& entityId = hit->getUserObjectBindings().getUserAny("entity");
      if(!entityId.isEmpty()) {
        const std::string& id = entityId.get<const std::string>();
        target = mEngine->getEntity(id);
      }
    }

    return std::make_tuple(result, target);
  }

  void RenderTarget::doRaycasting(float offsetX, float offsetY, unsigned int flags, bool select)
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

    if((target->getQueryFlags() & RenderComponent::STATIC) == RenderComponent::STATIC && select)
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

  RenderTargetType::Type RenderTarget::getType() const
  {
    return mType;
  }

  void RenderTarget::initialize(Ogre::SceneManager* sceneManager)
  {
    if(mSceneManager != 0) {
      return;
    }

    mSceneManager = sceneManager;
    switchToDefaultCamera();

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
    if(mWrappedTarget == 0) {
      return;
    }

#if OGRE_VERSION_MAJOR == 1
    mWrappedTarget->removeAllViewports();
    if(!camera) {
      return;
    }

    Ogre::Viewport* vp = mWrappedTarget->addViewport(mCurrentCamera);
    configureViewport(vp);
#else
    if(mWorkspace) {
      destroyCurrentWorkspace();
    }

    if(!camera) {
      return;
    }

    Ogre::CompositorManager2 *compositorManager = Ogre::Root::getSingletonPtr()->getCompositorManager2();
    const Ogre::String workspaceName(mParameters.get("workspaceName", "basic"));

    auto backgroundColor = mParameters.get<Ogre::ColourValue>("viewport.backgroundColor", Ogre::ColourValue::Black);

    LOG(INFO) << "Create workspace " << workspaceName;
    if(!compositorManager->hasWorkspaceDefinition(workspaceName)) {
      compositorManager->createBasicWorkspaceDef(workspaceName, backgroundColor, Ogre::IdString());
    }

    Ogre::CompositorWorkspace* workspace = compositorManager->addWorkspace(mSceneManager, mWrappedTarget, mCurrentCamera,
        workspaceName, true );

    mWorkspace = workspace;
    configureWorkspace(workspace);
#endif
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
#if OGRE_VERSION_MAJOR == 1
      mWrappedTarget->update();
#else
      if(mWorkspace) {
        mWorkspace->_update();
      }
#endif
    }

    if(mContinuousRaycast)
      doRaycasting(mMousePosition.x, mMousePosition.y, RenderComponent::STATIC, true);
    else
      doRaycasting(mMousePosition.x, mMousePosition.y);
  }

#if OGRE_VERSION_MAJOR == 1
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
#else
  void RenderTarget::configureWorkspace(Ogre::CompositorWorkspace* workspace)
  {
  }

  void RenderTarget::destroyCurrentWorkspace()
  {
    if(mWorkspace) {
      mWorkspace->setEnabled(false);
      Ogre::Root::getSingletonPtr()->getCompositorManager2()->removeWorkspace(mWorkspace);
      mWorkspace = 0;
      const Ogre::String workspaceName(mParameters.get("workspaceName", "basic"));
      LOG(INFO) << "Delete workspace " << workspaceName;
    }
  }
#endif

  void RenderTarget::switchToDefaultCamera()
  {
    if(mDestroying)
      return;


    // create default camera
    auto name = mParameters.get("defaultCamera.name", "");
    if(!name.empty()) {
      if(!mDefaultCamera) {
        mDefaultCamera = mSceneManager->createCamera(name);
        LOG(INFO) << "[" << mName << "] " << "Created default camera " << name;
      }

      setCamera(mDefaultCamera);
      LOG(INFO) << "Using default camera " << mName;
    } else {
      setCamera(nullptr);
    }
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
    : RenderTarget(name, RenderTargetType::Rtt, parameters, engine)
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

  void RttRenderTarget::createTexture(const std::string& name,
      unsigned int width,
      unsigned int height,
      unsigned int samples,
      Ogre::PixelFormat pixelFormat) {


    OgreRenderSystem* rs = mEngine->getSystem<OgreRenderSystem>();
    if(!rs) {
      return;
    }

    if(mTexture) {
      rs->deleteTexture(rs->getName());
      removeEventListener(mTexture.get(), Texture::RECREATE, &RenderTarget::onTextureEvent);
      removeEventListener(mTexture.get(), Texture::DESTROY, &RenderTarget::onTextureEvent);
    }
#if OGRE_VERSION >= 0x020100
    destroyCurrentWorkspace();
#endif

    DataProxy params;
    params.put("width", width);
    params.put("height", height);
    params.put("fsaa", samples);
    params.put("pixelFormat", pixelFormat);
    params.put("usage", (int)Ogre::TU_RENDERTARGET);

    TexturePtr texture = rs->getTexture(name);
    if(texture) {
      texture->updateConfig(params);
      mTexture = texture;
    } else {
      mTexture = rs->createTexture(name, params);
    }
    wrap();
    addEventListener(mTexture.get(), Texture::RECREATE, &RenderTarget::onTextureEvent);
    addEventListener(mTexture.get(), Texture::DESTROY, &RenderTarget::onTextureEvent);

    LOG(DEBUG) << "Created RTT texture \"" << name << "\", size: " << width << "x" << height << ", samples: " << samples << ", pixel format: " << pixelFormat;
  }

  void RttRenderTarget::setDimensions(int width, int height) {
    mTexture->update(0, 0, width, height);
    RenderTarget::setDimensions(width, height);
  }

  bool RttRenderTarget::onTextureEvent(EventDispatcher* sender, const Event& event)
  {
#if OGRE_VERSION >= 0x020100
    destroyCurrentWorkspace();
#endif
    if(event.getType() == Texture::RECREATE) {
      wrap();
    } else {
      mWrappedTarget = 0;
    }
    return true;
  }

  void RttRenderTarget::wrap()
  {
    mWrappedTarget = static_cast<OgreTexture*>(mTexture.get())->getOgreTexture()->getBuffer()->getRenderTarget();
    if(mCurrentCamera) {
      // reattach camera to the new render target
      setCamera(mCurrentCamera);
    }
  }

  WindowRenderTarget::WindowRenderTarget(const std::string& name, const DataProxy& parameters, Engine* engine)
    : RenderTarget(name, RenderTargetType::Window, parameters, engine)
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
#if OGRE_VERSION_MAJOR == 1
      params["externalWindowHandle"] = handle;
#else
      params["parentWindowHandle"] = handle;
#endif
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
    mWidth = parameters.get("width", 1024) * parameters.get("scale", 1);
    mHeight = parameters.get("height", 768) * parameters.get("scale", 1);

    Ogre::RenderWindow* window = Ogre::Root::getSingletonPtr()->createRenderWindow(
        parameters.get("name", "default"),
        mWidth,
        mHeight,
        parameters.get("fullscreen", false),
        &params);

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

  RenderTargetPtr RenderTargetFactory::create(const std::string& name, RenderTargetType::Type type, const DataProxy& parameters, Engine* engine)
  {
    RenderTarget* target;
    switch(type) {
      case RenderTargetType::Rtt:
        target = new RttRenderTarget(name, parameters, engine);
        break;
      case RenderTargetType::Window:
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
