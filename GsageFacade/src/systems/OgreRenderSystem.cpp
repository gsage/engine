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

#include "ogre/SceneNodeWrapper.h"
#include "ogre/EntityWrapper.h"
#include "ogre/LightWrapper.h"
#include "ogre/BillboardWrapper.h"
#include "ogre/ParticleSystemWrapper.h"
#include "ogre/CameraWrapper.h"

#include "RenderEvent.h"
#include "Logger.h"

#include "AnimationScheduler.h"

#include "CameraFactory.h"
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

  OgreRenderSystem::OgreRenderSystem() :
    mRoot(0),
    mFontManager(0),
    mManualMovableTextParticleFactory(0),
    mResourceManager(0),
    mCameraController(0),
    mCameraFactory(new CameraFactory()),
    mWindow(0),
    mViewport(0),
    mOgreInteractionManager(0)
  {
    mLogManager = new Ogre::LogManager();
  }

  OgreRenderSystem::~OgreRenderSystem()
  {
    if(mOgreInteractionManager)
      delete mOgreInteractionManager;
    delete mCameraFactory;
    if(mFontManager != 0)
      delete mFontManager;
    if(mRoot != 0)
      delete mRoot;
    if(mManualMovableTextParticleFactory != 0)
      delete mManualMovableTextParticleFactory;
    if(mResourceManager != 0)
      delete mResourceManager;

    delete mLogManager;
  }

  bool OgreRenderSystem::initialize(const Dictionary& settings)
  {
    std::string workdir   = mEngine->env().get("workdir", ".");
    std::string plugins   = workdir + GSAGE_PATH_SEPARATOR + settings.get("pluginsFile", ".");
    std::string config    = workdir + GSAGE_PATH_SEPARATOR + settings.get("configFile", ".");

    // redirect ogre logs to custom logger
    mLogManager->createLog("", true, false, false);
    mLogManager->getDefaultLog()->addListener(&mLogRedirect);

    LOG(INFO) << settings;

    // initialize ogre root
    mRoot = new Ogre::Root(plugins, config, "");
    // initialize resource manager
    mResourceManager = new ResourceManager(workdir);

    if(!(mRoot->restoreConfig()))
        return false;

    // initialize render window
    mRoot->initialise(false);

    Ogre::NameValuePairList params;

    auto paramNode = settings.get<Dictionary>("window.params");
    LOG(INFO) << "PARAMS=" << paramNode.first << " loaded:" << paramNode.second;
    if(paramNode.second)
    {
      for(auto& pair : paramNode.first)
      {
        params[pair.first.str()] = pair.second.as<std::string>();
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

    // initialize scene manager
    mSceneManager = mRoot->createSceneManager(settings.get("sceneManager", "OctreeSceneManager"));
    // TODO: make it customizable via config
    mSceneManager->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_MODULATIVE);
    mSceneManager->addRenderQueueListener(this);

    mFontManager = new Ogre::FontManager();
    // initializing custom factory for floating text particles
    mManualMovableTextParticleFactory = new Ogre::ManualMovableTextRendererFactory();
    Ogre::ParticleSystemManager::getSingletonPtr()->addRendererFactory(mManualMovableTextParticleFactory);

    auto resources = settings.get<Dictionary>("globalResources");
    if(resources.second && !mResourceManager->load(resources.first))
      return false;

    mRenderSystem = mRoot->getRenderSystem();

    assert(mRenderSystem != 0);

    // get viewport to initialize it
    getViewport();

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

  bool OgreRenderSystem::fillComponentData(RenderComponent* c, const Dictionary& dict)
  {
    if(!c->getResources().empty())
    {
      if(!mResourceManager->load(c->getResources()))
        return false;
    }

    auto element = dict.get<Dictionary>("root");
    if(element.second)
    {
      c->mRootNode = mObjectManager.create<SceneNodeWrapper>(element.first, c->getOwner()->getId(), mSceneManager);
    }
    element = dict.get<Dictionary>("animations");
    if(element.second)
    {
      LOG(INFO) << "Read animations for component of entity " << c->getOwner()->getId();
      c->mAnimationScheduler.initialize(element.first, mSceneManager);
    }
    c->mAddedToScene = true;

    return true;
  }

  void OgreRenderSystem::update(const double& time)
  {
    ComponentStorage<RenderComponent>::update(time);
    Ogre::WindowEventUtilities::messagePump();

    mOgreInteractionManager->update(time);
    if(mCameraController != 0)
      mCameraController->update(time);

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

  bool OgreRenderSystem::configure(const Dictionary& config)
  {

    auto resources = mConfig.get<Dictionary>("resources");
    if(resources.second)
      mResourceManager->unload(resources.first);

    EngineSystem::configure(config);

    resources = mConfig.get<Dictionary>("resources");
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

    if(config.count("camera") != 0)
    {
      mCameraController = mCameraFactory->initializeController(
          config.get<Dictionary>("camera").first,
          this,
          mEngine);
      updateCurrentCamera(mCameraController->getCamera());
    }
    return true;
  }

  Dictionary& OgreRenderSystem::getConfig()
  {
    if(mCameraController)
    {
      Dictionary camera;
      mCameraController->dump(camera);
      mConfig.put("camera", camera);
    }
    return mConfig;
  }

  void OgreRenderSystem::renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation)
  {
    if(queueGroupId == Ogre::RENDER_QUEUE_OVERLAY)
      mEngine->fireEvent(RenderEvent(RenderEvent::UPDATE_UI, mRenderSystem, mWindow));
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

  CameraController* OgreRenderSystem::getCamera()
  {
    return mCameraController;
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
}
