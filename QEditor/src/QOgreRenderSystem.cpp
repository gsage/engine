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

#include "QOgreRenderSystem.h"
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QThread>
#include "CameraFactory.h"

namespace Gsage {

  QOgreRenderSystem::QOgreRenderSystem()
    : mQQuickWindow(0)
  {
  }

  QOgreRenderSystem::~QOgreRenderSystem()
  {
    if(mOgreContext)
      delete mOgreContext;

    if(mRoot)
      mRoot->removeFrameListener(this);
  }

  QOpenGLContext* QOgreRenderSystem::getContext()
  {
    return mOgreContext;
  }

  void QOgreRenderSystem::setQtWindow(QQuickWindow* window)
  {
    if(mQQuickWindow)
      return;
    mQQuickWindow = window;

    mQtContext = QOpenGLContext::currentContext();

    mOgreContext = new QOpenGLContext();
    mOgreContext->setFormat(mQQuickWindow->requestedFormat());
    mOgreContext->setShareContext(mQtContext);
    mOgreContext->create();
  }

  bool QOgreRenderSystem::initialize(const DataProxy& settings)
  {
    activateOgreContext();
    bool res = OgreRenderSystem::initialize(settings);

    mSamples = (GLuint) settings.get<unsigned int>("window.params.FSAA", 0);
    mRoot->addFrameListener(this);

    deactivateOgreContext();
    return res;
  }

  bool QOgreRenderSystem::configure(const DataProxy& config)
  {
    activateOgreContext();
    bool res = OgreRenderSystem::configure(config);
    deactivateOgreContext();
    return res;
  }

  bool QOgreRenderSystem::fillComponentData(RenderComponent* component, const DataProxy& data)
  {
    activateOgreContext();
    bool res = OgreRenderSystem::fillComponentData(component, data);
    deactivateOgreContext();
    return res;
  }

  void QOgreRenderSystem::update(const double& time)
  {
    activateOgreContext();
    ComponentStorage<RenderComponent>::update(time);
    Ogre::WindowEventUtilities::messagePump();

    if(mCameraController != 0)
      mCameraController->update(time);

    mRoot->renderOneFrame();
    deactivateOgreContext();
  }

  bool QOgreRenderSystem::removeComponent(RenderComponent* component)
  {
    activateOgreContext();
    bool res = OgreRenderSystem::removeComponent(component);
    deactivateOgreContext();
    return res;
  }

  void QOgreRenderSystem::renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation)
  {
    OgreRenderSystem::renderQueueStarted(queueGroupId, invocation, skipThisInvocation);
  }

  bool QOgreRenderSystem::activateOgreContext()
  {
    if(!mQQuickWindow)
      return false;

    glPopAttrib();
    glPopClientAttrib();

    mQtContext->functions()->glUseProgram(0);
    mQtContext->doneCurrent();

    mOgreContext->makeCurrent(mQQuickWindow);
    return true;
  }

  bool QOgreRenderSystem::deactivateOgreContext()
  {
    if(!mQQuickWindow)
      return false;

    mOgreContext->functions()->glBindRenderbuffer(GL_RENDERBUFFER, 0);
    mOgreContext->functions()->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

    // unbind all possible remaining buffers; just to be on safe side
    mOgreContext->functions()->glBindBuffer(GL_ARRAY_BUFFER, 0);
#if GSAGE_PLATFORM != GSAGE_APPLE
    mOgreContext->functions()->glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
#endif
    mOgreContext->functions()->glBindBuffer(GL_COPY_READ_BUFFER, 0);
    mOgreContext->functions()->glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    mOgreContext->functions()->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    mOgreContext->functions()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    mOgreContext->functions()->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    mOgreContext->functions()->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    mOgreContext->functions()->glBindBuffer(GL_TEXTURE_BUFFER, 0);
    mOgreContext->functions()->glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
    mOgreContext->functions()->glBindBuffer(GL_UNIFORM_BUFFER, 0);

    mOgreContext->doneCurrent();

    mQtContext->makeCurrent(mQQuickWindow);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    return true;
  }

  Ogre::TexturePtr QOgreRenderSystem::getRtt(const std::string& name, float width, float height)
  {
    Ogre::TextureManager& texManager = Ogre::TextureManager::getSingleton();
    if(texManager.resourceExists(name))
      texManager.remove(name);

    mRttTexture = texManager.createManual(name,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D,
        width,
        height,
        0,
        Ogre::PF_R8G8B8A8,
        Ogre::TU_RENDERTARGET,
        0,
        false,
        mSamples);
    return mRttTexture;
  }

  bool QOgreRenderSystem::frameRenderingQueued(const Ogre::FrameEvent& fe)
  {
    return true;
  }

  void QOgreRenderSystem::flush()
  {
#if GSAGE_PLATFORM == GSAGE_APPLE
    glFlushRenderAPPLE();
#endif
  }
}
