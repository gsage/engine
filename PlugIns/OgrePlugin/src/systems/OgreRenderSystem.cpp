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
#include "ogre/CameraWrapper.h"
#include "ogre/ParticleSystemWrapper.h"
#include "ogre/BillboardWrapper.h"
#if OGRE_VERSION_MAJOR == 1
#include "ogre/v1/ManualObjectWrapper.h"
#else
#include "ogre/v2/ManualObjectWrapper.h"
#include "ogre/v2/ItemWrapper.h"
#include <OgreBitwise.h>
#endif

#include "RenderEvent.h"
#include "EngineEvent.h"
#include "Logger.h"

#include "AnimationScheduler.h"
#include "MaterialLoader.h"

#include "ComponentStorage.h"
#include "Entity.h"
#include "Engine.h"
#include "GsageFacade.h"
#include "EngineEvent.h"
#include "WindowManager.h"

#if GSAGE_PLATFORM == GSAGE_APPLE
#include <Overlay/OgreFontManager.h>
#else
#include <OgreFontManager.h>
#endif
#include <OgreParticleSystemManager.h>
#include "ogre/ManualMovableTextRenderer.h"
#include "WindowEventListener.h"

#ifdef OGRE_STATIC
#if OGRE_VERSION < 0x020100
#include <RenderSystems/GL/OgreGLPlugin.h>
#include <OgreBspSceneManagerPlugin.h>
#include <OgreOctreePlugin.h>
#include <OgrePCZPlugin.h>
#else
#include <RenderSystems/GL3Plus/OgreGL3PlusPlugin.h>
#ifdef WITH_METAL
#include <RenderSystems/Metal/OgreMetalPlugin.h>
#endif
#endif
#include <OgreParticleFXPlugin.h>
#endif

#if OGRE_VERSION >= 0x020100
#include <Compositor/OgreCompositorManager2.h>
#endif

#include <chrono>

#include "OgreGeom.h"
#include "GsageDefinitions.h"


namespace Gsage {

  inline bool pluginExists(const std::string& name) {
    std::stringstream ss;
    ss << name;
#if GSAGE_PLATFORM == GSAGE_WIN32
    ss << ".dll";
#elif GSAGE_PLATFORM == GSAGE_APPLE
    ss << "";
#elif GSAGE_PLATFORM == GSAGE_LINUX
    ss << ".so";
#else
    LOG(ERROR) << "File existence check is not supported on the current platform";
    return false;
#endif
    std::ifstream f(ss.str());
    return f.good();
  }

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

  OgreRenderSystem::OgreRenderSystem()
    : mRoot(0)
    , mFontManager(0)
    , mManualMovableTextParticleFactory(0)
    , mResourceManager(0)
    , mViewport(0)
    , mWindowEventListener(0)
    , mSceneManager(0)
    , mObjectManager(this)
    , mManualTextureManager(this)
    , mLogManager(0)
    , mMaterialLoader(0)
  {
    mSystemInfo.put("type", OgreRenderSystem::ID);
    mSystemInfo.put("version", OGRE_VERSION);
    mSystemInfo.put("majorVersion", OGRE_VERSION_MAJOR);
  }

  OgreRenderSystem::~OgreRenderSystem()
  {
    shutdown();
  }

