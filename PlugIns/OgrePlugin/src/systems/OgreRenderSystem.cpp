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

#include "systems/OgreRenderSystem.h"
#include <RenderSystems/GL/OgreGLTexture.h>
#include <RenderSystems/GL/OgreGLFrameBufferObject.h>
#include <RenderSystems/GL/OgreGLFBORenderTexture.h>

#include "ogre/SceneNodeWrapper.h"
#include "ogre/EntityWrapper.h"
#include "ogre/LightWrapper.h"
#include "ogre/BillboardWrapper.h"
#include "ogre/ParticleSystemWrapper.h"
#include "ogre/CameraWrapper.h"

#include "RenderEvent.h"
#include "EngineEvent.h"
#include "Logger.h"

#include "AnimationScheduler.h"

#include "ComponentStorage.h"
#include "Entity.h"
#include "Engine.h"
#include "EngineEvent.h"

#include "ResourceManager.h"

#if GSAGE_PLATFORM == GSAGE_APPLE
#include <Overlay/OgreFontManager.h>
#else
#include <OgreFontManager.h>
#endif
#include <OgreParticleSystemManager.h>
#include "ogre/ManualMovableTextRenderer.h"
#include "WindowEventListener.h"


namespace Gsage {

  void OgreLogRedirect::messageLogged(const std::string& message,
                                      Ogre::LogMessageLevel lml,
                                      bool maskDebug,
                                      const std::string& system,
                                      bool& skipMessage)
  {
     {
       switch(lml)
       {
         case Ogre::LogMessageLevel::LML_TRIVIAL:
           LOG(TRACE) << message;
           break;
         case Ogre::LogMessageLevel::LML_NORMAL:
           LOG(INFO) << message;
           break;
         case Ogre::LogMessageLevel::LML_CRITICAL:
           LOG(ERROR) << message;
           break;
       }
     }
  }

  const std::string OgreRenderSystem::CAMERA_CHANGED = "cameraChanged";
  // Render system identifier for the factory registration
  const std::string OgreRenderSystem::ID = "ogre";

  OgreRenderSystem::OgreRenderSystem() :
    mRoot(0),
    mFontManager(0),
    mManualMovableTextParticleFactory(0),
    mResourceManager(0),
    mWindow(0),
    mViewport(0),
    mOgreInteractionManager(0),
    mWindowEventListener(0)
  {
    mSystemInfo.put("type", OgreRenderSystem::ID);
    mLogManager = new Ogre::LogManager();
  }

  OgreRenderSystem::~OgreRenderSystem()
  {
    if(mOgreInteractionManager)
      delete mOgreInteractionManager;
    if(mFontManager != 0)
      delete mFontManager;

    // TODO: this probably can lead to some unexpected behavior in the future
    // if the system will be rendered into an external window
    if(mWindow != 0)
      mWindowEventListener->windowClosed(mWindow);
    if(mRoot != 0)
      delete mRoot;
    if(mManualMovableTextParticleFactory != 0)
      delete mManualMovableTextParticleFactory;
    if(mResourceManager != 0)
      delete mResourceManager;
    if(mWindowEventListener != 0)
      delete mWindowEventListener;

    delete mLogManager;
  }

  bool OgreRenderSystem::initialize(const DataProxy& settings)
  {
    std::string workdir   = mEngine->env().get("workdir", ".");
    std::string plugins   = workdir + GSAGE_PATH_SEPARATOR + settings.get("pluginsFile", ".");
    std::string config    = workdir + GSAGE_PATH_SEPARATOR + settings.get("configFile", ".");

    // redirect ogre logs to custom logger
    mLogManager->createLog("", true, false, false);
    mLogManager->getDefaultLog()->addListener(&mLogRedirect);

    // initialize ogre root
    mRoot = new Ogre::Root(plugins, config, "");
    // initialize resource manager
    mResourceManager = new ResourceManager(workdir);

    if(!(mRoot->restoreConfig()))
        return false;

    // initialize render window
    mRoot->initialise(false);

    Ogre::NameValuePairList params;

    auto paramNode = settings.get<DataProxy>("window.params");
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

    // create window
    mWindow = mRoot->createRenderWindow(
        settings.get("window.name", ""),
        settings.get("window.width", 1024),
        settings.get("window.height", 786),
        settings.get("window.fullscreen", false),
        &params);
    mWindow->setVisible(settings.get("window.visible", true));
    mWindowEventListener = new WindowEventListener(mWindow, mEngine);
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, mWindowEventListener);

