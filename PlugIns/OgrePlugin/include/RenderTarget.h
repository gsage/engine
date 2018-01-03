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
#include "DataProxy.h"
#include "EventSubscriber.h"
#include "CollisionTools.h"
#include <OgreVector2.h>

namespace Ogre {
  class SceneManager;
}

namespace Gsage {
  class Engine;
  class RenderTarget : public EventSubscriber<RenderTarget>
  {
    public:
      /**
       * Represents different kinds of underlying wrapped target
       */
      enum Type {
        // Generic can contain any Ogre::RenderTarget
        Generic = 0,
        // Rtt wraps render to texture target
        Rtt     = 1,
        // Window wraps Ogre::RenderWindow
        Window  = 2
      };

      RenderTarget(const std::string& name, Ogre::RenderTarget* wrapped, const DataProxy& parameters, Engine* engine);
      RenderTarget(const std::string& name, Type type, const DataProxy& parameters, Engine* engine);
      virtual ~RenderTarget();

      /**
       * Get RenderTarget name
       */
      const std::string& getName() const;

      /**
       * Get RenderTarget type
       */
      Type getType() const;

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
    private:
      void subscribe();

      bool handleRaycast(EventDispatcher* sender, const Event& event);

      bool handleMouseEvent(EventDispatcher* sender, const Event& event);

      void doRaycasting(const float& x, const float& y, unsigned int flags = 0xFFFF, bool select = false);
    protected:
      virtual void updateCameraAspectRatio();

      virtual void configureViewport(Ogre::Viewport* viewport);

      Ogre::RenderTarget* mWrappedTarget;
      std::string renderQueueSequenceName;

      int mX;
      int mY;
      int mWidth;
      int mHeight;

      DataProxy mParameters;
      const std::string mName;
      Ogre::Camera* mCurrentCamera;

      bool mAutoUpdate;
      Type mType;
      bool mRenderQueueSequenceCreated;
      bool mHasQueueSequence;
      int mSamples;

      Engine* mEngine;
      Ogre::SceneManager* mSceneManager;
      std::shared_ptr<MOC::CollisionTools> mCollisionTools;
      bool mContinuousRaycast;
      bool mMouseOver;

      Ogre::Vector2 mMousePosition;
      std::string mRolledOverObject;
  };

  /**
   * Wrap render to texture target
   */
  class RttRenderTarget : public RenderTarget
  {
    public:
      RttRenderTarget(const std::string& name, const DataProxy& parameters, Engine* engine);
      virtual ~RttRenderTarget();

      /**
       * Get GL id of the underlying texture
       */
      unsigned int getGLID() const;

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

      Ogre::TexturePtr mTexture;
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

  typedef std::shared_ptr<RenderTarget> RenderTargetPtr;

  /**
   * Factory creates Ogre::RenderTarget wrapped into Gsage wrapper
   */
  class RenderTargetFactory {
    public:
      /**
       * Create render target of specified type
       *
       * @param name render target name
       * @param type RenderTarget:Rtt or RenderTarget::Window
       * @param parameters additional parameters
       * @param engine Engine instance
       */
      RenderTargetPtr create(const std::string& name, RenderTarget::Type type, const DataProxy& parameters, Engine* engine);

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