  bool OgreRenderSystem::initialize(const DataProxy& settings)
  {
    if(mLogManager == 0)
      mLogManager = new Ogre::LogManager();

    std::string workdir   = mEngine->env().get("workdir", ".");
    std::string config    = workdir + GSAGE_PATH_SEPARATOR + settings.get("configFile", "");

    // redirect ogre logs to custom logger
    mLogManager->createLog("", true, false, false);
    mLogManager->getDefaultLog()->addListener(&mLogRedirect);

    // initialize ogre root
    mRoot = new Ogre::Root("", config, "");
    // initialize custom material loader
    mMaterialLoader = new MaterialLoader(this, mFacade);
    // initialize resource manager
    mResourceManager = new ResourceManager(mFacade, mMaterialLoader);

    auto pair = settings.get<DataProxy>("plugins");
    if(pair.second) {
      for(auto p : pair.first) {
        std::string id = p.second.getValueOptional<std::string>("");
        if(!installPlugin(id) && !settings.get("ignoreFailedPlugins", true)) {
          return false;
        }
      }
    }

    auto p = settings.get<DataProxy>("window");
    if(!p.second) {
      LOG(ERROR) << "No primary window is defined for OGRE render system";
      return false;
    }

    DataProxy windowParams = p.first;

    std::string windowName = windowParams.get("name", "mainWindow");

    if(windowParams.get("useWindowManager", false)) {
      WindowPtr window;
      auto createWindow = [&]() {
        WindowManagerPtr wm = mFacade->getWindowManager();
        if(!wm) {
          return;
        }

        window = wm->getWindow(windowName);
        if(!window) {
          window = wm->createWindow(
            windowName,
            windowParams.get("width", 1024),
            windowParams.get("height", 786),
            windowParams.get("fullscreen", false),
            windowParams
          );
        } else {
          int width, height;
          std::tie(width, height) = window->getSize();
          windowParams.put("width", width);
          windowParams.put("height", height);
        }
      };

      if(settings.get("dedicatedThread", false)) {
        LOG(INFO) << "Creating window in the main thread";
        mEngine->executeInMainThread(createWindow)->wait();
      } else {
        createWindow();
      }
      if(window == nullptr) {
        LOG(ERROR) << "Failed to create window";
        return false;
      }

      windowParams.put("windowHandle", std::to_string(window->getWindowHandle()));
      if(windowParams.get("openGL", false)) {
        windowParams.put("glContext", Ogre::StringConverter::toString((size_t) window->getGLContext()));
      }
    }

    auto rs = settings.get<DataProxy>("renderSystems");
    if(rs.second) {
      Ogre::RenderSystem* renderSystem = nullptr;
      // iterate over defined render systems and try install one of them
      for(auto p : rs.first) {
        std::string id = p.second.get("id", "");
        renderSystem = mRoot->getRenderSystemByName(id);
        if(!renderSystem) {
          LOG(INFO) << "Can't create render system of type " << id << ", using fallback to the next one";
          continue;
        }
        try
        {
          for (auto pair : p.second)
          {
            if(pair.first == "id") {
              continue;
            }
            renderSystem->setConfigOption(pair.first, pair.second.as<std::string>());
          }
        }
        catch( Ogre::Exception &e )
        {
          LOG(ERROR) << e.getFullDescription();
          return false;
        }

        Ogre::String err = renderSystem->validateConfigOptions();
        if (err.length() > 0)
          return false;

        break;
      }
      if(!renderSystem) {
        LOG(ERROR) << "Failed to create render system using config: " << dumps(rs.second, DataWrapper::JSON_OBJECT);
        return false;
      }
      mRoot->setRenderSystem(renderSystem);
#if GSAGE_PLATFORM==GSAGE_APPLE && defined(WITH_METAL) && OGRE_VERSION >= 0x020100
      if(renderSystem->getFriendlyName() == "Metal_RS") {
        mManualTextureManager.setDefaultPixelFormat(Ogre::PF_X8R8G8B8);
      }
#endif
    } else {
      LOG(ERROR) << "No \"renderSystem\" configuration found";
      return false;
    }

    // initialize render window
    mRoot->initialise(false);

    mWindow = createRenderTarget(windowName, RenderTargetType::Window, windowParams);

    EventSubscriber<OgreRenderSystem>::addEventListener(mEngine, WindowEvent::RESIZE, &OgreRenderSystem::handleWindowResized, 0);

    if(!settings.get("window.useWindowManager", false)) {
      mWindowEventListener = new WindowEventListener(getRenderWindow(), mEngine);
      Ogre::WindowEventUtilities::addWindowEventListener(getRenderWindow(), mWindowEventListener);
    }

    std::string defaultSceneManager;
#if OGRE_VERSION_MAJOR == 1
    defaultSceneManager = "OctreeSceneManager";
#else
    defaultSceneManager = "DefaultSceneManager";
#endif

#if OGRE_VERSION >= 0x020100
    mCustomPassProvider.initialize(mEngine);

    auto compositorManager = mRoot->getCompositorManager2();
    compositorManager->setCompositorPassProvider(&mCustomPassProvider);
#endif

    // initialize scene manager
    mSceneManager = mRoot->createSceneManager(
        settings.get("sceneManager", defaultSceneManager)
#if OGRE_VERSION_MAJOR == 2
        , settings.get("numWorkerThreads", 1)
        , Ogre::INSTANCING_CULLING_SINGLETHREAD // TODO: allow configuring that
#endif
    );
#if OGRE_VERSION < 0x020100
    // TODO: make it customizable via config
    mSceneManager->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_MODULATIVE);
#else
    // same here
    mSceneManager->setShadowDirectionalLightExtrusionDistance(500.0f);
    mSceneManager->setShadowFarDistance(500.0f);
#endif

