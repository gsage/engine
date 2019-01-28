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

#ifndef _OgreRenderSystem_H_
#define _OgreRenderSystem_H_

#include <OgreRoot.h>
#include <OgreRenderWindow.h>
#include <OgreEntity.h>
#include <OgreConfigFile.h>
#include <OgreSceneManager.h>
#include <OgreRenderSystem.h>
#include <OgreWindowEventUtilities.h>
#include <OgreStringConverter.h>
#include <OgreLog.h>
#include <OgreRenderQueueListener.h>

#include "ComponentStorage.h"
#include "systems/RenderSystem.h"
#include "RenderTarget.h"
#include "RenderTargetTypes.h"
#include "ResourceManager.h"
#include "components/OgreRenderComponent.h"
#include "ogre/OgreObjectManager.h"
#include "EventDispatcher.h"
#include "ThreadSafeQueue.h"
#include "ObjectMutation.h"
#include "ManualTextureManager.h"

#include "Definitions.h"
#if OGRE_VERSION >= 0x020100
#include <OgreRectangle2D.h>
#include "ogre/v2/CustomPassProvider.h"
#endif

static const std::string OGRE_SECTION        = "OgreRenderer";
static const std::string OGRE_PLUGINS_PATH   = "PluginsPath";
static const std::string OGRE_CONFIG_PATH    = "ConfigPath";
static const std::string OGRE_RESOURCES      = "Resources";


namespace Ogre
{
  class FontManager;
  class ManualMovableTextRendererFactory;
  class Viewport;
  class RenderWindow;
  class SceneManager;
}

namespace Gsage
{
  class AnimationScheduler;
  class Entity;
  class Engine;
  class EngineEvent;
  class OgreInteractionManager;
  class WindowEventListener;

  /**
   * Class, used to redirect all ogre output to the easylogging++
   */
  class OgreLogRedirect : public Ogre::LogListener
  {
     void messageLogged( const std::string& message,
              Ogre::LogMessageLevel lml,
              bool maskDebug,
              const std::string& system,
              bool& skipMessage
              );
  };

  class GSAGE_OGRE_PLUGIN_API OgreRenderSystem : public ComponentStorage<OgreRenderComponent>, public Ogre::RenderQueueListener, public EventDispatcher, public EventSubscriber<OgreRenderSystem>, public RenderSystem
  {
    public:
      static const std::string ID;

      OgreRenderSystem();
      virtual ~OgreRenderSystem();

      // --------------------------------------------------------------------------------
      // Gsage::ComponentStorage implementation
      // --------------------------------------------------------------------------------

      /**
       * Intializes ogre render system
       * @param settings DataProxy with initial settings for the render system
       */
      virtual bool initialize(const DataProxy& settings);

      /**
       * Shutdown OGRE
       */
      virtual void shutdown();

      /**
       * Initializes OgreRenderComponent, injects objectManager, resourceManager and sceneManager
       *
       * @param c OgreRenderComponent component to initialize
       */
      virtual bool prepareComponent(OgreRenderComponent* c);
      /**
       * Finalizes setup
       *
       * @param component OgreRenderComponent component to initialize
       * @param data DataProxy with node settings
       * @returns false if failed to initialize component for some reason
       */
      virtual bool fillComponentData(OgreRenderComponent* component, const DataProxy& data);

      /**
       * Update render system
       * @param time Elapsed time
       */
      virtual void update(const double& time);

      /**
       * Update OgreRenderComponent
       *
       * @param component OgreRenderComponent to update
       * @param entity Entity that owns that component
       * @param time Elapsed time
       */
      virtual void updateComponent(OgreRenderComponent* component, Entity* entity, const double& time);

      /**
       * Remove render component from the system. Removes all related visual nodes
       *
       * @param component OgreRenderComponent component to remove
       */
      virtual bool removeComponent(OgreRenderComponent* component);

      /**
       * Reconfigure render system
       *
       * @param config DataProxy with configs
       */
      virtual bool configure(const DataProxy& config);