    // initialize scene manager
    mSceneManager = mRoot->createSceneManager(settings.get("sceneManager", "OctreeSceneManager"));
    // TODO: make it customizable via config
    mSceneManager->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_MODULATIVE);
    mSceneManager->addRenderQueueListener(this);

    mFontManager = new Ogre::FontManager();
    // initializing custom factory for floating text particles
    mManualMovableTextParticleFactory = new Ogre::ManualMovableTextRendererFactory();
    Ogre::ParticleSystemManager::getSingletonPtr()->addRendererFactory(mManualMovableTextParticleFactory);

    auto resources = settings.get<DataProxy>("globalResources");
    if(resources.second && !mResourceManager->load(resources.first))
      return false;

    mRenderSystem = mRoot->getRenderSystem();

    assert(mRenderSystem != 0);

    // get viewport to initialize it
    getViewport();

    DataProxy rtt = settings.get("rtt", DataProxy());
    if(rtt.size() > 0) {
      for(auto pair : rtt) {
        const std::string name = pair.first;
        unsigned int width = pair.second.get<unsigned int>("width", 1);
        unsigned int height = pair.second.get<unsigned int>("height", 1);
        unsigned int samples = pair.second.get<unsigned int>("samples", 0);
        Ogre::PixelFormat pf = pair.second.get<Ogre::PixelFormat>("format", Ogre::PF_R8G8B8A8);

        createRttTexture(name, width, height, samples, pf);

        auto camera = pair.second.get<std::string>("camera");
        if(camera.second) {
          renderCameraToTarget(camera.first, name);
        }
      }
    }

    // initialize built-in types
    mObjectManager.registerElement<SceneNodeWrapper>();
    mObjectManager.registerElement<EntityWrapper>();
    mObjectManager.registerElement<LightWrapper>();
    mObjectManager.registerElement<BillboardSetWrapper>();
    mObjectManager.registerElement<ParticleSystemWrapper>();
    mObjectManager.registerElement<CameraWrapper>();

    mRoot->clearEventTimes();
    mRenderSystem->_initRenderTargets();
    mOgreInteractionManager = new OgreInteractionManager();
    mOgreInteractionManager->initialize(this, mEngine);
    EngineSystem::initialize(settings);
    return true;
  }

  bool OgreRenderSystem::fillComponentData(RenderComponent* c, const DataProxy& dict)
  {
    if(!c->getResources().empty())
    {
      if(!mResourceManager->load(c->getResources()))
        return false;
    }

    auto element = dict.get<DataProxy>("root");
    if(element.second)
    {
      c->mRootNode = mObjectManager.create<SceneNodeWrapper>(element.first, c->getOwner()->getId(), mSceneManager);
    }
    element = dict.get<DataProxy>("animations");
    if(element.second)
    {
      LOG(INFO) << "Read animations for component of entity " << c->getOwner()->getId();
      c->mAnimationScheduler.initialize(element.first, mSceneManager, c);
    }
    c->mAddedToScene = true;

    return true;
  }

  void OgreRenderSystem::update(const double& time)
  {
    ComponentStorage<RenderComponent>::update(time);
    Ogre::WindowEventUtilities::messagePump();

    mOgreInteractionManager->update(time);

    mEngine->fireEvent(RenderEvent(RenderEvent::UPDATE, this));

    bool continueRendering = !mWindow->isClosed();
    if(continueRendering)
      continueRendering = mRoot->renderOneFrame();

    if(!continueRendering)
      mEngine->fireEvent(EngineEvent(EngineEvent::HALT));
  }

  void OgreRenderSystem::updateComponent(RenderComponent* component, Entity* entity, const double& time)
  {
    component->mAnimationScheduler.update(time);
  }

  bool OgreRenderSystem::configure(const DataProxy& config)
  {

    auto resources = mConfig.get<DataProxy>("resources");
    if(resources.second)
      mResourceManager->unload(resources.first);

    EngineSystem::configure(config);

    resources = mConfig.get<DataProxy>("resources");
    if(resources.second)
      mResourceManager->load(resources.first);
    mSceneManager->setAmbientLight(config.get("colourAmbient", Ogre::ColourValue()));
    if(config.count("fog") != 0)
    {
      mSceneManager->setFog(
          Ogre::FOG_LINEAR, // TODO: do it propely
          config.get("fog.colour", Ogre::ColourValue()),
          config.get("fog.density", 0.0f),
          config.get("fog.start", 0.0f),
          config.get("fog.end", 0.0f)
      );
    }

    if(config.count("skybox") != 0)
    {
      mSceneManager->setSkyBox(true, config.get("skybox", ""));
    }

    return true;
  }

  DataProxy& OgreRenderSystem::getConfig()
  {
    return mConfig;
  }

  void OgreRenderSystem::renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation)
  {
    if(queueGroupId == Ogre::RENDER_QUEUE_OVERLAY)
      mEngine->fireEvent(RenderEvent(RenderEvent::UPDATE_UI, this, queueGroupId, invocation));
  }

  void OgreRenderSystem::renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation)
  {
    mEngine->fireEvent(RenderEvent(RenderEvent::RENDER_QUEUE_ENDED, this, queueGroupId, invocation));
  }

  bool OgreRenderSystem::removeComponent(RenderComponent* component)
  {
    LOG(INFO) << "Remove component " << component->getOwner()->getId();
    if(component->mRootNode)
    {
      component->mRootNode->destroy();
      component->mRootNode = 0;
    }
    component->mAddedToScene = false;

    mResourceManager->unload(component->getResources());
    ComponentStorage<RenderComponent>::removeComponent(component);
    return true;
  }

  OgreRenderSystem::OgreEntities OgreRenderSystem::getEntities(const unsigned int& query)
  {
    OgreRenderSystem::OgreEntities res;
    Ogre::SceneManager::MovableObjectIterator iterator = mSceneManager->getMovableObjectIterator("Entity");
    while(iterator.hasMoreElements())
    {
      Ogre::Entity* e = static_cast<Ogre::Entity*>(iterator.getNext());
      if((e->getQueryFlags() | query) == query)
        res.push_back(e);
    }
    return res;
  }

  Ogre::Camera* OgreRenderSystem::getOgreCamera()
  {
    return mCurrentCamera;
  }

  OgreRenderSystem::Entities OgreRenderSystem::getObjectsInRadius(const Ogre::Vector3& center, const float& distance, const unsigned int flags, const std::string& id)
  {
    Ogre::Sphere sphere(center, distance);
    Ogre::SphereSceneQuery* query = mSceneManager->createSphereQuery(sphere, flags);
    Ogre::SceneQueryResultMovableList nodes;

    nodes = query->execute().movables;
    Entities res;
    if(nodes.empty())
      return res;

    Ogre::Any entityId;
    for(Ogre::MovableObject* element : nodes)
    {
      entityId = element->getUserObjectBindings().getUserAny("entity");
      if(entityId.isEmpty())
        continue;

      if(!id.empty() && id != entityId.get<std::string>())
        continue;

      Entity* entity = mEngine->getEntity(entityId.get<std::string>());
      if(!entity)
        continue;

      res.push_back(entity);
    }
    mSceneManager->destroyQuery(query);
    return res;
  }

  void OgreRenderSystem::updateCurrentCamera(Ogre::Camera* camera)
  {
    mCurrentCamera = camera;
    fireEvent(Event(OgreRenderSystem::CAMERA_CHANGED));
  }

  Ogre::Viewport* OgreRenderSystem::getViewport()
  {
    if(mViewport == 0) {
      updateCurrentCamera(mSceneManager->createCamera("main"));
      mViewport = mWindow->addViewport(mCurrentCamera);
    }
    return mViewport;
  }

  unsigned int OgreRenderSystem::getFBOID(const std::string& target) const
  {
    if(mRttTextures.count(target) == 0) {
      LOG(WARNING) << "Attempt to get fboid of not existing render target: " << target;
      return 0;
    }

    Ogre::GLTexture *nativeTexture = static_cast<Ogre::GLTexture*>(mRttTextures.at(target).get());
    return nativeTexture->getGLID();
  }

  void OgreRenderSystem::setWidth(unsigned int width, const std::string& target)
  {
    if(target.empty()) {
      LOG(WARNING) << "Setting width for window is not implemented yet";
    }

    if(mRttTextures.count(target) == 0) {
      LOG(WARNING) << "Attempt to set width to not existing render target: " << target;
      return;
    }

    Ogre::TexturePtr texture = mRttTextures[target];
    createRttTexture(target, width, texture->getHeight(), texture->getFSAA(), texture->getFormat());
  }

  unsigned int OgreRenderSystem::getWidth(const std::string& target) const
  {
    if(target.empty())
      return mWindow ? mWindow->getWidth() : 0;

    if(mRttTextures.count(target) == 0) {
      LOG(WARNING) << "Attempt to get height of not existing render target: " << target;
      return 0;
    }

    return mRttTextures.at(target)->getWidth();
  }

  void OgreRenderSystem::setHeight(unsigned int height, const std::string& target)
  {
    if(target.empty()) {
      LOG(WARNING) << "Setting height for window is not implemented yet";
    }

    if(mRttTextures.count(target) == 0) {
      LOG(WARNING) << "Attempt to set height to not existing render target: " << target;
      return;
    }

    Ogre::TexturePtr texture = mRttTextures[target];
    createRttTexture(target, texture->getWidth(), height, texture->getFSAA(), texture->getFormat());
  }

  unsigned int OgreRenderSystem::getHeight(const std::string& target) const
  {
    if(target.empty())
      return mWindow ? mWindow->getHeight() : 0;

    if(mRttTextures.count(target) == 0) {
      LOG(WARNING) << "Attempt to get height of not existing render target: " << target;
      return 0;
    }

    return mRttTextures.at(target)->getHeight();
  }

  void OgreRenderSystem::setSize(unsigned int width, unsigned int height, const std::string& target)
  {
    if(mRttTextures.count(target) == 0) {
      LOG(WARNING) << "Attempt to set size of not existing render target: " << target;
      return;
    }
    Ogre::TexturePtr texture = mRttTextures[target];

    createRttTexture(target, width, height, texture->getFSAA(), texture->getFormat());
  }

  void OgreRenderSystem::createRttTexture(const std::string& name,
      unsigned int width,
      unsigned int height,
      unsigned int samples,
      Ogre::PixelFormat pixelFormat) {

    Ogre::TextureManager& texManager = Ogre::TextureManager::getSingleton();
    if(texManager.resourceExists(name))
      texManager.remove(name);

    mRttTextures[name] = texManager.createManual(name,
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
    LOG(INFO) << "Created RTT texture \"" << name << "\", size: " << width << "x" << height << ", samples: " << samples << ", pixel format: " << pixelFormat;
  }

  void OgreRenderSystem::renderCameraToTarget(const std::string& cameraName, const std::string& target) {
    if(mRttTextures.count(target) == 0) {
      return;
    }

    Ogre::Camera* cam = NULL;
    if(cameraName == "__current__") {
      cam = mCurrentCamera;
    } else {
      try {
        cam = mSceneManager->getCamera(cameraName);
      } catch(...) {
        LOG(WARNING) << "Camera with name " << cameraName << " does not exist";
      }
    }

    if(cam == NULL) {
      LOG(INFO) << "Failed to make camera " << cameraName << " render to target " << target;
      return;
    }

    Ogre::TexturePtr texture = mRttTextures[target];

    Ogre::RenderTarget* renderTarget = texture->getBuffer()->getRenderTarget();

    renderTarget->addViewport(cam);
    renderTarget->getViewport(0)->setClearEveryFrame(true);
    renderTarget->getViewport(0)->setBackgroundColour(Ogre::ColourValue::Black);
    renderTarget->getViewport(0)->setOverlaysEnabled(true);
    renderTarget->setAutoUpdated(false);

    Ogre::Real aspectRatio = texture->getWidth() / texture->getHeight();
    cam->setAspectRatio(aspectRatio);
  }
}