    mSceneManager->addRenderQueueListener(this);

    mFontManager = new Ogre::FontManager();
    // initializing custom factory for floating text particles
    mManualMovableTextParticleFactory = new Ogre::ManualMovableTextRendererFactory();
    Ogre::ParticleSystemManager::getSingletonPtr()->addRendererFactory(mManualMovableTextParticleFactory);

    auto resources = settings.get<DataProxy>("globalResources");
    if(resources.second && !mResourceManager->load(resources.first, true))
      return false;

    resources = mConfig.get<DataProxy>("resources");
    if(resources.second)
      mResourceManager->load(resources.first, false);

#if OGRE_VERSION >= 0x020100
    // TODO: make this configurable
    Ogre::Hlms *hlms = mRoot->getHlmsManager()->getHlms(Ogre::HLMS_PBS);
    Ogre::HlmsPbs *pbs = static_cast<Ogre::HlmsPbs*>(hlms);
    pbs->setShadowSettings(Ogre::HlmsPbs::PCF_4x4);

    // configure forward3D if it's defined
    auto forward3D = settings.get<DataProxy>("forward3D");

    if(forward3D.second) {
      mSceneManager->setForward3D(
        true,
        forward3D.first.get("width", 4),
        forward3D.first.get("height", 4),
        forward3D.first.get("numSlices", 5),
        forward3D.first.get("lightsPerCell", 96),
        forward3D.first.get("minDistance", 3.0f),
        forward3D.first.get("maxDistance", 200.0f)
      );
    } else {
      mSceneManager->setForward3D(
        false,
        4, 4, 5, 96, 3.0f, 200.f
      );
    }
    mSceneManager->getRenderQueue()->setRenderQueueMode(51, Ogre::RenderQueue::Modes::V1_FAST);
    //---------------------------------------------------------------------------------
#endif
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

        createRenderTarget(name, pair.second.get<RenderTargetType::Type>("type", RenderTargetType::Rtt), pair.second);
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
    mObjectManager.registerElement<ManualObjectWrapper>();
#if OGRE_VERSION >= 0x020100
    mObjectManager.registerElement<ItemWrapper>();
#endif

    mRoot->clearEventTimes();
    mRenderSystem->_initRenderTargets();

    // finally initialize render targets defined in configs
    for(auto pair : mRenderTargets) {
      pair.second->initialize(mSceneManager);
      mRenderTargetsReverseIndex[pair.second->getOgreRenderTarget()] = pair.first;
    }
    