      /**
       * Get current configs of the system
       */
      DataProxy& getConfig();

      // --------------------------------------------------------------------------------
      // RenderSystem implementation
      // --------------------------------------------------------------------------------

      /**
       * Get implementation independent Geometry information
       *
       * @param bounds get entities in bounds
       * @param flags filter entities by flags (default is all)
       *
       * @returns GeomPtr
       */
      GeomPtr getGeometry(const BoundingBox& bounds, int flags = 0xFF);

      /**
       * Get implementation independent Geometry information
       *
       * @param entities filter entities by names
       * @returns GeomPtr
       */
      GeomPtr getGeometry(std::vector<std::string> entities);

      /**
       * Create texture manually
       *
       * @param handle texture id
       * @param parameters variable parameters that can be used for texture creation
       */
      virtual TexturePtr createTexture(RenderSystem::TextureHandle handle, const DataProxy& parameters);

      /**
       * Get texture by name
       *
       * @param handle texture id
       */
      virtual TexturePtr getTexture(RenderSystem::TextureHandle handle);

      /**
       * Delete texture by handle
       *
       * @param handle texture id
       * @returns true if succeed
       */
      virtual bool deleteTexture(RenderSystem::TextureHandle handle);

      // --------------------------------------------------------------------------------
      // Ogre::RenderQueueListener implementation
      // --------------------------------------------------------------------------------

      /**
       * Callback for ui updating 1.x, 2.0
       */
      virtual void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);

      /**
       * Render queue event pass through
       */
      virtual void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);

      // --------------------------------------------------------------------------------
      /**
       * Register new type of factory in the render system
       */
      template<typename T>
      void registerElement() { mObjectManager.registerElement<T>(); }

      /**
       * Unregister factory type
       */
      template<typename T>
      bool unregisterElement() { return mObjectManager.unregisterElement<T>(); }

#if OGRE_VERSION >= 0x020100
      /**
       * Register pass factory
       */
      template<class C>
      bool registerPassDef(Ogre::IdString id) { return mCustomPassProvider.registerPassDef<C>(id); }
#endif


      /**
       * Get ogre render window
       */
      Ogre::RenderWindow* getRenderWindow() { return static_cast<Ogre::RenderWindow*>(mWindow->getOgreRenderTarget()); }

      /**
       * Get ogre scene manager
       */
      Ogre::SceneManager* getSceneManager() {return mSceneManager; }

      /**
       * Get ogre render system
       */
      Ogre::RenderSystem* getRenderSystem() {return mRenderSystem;}

      typedef std::vector<Entity*> Entities;
      /**
       * Enumerable of Ogre::Entity
       */
      typedef std::vector<OgreV1::Entity*> OgreEntities;
      /**
       * Gets all scene entities
       *
       * @param query Filter entities by some query, default is all
       * @param bounds filter entities which are intersected by bounds
       */
      OgreEntities getEntities(const unsigned int& query, const BoundingBox& bounds);

      /**
       * Gets all scene entities
       *
       * @param query Filter entities by some query, default is all
       */
      OgreEntities getEntities(const unsigned int& query = 0xFF);

      /**
       * Get objects in radius
       * @param position Point to search around
       * @param distance Radius of the sphere query
       * @returns list of entities
       */
      Entities getObjectsInRadius(const Ogre::Vector3& center, float distance, const unsigned int flags = 0xFF, const std::string& id = "");

      virtual unsigned int getFBOID(const std::string& target = "") const;

      /**
       * Set render target width
       *
       * @param target Render target name
       */
      virtual void setWidth(unsigned int width, const std::string& target = "");

      /**
       * Get render window width
       */
      virtual unsigned int getWidth(const std::string& target = "") const;

      /**
       * Set render target height
       *
       * @param target Render target name
       */
      virtual void setHeight(unsigned int height, const std::string& target = "");

      /**
       * Get render window height
       */
      virtual unsigned int getHeight(const std::string& target = "") const;

      /**
       * Set size of render target
       *
       * @param target Render target name
       */
      virtual void setSize(unsigned int width, unsigned int height, const std::string& target = "");

      /**
       * Render camera to texture with name
       *
       * @param cameraName Name of the camera to use
       * @param target Name of RTT to render to
       */
      void renderCameraToTarget(const std::string& cameraName, const std::string& target);

      /**
       * Render camera to texture with name
       *
       * @param cam Camera pointer
       * @param target Name of RTT to render to
       */
      void renderCameraToTarget(Ogre::Camera* cam, const std::string& target);

      /**
       * Create new render target
       *
       * @param name Render target name
       * @param type Render target type
       * @param parameters Additional parameters
       */
      RenderTargetPtr createRenderTarget(const std::string& name, RenderTargetType::Type type, DataProxy parameters);

      /**
       * Get render target by name
       *
       * @param name Render target name
       */
      RenderTargetPtr getRenderTarget(const std::string& name);

      /**
       * Get render target by wrapped Ogre::RenderTarget*
       *
       * @param target Ogre::RenderTarget
       */
      RenderTargetPtr getRenderTarget(Ogre::RenderTarget* target);

      /**
       * Get main render target (window)
       */
      RenderTargetPtr getMainRenderTarget();

