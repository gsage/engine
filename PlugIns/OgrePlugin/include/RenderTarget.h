#ifndef _RenderTarget_H_
#define _RenderTarget_H_

/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Gsage Authors

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

#include <OgreRenderTarget.h>
#include "RenderTargetTypes.h"
#include "DataProxy.h"
#include "EventSubscriber.h"
#include "systems/RenderSystem.h"
#include "CollisionTools.h"
#include <OgreVector2.h>
#include "OgreConverters.h"

namespace Ogre {
  class SceneManager;
#if OGRE_VERSION_MAJOR == 2
  class CompositorWorkspace;
#endif
}

namespace Gsage {
  class Engine;
  class Entity;
  class RenderTarget : public EventSubscriber<RenderTarget>
  {
    public:
      RenderTarget(const std::string& name, Ogre::RenderTarget* wrapped, const DataProxy& parameters, Engine* engine);
      RenderTarget(const std::string& name, RenderTargetType::Type type, const DataProxy& parameters, Engine* engine);
      virtual ~RenderTarget();

      /**
       * Get RenderTarget name
       */
      const std::string& getName() const;

      /**
       * Get RenderTarget type
       */
      RenderTargetType::Type getType() const;

      /**
       * Initialize with scene manager
       *
       * @param sceneManager Ogre::SceneManager
       */
      void initialize(Ogre::SceneManager* sceneManager);

      /**
       * Get render target width
       */
      virtual int getWidth() const;

      /**
       * Get render target height
       */
      virtual int getHeight() const;

      /**
       * Set render target width
       * @param width
       */
      virtual void setWidth(int width);

      /**
       * Set render target height
       * @param height
       */
      virtual void setHeight(int height);

      /**
       * Set render target dimensions
       *
       * @param width
       * @param height
       */
      virtual void setDimensions(int width, int height);

      /**
       * Set render target position
       *
       * @param x
       * @param y
       */
      virtual void setPosition(int x, int y);

      /**
       * Set render target camera
       *
       * @param camera
       */
      virtual void setCamera(Ogre::Camera* camera);

      /**
       * Get camera
       */
      virtual Ogre::Camera* getCamera();

      /**
       * Get viewport
       */
      virtual Ogre::Viewport* getViewport();

      /**
       * Update render target
       */
      virtual void update();

      /**
       * Is render target auto updated
       */
      bool isAutoUpdated() const;

      /**
       * Get underlying Ogre::RenderTarget
       */
      Ogre::RenderTarget* getOgreRenderTarget();

      /**
       * Check if mouse over
       */
      bool isMouseOver() const;

      /**
       * Switches back to default camera
       */
      void switchToDefaultCamera();
#if OGRE_VERSION >= 0x020100
      void destroyCurrentWorkspace();
#endif
      virtual bool onTextureEvent(EventDispatcher* sender, const Event& event) { return true; }

      /**
       * Raycast and get object and 3d point under the pointer
       *
       * @param defaultDistance If nothing is hit, use ray and point on it
       * @param closestDistance Closest raycast distance
       * @param flags Objects flags filter
       *
       * @returns tuple: point and object hit by ray
       */
      std::tuple<Ogre::Vector3, Entity*> raycast(Ogre::Real defaultDistance, Ogre::Real closestDistance, unsigned int flags) const;
    private:
      void subscribe();

      bool handleRaycast(EventDispatcher* sender, const Event& event);

      bool handleMouseEvent(EventDispatcher* sender, const Event& event);

      std::tuple<Ogre::Ray, bool> getRay() const;

      void doRaycasting(float x, float y, unsigned int flags = 0xFFFF, bool select = false);
    protected:
      virtual void updateCameraAspectRatio();

#if OGRE_VERSION_MAJOR == 2
      virtual void configureWorkspace(Ogre::CompositorWorkspace* workspace);
#else
      virtual void configureViewport(Ogre::Viewport* viewport);
#endif

      Ogre::RenderTarget* mWrappedTarget;
      std::string renderQueueSequenceName;

      int mX;
      int mY;
      int mWidth;
      int mHeight;

      DataProxy mParameters;
      const std::string mName;
      Ogre::Camera* mCurrentCamera;
      Ogre::Camera* mDefaultCamera;

      bool mAutoUpdate;
      RenderTargetType::Type mType;

      bool mHasQueueSequence;
      bool mDestroying;
      int mSamples;

      Engine* mEngine;
      Ogre::SceneManager* mSceneManager;
      std::shared_ptr<MOC::CollisionTools> mCollisionTools;
      bool mContinuousRaycast;
      bool mMouseOver;

      Ogre::Vector2 mMousePosition;
      std::string mRolledOverObject;
#if OGRE_VERSION >= 0x020100
      Ogre::CompositorWorkspace* mWorkspace;
#else
      bool mRenderQueueSequenceCreated;
#endif
  };

  /**
   * Wraps Ogre RT target
   */
  class RttRenderTarget : public RenderTarget
  {
    public:
      RttRenderTarget(const std::string& name, const DataProxy& parameters, Engine* engine);
      virtual ~RttRenderTarget();

      /**
       * @copydoc RenderTarget::setDimensions
       */
      void setDimensions(int width, int height);

    protected:
      void createTexture(const std::string& name,
          unsigned int width,
          unsigned int height,
          unsigned int samples,
          Ogre::PixelFormat pixelFormat);

      TexturePtr mTexture;
      bool onTextureEvent(EventDispatcher* sender, const Event& event);
    private:
      void wrap();
  };

  /**
   * Wrapped Ogre::RenderWindow
   */
  class WindowRenderTarget : public RenderTarget
  {
    public:
      WindowRenderTarget(const std::string& name, const DataProxy& parameters, Engine* engine);
      virtual ~WindowRenderTarget();

      /**
       * @copydoc RenderTarget::setDimensions
       */
      void setDimensions(int width, int height);
  };

  typedef RenderTarget* RenderTargetPtr;

  /**
   * Factory creates Ogre::RenderTarget wrapped into Gsage wrapper
   */
  class RenderTargetFactory {
    public:
      /**
       * Create render target of specified type
       *
       * @param name render target name
       * @param type RenderTargetType:Rtt or RenderTargetType::Window
       * @param parameters additional parameters
       * @param engine Engine instance
       */
      RenderTargetPtr create(const std::string& name, RenderTargetType::Type type, const DataProxy& parameters, Engine* engine);

      /**
       * Wrap render target
       *
       * @param renderTarget Ogre::RenderTarget
       * @param name wrapped rt name
       * @param parameters additional parameters
       * @param engine Engine instance
       */
      RenderTargetPtr wrap(Ogre::RenderTarget* renderTarget, const std::string& name, const DataProxy& parameters, Engine* engine);
  };
}

#endif