    return EngineSystem::initialize(settings);
  }

  void OgreRenderSystem::shutdown()
  {
    if(!isReady())
    {
      return;
    }

    EngineSystem::shutdown();
    mManualTextureManager.reset();

    if(getRenderWindow() != 0 && mWindowEventListener != 0)
      mWindowEventListener->windowClosed(getRenderWindow());

    for(auto pair : mRenderTargets) {
      delete pair.second;
    }
    mRenderTargets.clear();
    if(mFontManager)
      delete mFontManager;

    mFontManager = 0;

    if(mResourceManager != 0) {
      delete mResourceManager;
      mResourceManager = 0;
    }

    if(mWindowEventListener != 0) {
      delete mWindowEventListener;
      mWindowEventListener = 0;
    }

    if(mRoot) {
      mRoot->shutdown();
      delete mRoot;
    }
    mRoot = 0;
    mSceneManager = 0;

    if(mManualMovableTextParticleFactory) {
      delete mManualMovableTextParticleFactory;
      mManualMovableTextParticleFactory = 0;
    }
#if OGRE_STATIC
    for(auto pair : mOgrePlugins) {
      delete pair.second;
    }
    mOgrePlugins.clear();
#endif

    if(mLogManager) {
      delete mLogManager;
      mLogManager = 0;
    }

    delete mMaterialLoader;
  }

  bool OgreRenderSystem::prepareComponent(OgreRenderComponent* c)
  {
    if(!mSceneManager) {
      return false;
    }
    c->prepare(mSceneManager, mResourceManager, &mObjectManager);
    return true;
  }

  bool OgreRenderSystem::fillComponentData(OgreRenderComponent* c, const DataProxy& dict)
  {
    c->mAddedToScene = true;
    return true;
  }

  void OgreRenderSystem::update(const double& time)
  {
    for(int i = 0; i < mMutationQueue.size(); ++i) {
      ObjectMutation om;
      mMutationQueue.get(om);
      om.execute();
    }

    ComponentStorage<OgreRenderComponent>::update(time);
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

    mManualTextureManager.updateDirtyTexures();

    if(!continueRendering)
      mEngine->fireEvent(EngineEvent(EngineEvent::SHUTDOWN));

    if(dedicatedThread()) {
      std::this_thread::sleep_for(std::chrono::microseconds((long)(6000 - time)));
    }
  }

  void OgreRenderSystem::updateComponent(OgreRenderComponent* component, Entity* entity, const double& time)
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
    Ogre::ColourValue ambientColour = config.get("colourAmbient", Ogre::ColourValue::Black);
#if OGRE_VERSION >= 0x020100
    mSceneManager->setAmbientLight(
        config.get("ambientLight.upperHemisphere", ambientColour),
        config.get("ambientLight.lowerHemisphere", ambientColour),
        config.get("ambientLight.hemisphereDir", Ogre::Vector3::UNIT_Y),
        config.get("ambientLight.envmapScale", 1.0f)
    );
#else
    mSceneManager->setAmbientLight(ambientColour);