#if OGRE_VERSION >= 0x020100
      /**
       * Register new Hlms
       */
      template<class T>
      void registerHlms(const std::string& resourcePath)
      {
        mResourceManager->registerHlms<T>(resourcePath);
      }
#endif
      /**
       * Allow multithreaded mode for OgreRenderSystem
       */
      bool allowMultithreading();

      /**
       * Queue object mutation is called from Ogre wrappers
       *
       * @param callback Mutation callback
       */
      void queueMutation(ObjectMutation::Callback callback);
    protected:
      /**
       * Handle window resizing
       * @param sender Engine
       * @param event WindowEvent
       */
      bool handleWindowResized(EventDispatcher* sender, const Event& event);

      /**
       * Handle settings update
       * @param sender Engine
       * @param event EngineEvent
       */
      bool handleEnvUpdate(EventDispatcher* sender, const Event& event);

      bool installPlugin(const std::string& name);

      GeomPtr getGeometry(OgreEntities entities);

      void removeAllRenderTargets();

      Ogre::Root* mRoot;
      Ogre::SceneManager* mSceneManager;
      Ogre::RenderSystem* mRenderSystem;
      Ogre::LogManager* mLogManager;
      Ogre::FontManager* mFontManager;
      Ogre::ManualMovableTextRendererFactory* mManualMovableTextParticleFactory;
      Ogre::Viewport* mViewport;

      OgreLogRedirect mLogRedirect;
      OgreObjectManager mObjectManager;
      WindowEventListener* mWindowEventListener;

      ResourceManager* mResourceManager;

      typedef std::queue<DataProxy> ComponentLoadQueue;
      ComponentLoadQueue mLoadQueue;

      typedef std::map<std::string, RenderTargetPtr> RenderTargets;
      RenderTargets mRenderTargets;

      typedef std::map<Ogre::RenderTarget*, std::string> RenderTargetReverseIndex;
      RenderTargetReverseIndex mRenderTargetsReverseIndex;

      RenderTargetFactory mRenderTargetFactory;

      typedef std::map<std::string, RenderTargetPtr> RenderWindowsByHandle;
      RenderWindowsByHandle mRenderWindowsByHandle;

      RenderTargetPtr mWindow;
#if OGRE_VERSION >= 0x020100
      OgreV1::Rectangle2DFactory* mRectangle2DFactory;

      CustomPassProvider mCustomPassProvider;
#endif
#if OGRE_STATIC
      typedef std::map<std::string, Ogre::Plugin*> OgrePlugins;
      OgrePlugins mOgrePlugins;
#endif
      ThreadSafeQueue<ObjectMutation> mMutationQueue;

      ManualTextureManager mManualTextureManager;
  };
}

#endif
