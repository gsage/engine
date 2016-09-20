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
#include "components/RenderComponent.h"
#include "ogre/OgreObjectManager.h"
#include "EventDispatcher.h"
#include "OgreInteractionManager.h"

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
  class CameraFactory;
  class CameraController;
  class ResourceManager;
  class Entity;
  class Engine;
  class EngineEvent;
  class OgreInteractionManager;

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

  class OgreRenderSystem : public ComponentStorage<RenderComponent>, public Ogre::RenderQueueListener, public EventDispatcher
  {
    public:
      static const std::string CAMERA_CHANGED;

      OgreRenderSystem();
      virtual ~OgreRenderSystem();

      // --------------------------------------------------------------------------------
      // Gsage::ComponentStorage implementation
      // --------------------------------------------------------------------------------

      /**
       * Intializes ogre render system
       * @param settings Dictionary with initial settings for the render system
       */
      virtual bool initialize(const Dictionary& settings);

      /**
       * Initializes RenderComponent: creates ogre node, lights and etc.
       *
       * @param component RenderComponent component to initialize
       * @param data Dictionary with node settings
       * @returns false if failed to initialize component for some reason
       */
      virtual bool fillComponentData(RenderComponent* component, const Dictionary& data);

      /**
       * Update render system
       * @param time Elapsed time
       */
      virtual void update(const double& time);

      /**
       * Update RenderComponent
       *
       * @param component RenderComponent to update
       * @param entity Entity that owns that component
       * @param time Elapsed time
       */
      virtual void updateComponent(RenderComponent* component, Entity* entity, const double& time);

      /**
       * Remove render component from the system. Removes all related visual nodes
       *
       * @param component RenderComponent component to remove
       */
      virtual bool removeComponent(RenderComponent* component);

      /**
       * Reconfigure render system
       *
       * @param config Dictionary with configs
       */
      virtual bool configure(const Dictionary& config);

      /**
       * Get current configs of the system
       */
      Dictionary& getConfig();

      /**
       * Get render window width
       */
      unsigned int getWindowWidth() { return mWindow ? mWindow->getWidth() : 0; }

      /**
       * Get render window height
       */
      unsigned int getWindowHeight() { return mWindow ? mWindow->getHeight() : 0; };

      // --------------------------------------------------------------------------------
      // Ogre::RenderQueueListener implementation
      // --------------------------------------------------------------------------------

      /**
       * Callback for ui updating
       */
      virtual void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);

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

      /**
       * Get ogre render window
       */
      Ogre::RenderWindow* getRenderWindow() { return mWindow; }

      /**
       * Get ogre scene manager
       */
      Ogre::SceneManager* getSceneManager() {return mSceneManager; }

      typedef std::vector<Entity*> Entities;
      /**
       * Enumerable of Ogre::Entity
       */
      typedef std::vector<Ogre::Entity*> OgreEntities;
      /**
       * Gets all scene entities
       *
       * @param query Filter entities by some query, default is all
       */
      OgreEntities getEntities(const unsigned int& query = 0xFF);
      /**
       * Get currently active camera controller
       */
      CameraController* getCamera();
      /**
       * Get currently active ogre camera
       */
      Ogre::Camera* getOgreCamera();
      /**
       * Get objects in radius
       * @param position Point to search around
       * @param distance Radius of the sphere query
       * @returns list of entities
       */
      Entities getObjectsInRadius(const Ogre::Vector3& center, const float& distance, const unsigned int flags = 0xFF, const std::string& id = "");

      /**
       * Get main viewport
       */
      Ogre::Viewport* getViewport();

    protected:
      /**
       * Update current camera
       * @param camera New camera instance
       */
      void updateCurrentCamera(Ogre::Camera* camera);

      Ogre::Root* mRoot;
      Ogre::SceneManager* mSceneManager;
      Ogre::RenderSystem* mRenderSystem;
      Ogre::RenderWindow* mWindow;
      Ogre::Camera* mCurrentCamera;
      Ogre::LogManager* mLogManager;
      Ogre::FontManager* mFontManager;
      Ogre::ManualMovableTextRendererFactory* mManualMovableTextParticleFactory;
      Ogre::Viewport* mViewport;

      CameraController* mCameraController;

      OgreLogRedirect mLogRedirect;
      CameraFactory* mCameraFactory;
      OgreObjectManager mObjectManager;
      OgreInteractionManager* mOgreInteractionManager;

      ResourceManager* mResourceManager;

      typedef std::queue<Dictionary> ComponentLoadQueue;
      ComponentLoadQueue mLoadQueue;
  };
}

#endif