#endif
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

  bool OgreRenderSystem::installPlugin(const std::string& id)
  {
    LOG(INFO) << "Installing plugin " << id;
#if OGRE_STATIC
    Ogre::Plugin* plugin = nullptr;
    if(id == "ParticleFX") {
      plugin = new Ogre::ParticleFXPlugin();
    }
#if OGRE_VERSION_MAJOR == 1
    else if(id == "RenderSystem_GL") {
      plugin = new Ogre::GLPlugin();
    } else if(id == "OctreeSceneManager") {
      plugin = new Ogre::OctreePlugin();
    } else if(id == "PCZSceneManager") {
      plugin = new Ogre::PCZPlugin();
    } else if(id == "BSPSceneManager") {
      plugin = new Ogre::BspSceneManagerPlugin();
    }
#else
    else if(id == "RenderSystem_GL" || id == "RenderSystem_GL3Plus") {
      plugin = new Ogre::GL3PlusPlugin();
    }
#ifdef WITH_METAL
    else if(id == "RenderSystem_Metal") {
      plugin = new Ogre::MetalPlugin();
    }
#endif
#endif

    if(plugin) {
      mRoot->installPlugin(plugin);
      mOgrePlugins[id] = plugin;
      return true;
    }
#endif
    std::map<std::string, std::string> pluginIDToFilename;
    pluginIDToFilename["OctreeSceneManager"] = "Plugin_OctreeSceneManager";
    pluginIDToFilename["PCZSceneManager"] = "Plugin_PCZSceneManager";
    pluginIDToFilename["BSPSceneManager"] = "Plugin_BSPSceneManager";
    pluginIDToFilename["ParticleFX"] = "Plugin_ParticleFX";

    std::string pluginsDir = mConfig.get("pluginsDir", "");
    if(!pluginsDir.empty()) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
      pluginsDir += "\\";
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
      pluginsDir += "/";
#endif
    }
    try {
      std::string pluginPath;
      std::string filename = pluginIDToFilename.count(id) != 0 ? pluginIDToFilename[id] : id;
#if GSAGE_PLATFORM ==  GSAGE_WIN32 && (!defined(NDEBUG) && !defined(__OPTIMIZE__) || defined(_DEBUG) && !defined(NDEBUG))
      pluginPath = pluginsDir + filename + "_d";
      if(!pluginExists(pluginPath)) {
        LOG(WARNING) << "Debug plugin " << pluginPath << " is not available";
        pluginPath = "";
      }
#endif
      if(pluginPath.empty()) {
        pluginPath = pluginsDir + filename;
      }

      if(!pluginExists(pluginPath)) {
        LOG(ERROR) << "Failed to load plugin " << pluginPath << ": file not found";
        return false;
      }

      LOG(INFO) << "Loading plugin " << pluginPath;
      mRoot->loadPlugin(pluginPath);
      return true;
    } catch(Ogre::Exception& e) {

      LOG(ERROR) << "Failed to load plugin " << id;
    }

    return false;
  }

  bool OgreRenderSystem::allowMultithreading()
  {
    return true;
  }

  void OgreRenderSystem::queueMutation(ObjectMutation::Callback callback)
  {
    mMutationQueue << ObjectMutation(callback);
  }

  GeomPtr OgreRenderSystem::getGeometry(const BoundingBox& bounds, int flags)
  {
    OgreEntities entities = getEntities(flags, bounds);
    return getGeometry(entities);
  }

  GeomPtr OgreRenderSystem::getGeometry(std::vector<std::string> entities)
  {
    std::map<std::string, bool> ids;
    for(auto id : entities) {
      ids[id] = true;
    }

    OgreEntities ogreEntities = getEntities();
    OgreEntities toProcess;
    for(auto e : ogreEntities) {
      Ogre::Any entityId;
      entityId = e->getUserObjectBindings().getUserAny("entity");
      if(entityId.isEmpty() || ids.count(entityId.get<std::string>()) == 0)
        continue;

      toProcess.push_back(e);
    }

    return getGeometry(entities);
  }

  GeomPtr OgreRenderSystem::getGeometry(OgreEntities entities) {
#if OGRE_VERSION >= 0x020100
    mSceneManager->updateSceneGraph();
#endif

    Ogre::SceneNode* root = mSceneManager->getRootSceneNode();
    if(!root) {
      LOG(INFO) << "Failed to get geometry: no root node defined";
      return nullptr;
    }

    GeomPtr geom = GeomPtr(new OgreGeom(entities, root));
    Ogre::Vector3 min(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
    Ogre::Vector3 max = -min;
#if OGRE_VERSION_MAJOR == 1
    Ogre::AxisAlignedBox meshBoundingBox;
#else
    Ogre::Aabb meshBoundingBox(min, max);
#endif


    // calculate bounds
    for(auto entity : entities) {
#if OGRE_VERSION_MAJOR == 1
      Ogre::Matrix4 transform = root->_getFullTransform().inverse() * entity->getParentSceneNode()->_getFullTransform();
      Ogre::AxisAlignedBox bbox = entity->getBoundingBox();
      bbox.transform(transform);
      meshBoundingBox.merge(bbox);
#else
      meshBoundingBox.merge(entity->getWorldAabb());
#endif
    }

    if(entities.size() == 0) {
      min = max = Ogre::Vector3::ZERO;
    } else {
      min = meshBoundingBox.getMinimum();
      max = meshBoundingBox.getMaximum();
    }

    geom->bmin = new float[3]{min.x, min.y, min.z};
    geom->bmax = new float[3]{max.x, max.y, max.z};

    return geom;
  }

  TexturePtr OgreRenderSystem::createTexture(RenderSystem::TextureHandle handle, const DataProxy& parameters)
  {
    return mManualTextureManager.createTexture(handle, parameters);
  }

  TexturePtr OgreRenderSystem::getTexture(RenderSystem::TextureHandle handle)
  {
    return mManualTextureManager.getTexture(handle);
  }

  bool OgreRenderSystem::deleteTexture(RenderSystem::TextureHandle handle)
  {
    return mManualTextureManager.deleteTexture(handle);
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

  bool OgreRenderSystem::removeComponent(OgreRenderComponent* component)
  {
    LOG(INFO) << "Remove component " << component->getOwner()->getId();
    if(component->mRootNode)
    {
      component->mRootNode->destroy();
      component->mRootNode = 0;
    }
    component->mAddedToScene = false;

    mResourceManager->unload(component->getResources());
    ComponentStorage<OgreRenderComponent>::removeComponent(component);
    return true;
  }

  OgreRenderSystem::OgreEntities OgreRenderSystem::getEntities(const unsigned int& query)
  {
    OgreRenderSystem::OgreEntities res;
    Ogre::SceneManager::MovableObjectIterator iterator = mSceneManager->getMovableObjectIterator("Entity");
    while(iterator.hasMoreElements())
    {
      OgreV1::Entity* e = static_cast<OgreV1::Entity*>(iterator.getNext());
      if((e->getQueryFlags() | query) == query)
        res.push_back(e);
    }
    return res;
  }

  OgreRenderSystem::OgreEntities OgreRenderSystem::getEntities(const unsigned int& query, const BoundingBox& bounds)
  {
    if(bounds.extent != BoundingBox::EXTENT_FINITE) {
      return getEntities(query);
    }

    Ogre::AxisAlignedBox bbox = BoundingBoxToAxisAlignedBox(bounds);
    OgreRenderSystem::OgreEntities res;
    Ogre::SceneManager::MovableObjectIterator iterator = mSceneManager->getMovableObjectIterator("Entity");
    while(iterator.hasMoreElements())
    {
      OgreV1::Entity* e = static_cast<OgreV1::Entity*>(iterator.getNext());
#if OGRE_VERSION_MAJOR == 1
      if(!e->getBoundingBox().intersects(bbox)) {
        continue;
      }
#else
      if(!e->getWorldAabb().intersects(Ogre::Aabb::newFromExtents(bbox.getMinimum(), bbox.getMaximum()))) {
        continue;
      }
#endif
      if((e->getQueryFlags() | query) == query)
        res.push_back(e);
    }
    return res;
  }

  OgreRenderSystem::Entities OgreRenderSystem::getObjectsInRadius(const Ogre::Vector3& center, float distance, const unsigned int flags, const std::string& id)
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

  RenderTargetPtr OgreRenderSystem::createRenderTarget(const std::string& name, RenderTargetType::Type type, DataProxy parameters) {
    parameters.put("name", name);
    mRenderTargets[name] = mRenderTargetFactory.create(name, type, parameters, mEngine);
    if(type == RenderTargetType::Window) {
      std::string windowHandle = parameters.get("windowHandle", "");
      if(!windowHandle.empty()) {
        mRenderWindowsByHandle[windowHandle] = mRenderTargets[name];
      }
    }

    if(mSceneManager) {
      mRenderTargets[name]->initialize(mSceneManager);
      mRenderTargetsReverseIndex[mRenderTargets[name]->getOgreRenderTarget()] = name;
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
#if OGRE_VERSION_MAJOR == 1
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
#endif
  }

  RenderTargetPtr OgreRenderSystem::getRenderTarget(const std::string& name)
  {
    if(mRenderTargets.count(name) == 0) {
      return nullptr;
    }

    return mRenderTargets[name];
  }

  RenderTargetPtr OgreRenderSystem::getRenderTarget(Ogre::RenderTarget* target)
  {
    if(mRenderTargetsReverseIndex.count(target) == 0) {
      return nullptr;
    }

    return mRenderTargets[mRenderTargetsReverseIndex[target]];
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
