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

#ifndef _QOgreRenderSystem_H_
#define _QOgreRenderSystem_H_

#include <QOpenGLFunctions>

#include "systems/OgreRenderSystem.h"

class QQuickWindow;
class QOpenGLContext;

namespace Gsage {

  class QOgreRenderSystem : public OgreRenderSystem, public Ogre::FrameListener
  {
    public:
      QOgreRenderSystem();
      virtual ~QOgreRenderSystem();

      /**
       * Gets ogre context
       */
      QOpenGLContext* getContext();

      /**
       * Set qt window to use
       * @param window QQuickWindow to use for rendering
       */
      void setQtWindow(QQuickWindow* window);

      /**
       * Switches context to ogre, sets all necessary settings
       * @returns True if switch was successful
       */
      bool activateOgreContext();
      /**
       * Switches to qt context, resets buffers
       * @returns True if switch was successful
       */
      bool deactivateOgreContext();

      // Overriding all functions that work with Ogre to enable context switching

      bool initialize(const DataProxy& settings);

      bool configure(const DataProxy& config);

      bool fillComponentData(RenderComponent* component, const DataProxy& data);

      void update(const double& time);

      bool removeComponent(RenderComponent* component);

      void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);

      /**
       * Create render texture with projection of the main camera
       * @param name Texture id
       * @param width Texture width
       * @param height Texture height
       */
      Ogre::TexturePtr getRtt(const std::string& name, float width, float height);

      /**
       * Override listener for frameRenderingQueued
       */
      virtual bool frameRenderingQueued(const Ogre::FrameEvent& fe);

      void flush();
    protected:
      void createCamera() { updateCurrentCamera(mSceneManager->createCamera("main"));}
    private:
      QQuickWindow* mQQuickWindow;
      /** Pointer to QOpenGLContext to be used by Ogre. */
      QOpenGLContext* mOgreContext;
      /** Pointer to QOpenGLContext to be restored after Ogre context. */
      QOpenGLContext* mQtContext;

      Ogre::TexturePtr mRttTexture;

      GLuint mSamples;
  };
}

#endif
