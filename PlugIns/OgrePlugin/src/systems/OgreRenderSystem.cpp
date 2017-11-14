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
#include "GsageFacade.h"
#include "EngineEvent.h"
#include "WindowManager.h"

#include "ResourceManager.h"

#if GSAGE_PLATFORM == GSAGE_APPLE
#include <Overlay/OgreFontManager.h>
#else
#include <OgreFontManager.h>
#endif
#include <OgreParticleSystemManager.h>
#include "ogre/ManualMovableTextRenderer.h"
#include "WindowEventListener.h"

#ifdef OGRE_STATIC
#include <RenderSystems/GL/OgreGLPlugin.h>
#include <OgreBspSceneManagerPlugin.h>
#include <OgreOctreePlugin.h>
#include <OgrePCZPlugin.h>
#include <OgreParticleFXPlugin.h>
#endif


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

  // Render system identifier for the factory registration
  const std::string OgreRenderSystem::ID = "ogre";

  OgreRenderSystem::OgreRenderSystem() :
    mRoot(0),
    mFontManager(0),
    mManualMovableTextParticleFactory(0),
    mResourceManager(0),
    mViewport(0),
    mWindowEventListener(0),
    mSceneManager(0)
  {
    mSystemInfo.put("type", OgreRenderSystem::ID);
    mLogManager = new Ogre::LogManager();
  }

  OgreRenderSystem::~OgreRenderSystem()
  {
    mRenderTargets.clear();

    if(mFontManager != 0)
      delete mFontManager;

    if(getRenderWindow() != 0 && mWindowEventListener != 0)
      mWindowEventListener->windowClosed(getRenderWindow());
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
    std::string plugins;
    std::string config    = workdir + GSAGE_PATH_SEPARATOR + settings.get("configFile", "ogreConfig.cfg");
#ifndef OGRE_STATIC
    plugins = workdir + GSAGE_PATH_SEPARATOR + settings.get("pluginsFile", "plugins.cfg");
#endif

    // redirect ogre logs to custom logger
    mLogManager->createLog("", true, false, false);
    mLogManager->getDefaultLog()->addListener(&mLogRedirect);

    // initialize ogre root
    mRoot = new Ogre::Root(plugins, config, "");
    // initialize resource manager
    mResourceManager = new ResourceManager(workdir);

#ifdef OGRE_STATIC
    auto pair = settings.get<DataProxy>("plugins");
    if(pair.second) {
      for(auto p : pair.first) {
        std::string id = p.second.getValueOptional<std::string>("");
        LOG(INFO) << "Installing plugin " << id;

        if(id == "RenderSystem_GL") {
          mRoot->installPlugin(new Ogre::GLPlugin());
        } else if(id == "OctreeSceneManager") {
          mRoot->installPlugin(new Ogre::OctreePlugin());
        } else if(id == "ParticleFX") {
          mRoot->installPlugin(new Ogre::ParticleFXPlugin());
        } else if(id == "PCZSceneManager") {
          mRoot->installPlugin(new Ogre::PCZPlugin());
        } else if(id == "BSPSceneManager") {
          mRoot->installPlugin(new Ogre::BspSceneManagerPlugin());
        } else {
          LOG(ERROR) << "Can't install plugin " << id;
          return false;
        }
      }
    }
#endif
    auto p = settings.get<DataProxy>("window");
    if(!p.second) {
      LOG(ERROR) << "No primary window is defined for OGRE render system";
      return false;
    }

    DataProxy windowParams = p.first;

    std::string windowName = windowParams.get("name", "mainWindow");

    if(windowParams.get("useWindowManager", false)) {
      WindowPtr window = mFacade->getWindowManager()->createWindow(
        windowName,
        windowParams.get("width", 1024),
        windowParams.get("height", 786),
        windowParams.get("fullscreen", false),
        windowParams
      );
      if(window == nullptr) {
        return false;
      }

      windowParams.put("windowHandle", Ogre::StringConverter::toString(window->getWindowHandle()));
    }

    if(!(mRoot->restoreConfig()))
        return false;

    // initialize render window
    mRoot->initialise(false);

    mWindow = createRenderTarget(windowName, RenderTarget::Window, windowParams);

    EventSubscriber<OgreRenderSystem>::addEventListener(mEngine, WindowEvent::RESIZE, &OgreRenderSystem::handleWindowResized, 0);

    if(!settings.get("window.useWindowManager", false)) {
      mWindowEventListener = new WindowEventListener(getRenderWindow(), mEngine);
      Ogre::WindowEventUtilities::addWindowEventListener(getRenderWindow(), mWindowEventListener);
    }

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

    DataProxy rtt = settings.get("renderTargets", DataProxy());
    if(rtt.size() > 0) {
      for(auto pair : rtt) {
        const std::string name = pair.first;
        if(pair.second.count("type") == 0) {
          LOG(ERROR) << "Failed to create render target " << name << ": 'type' field is missing";
          continue;
        }

        createRenderTarget(name, pair.second.get<RenderTarget::Type>("type", RenderTarget::Rtt), pair.second);
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

    // finally initialize render targets defined in configs
    for(auto pair : mRenderTargets) {
      pair.second->initialize(mSceneManager);
    }

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

    mEngine->fireEvent(RenderEvent(RenderEvent::UPDATE, this));

    for(auto pair : mRenderTargets) {
      if(!pair.second->isAutoUpdated()) {
        pair.second->update();
      }
    }

    bool continueRendering = !getRenderWindow()->isClosed();
    if(continueRendering) {
      continueRendering = mRoot->renderOneFrame();
    }

    if(!continueRendering)
      mEngine->fireEvent(EngineEvent(EngineEvent::SHUTDOWN));
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
    if(!skipThisInvocation) {
      RenderTargetPtr target;
      if(mRenderTargets.count(invocation) != 0) {
        target = mRenderTargets[invocation];
      } else {
        target = mWindow;
      }
      mEngine->fireEvent(RenderEvent(RenderEvent::RENDER_QUEUE_STARTED, this, queueGroupId, target));
    }
  }

  void OgreRenderSystem::renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation)
  {
    if(!skipThisInvocation) {
      RenderTargetPtr target;
      if(mRenderTargets.count(invocation) != 0) {
        target = mRenderTargets[invocation];
      } else {
        target = mWindow;
      }
      mEngine->fireEvent(RenderEvent(RenderEvent::RENDER_QUEUE_ENDED, this, queueGroupId, target));
    }
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

  unsigned int OgreRenderSystem::getFBOID(const std::string& target) const
  {
    if(mRenderTargets.count(target) == 0) {
      LOG(WARNING) << "Attempt to get fboid of not existing render target: " << target;
      return 0;
    }

    if(mRenderTargets.at(target)->getType() != RenderTarget::Rtt) {
      LOG(WARNING) << "FBOID can be obtained only from Rtt target";
      return 0;
    }

    const RttRenderTarget* rtt = static_cast<const RttRenderTarget*>(mRenderTargets.at(target).get());
    return rtt->getGLID();
  }

  void OgreRenderSystem::setWidth(unsigned int width, const std::string& target)
  {
    if(target.empty()) {
      mWindow->setWidth(width);
      return;
    }

    if(mRenderTargets.count(target) == 0) {
      LOG(WARNING) << "Attempt to set width to not existing render target: " << target;
      return;
    }

    mRenderTargets[target]->setWidth(width);
  }

  unsigned int OgreRenderSystem::getWidth(const std::string& target) const
  {
    if(target.empty())
      return mWindow ? mWindow->getWidth() : 0;

    if(mRenderTargets.count(target) == 0) {
      LOG(WARNING) << "Attempt to get height of not existing render target: " << target;
      return 0;
    }

    return mRenderTargets.at(target)->getWidth();
  }

  void OgreRenderSystem::setHeight(unsigned int height, const std::string& target)
  {
    if(target.empty()) {
      mWindow->setHeight(height);
      return;
    }

    if(mRenderTargets.count(target) == 0) {
      LOG(WARNING) << "Attempt to set height to not existing render target: " << target;
      return;
    }

    mRenderTargets[target]->setHeight(height);
  }

  unsigned int OgreRenderSystem::getHeight(const std::string& target) const
  {
    if(target.empty())
      return mWindow ? mWindow->getHeight() : 0;

    if(mRenderTargets.count(target) == 0) {
      LOG(WARNING) << "Attempt to get height of not existing render target: " << target;
      return 0;
    }

    return mRenderTargets.at(target)->getHeight();
  }

  void OgreRenderSystem::setSize(unsigned int width, unsigned int height, const std::string& target)
  {
    if(mRenderTargets.count(target) == 0) {
      LOG(WARNING) << "Attempt to set size of not existing render target: " << target;
      return;
    }

    mRenderTargets[target]->setDimensions(width, height);
  }

  RenderTargetPtr OgreRenderSystem::createRenderTarget(const std::string& name, RenderTarget::Type type, DataProxy parameters) {
    parameters.put("name", name);
    mRenderTargets[name] = mRenderTargetFactory.create(name, type, parameters, mEngine);
    if(type == RenderTarget::Window) {
      std::string windowHandle = parameters.get("windowHandle", "");
      if(!windowHandle.empty()) {
        mRenderWindowsByHandle[windowHandle] = mRenderTargets[name];
      }
    }

    if(mSceneManager) {
      mRenderTargets[name]->initialize(mSceneManager);
    }
    return mRenderTargets[name];
  }

  void OgreRenderSystem::renderCameraToTarget(Ogre::Camera* cam, const std::string& target) {
    if(mRenderTargets.count(target) == 0) {
      LOG(ERROR) << "Can't render camera to target " << target << " no such RTT found";
      return;
    }

    mRenderTargets[target]->setCamera(cam);
  }

  void OgreRenderSystem::renderCameraToTarget(const std::string& cameraName, const std::string& target) {
    if(mRenderTargets.count(target) == 0) {
      return;
    }

    Ogre::Camera* cam = NULL;
    try {
      cam = mSceneManager->getCamera(cameraName);
    } catch(...) {
      LOG(WARNING) << "Camera with name " << cameraName << " does not exist";
    }

    if(cam == NULL) {
      LOG(INFO) << "Failed to make camera " << cameraName << " render to target " << target;
      return;
    }

    renderCameraToTarget(cam, target);
  }

  RenderTargetPtr OgreRenderSystem::getRenderTarget(const std::string& name)
  {
    if(mRenderTargets.count(name) == 0) {
      return nullptr;
    }

    return mRenderTargets[name];
  }

  RenderTargetPtr OgreRenderSystem::getMainRenderTarget()
  {
    return mWindow;
  }

  bool OgreRenderSystem::handleWindowResized(EventDispatcher* sender, const Event& e)
  {
    const WindowEvent& event = static_cast<const WindowEvent&>(e);
    mWindow->setDimensions(event.width, event.height);
    return true;
  }
}
