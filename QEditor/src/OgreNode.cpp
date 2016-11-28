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

#include <RenderSystems/GL/OgreGLTexture.h>
#include <RenderSystems/GL/OgreGLFrameBufferObject.h>
#include <RenderSystems/GL/OgreGLFBORenderTexture.h>

#include "OgreNode.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include "CameraFactory.h"
#include "Engine.h"
#include "QOgreRenderSystem.h"

namespace Gsage {

  OgreNode::OgreNode(QQuickWindow* window)
    : QSGGeometryNode()
    , mGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4)
    , mTexture(0)
    , mRenderSystem(0)
    , mRenderTarget(0)
    , mViewport(0)
    , mWindow(0)
    , mOgreFboId(0)
    , mDirtyFBO(false)
    , mQuickWindow(window)
  {
    setMaterial(&mMaterial);
    setOpaqueMaterial(&mMaterialO);
    setGeometry(&mGeometry);
    setFlag(UsePreprocess);
  }

  OgreNode::~OgreNode()
  {
    Ogre::Root* root = Ogre::Root::getSingletonPtr();
    if(!root)
      return;

    if (mRenderTarget) 
    {
      mRenderTarget->removeAllViewports();
    }

    Ogre::Root::getSingletonPtr()->detachRenderTarget(mRenderTarget);
  }

  void OgreNode::setEngine(Engine* engine)
  {
    mRenderSystem = engine->getSystem<QOgreRenderSystem>();
    if(!mRenderSystem)
      return;
    mRenderSystem->setQtWindow(mQuickWindow);
    addEventListener(mRenderSystem, QOgreRenderSystem::CAMERA_CHANGED, &OgreNode::onCameraChanged);
  }

  void OgreNode::doneOgreContext()
  {
    if (mOgreFboId != 0)
    {
      Ogre::GLFrameBufferObject *ogreFbo = NULL;
      mRenderTarget->getCustomAttribute("FBO", &ogreFbo);
      Ogre::GLFBOManager *manager = ogreFbo->getManager();
      mRenderSystem->flush();
      manager->unbind(mRenderTarget);
    }
    mRenderSystem->deactivateOgreContext();
  }

  void OgreNode::activateOgreContext()
  {
    mRenderSystem->activateOgreContext();
    mRenderSystem->getContext()->functions()->glBindFramebuffer(GL_FRAMEBUFFER_EXT, mOgreFboId);
  }

  GLuint OgreNode::getOgreFboId()
  {
    if (!mRenderTarget)
      return 0;

    Ogre::GLFrameBufferObject *ogreFbo = 0;
    mRenderTarget->getCustomAttribute("FBO", &ogreFbo);
    Ogre::GLFBOManager *manager = ogreFbo->getManager();
    manager->bind(mRenderTarget);

    GLint id;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &id);

    return id;
  }

  void OgreNode::preprocess()
  {
    activateOgreContext();
    mRenderTarget->update(true);
    doneOgreContext();
  }

  void OgreNode::update()
  {
    if (mDirtyFBO) {
      activateOgreContext();
      updateFBO();
      mOgreFboId = getOgreFboId();
      mDirtyFBO = false;
      doneOgreContext();
    }
  }

  void OgreNode::updateFBO()
  {
    Ogre::TexturePtr rttTexture = mRenderSystem->getRtt("RttTex", mSize.width(), mSize.height());
    mRenderTarget = rttTexture->getBuffer()->getRenderTarget();

    Ogre::Camera* cam = mRenderSystem->getOgreCamera();

    mRenderTarget->addViewport(cam);

    Ogre::Real aspectRatio = Ogre::Real(mSize.width()) / Ogre::Real(mSize.height());
    cam->setAspectRatio(aspectRatio);

    QSGGeometry::updateTexturedRectGeometry(&mGeometry,
        QRectF(0, 0, mSize.width(), mSize.height()),
        QRectF(0, 0, 1, 1));

    Ogre::GLTexture *nativeTexture = static_cast<Ogre::GLTexture *>(rttTexture.get());

    delete mTexture;
    mTexture = mQuickWindow->createTextureFromId(nativeTexture->getGLID(), mSize);

    mMaterial.setTexture(mTexture);
    mMaterialO.setTexture(mTexture);

    LOG(INFO) << "Updated FBO, new size: " << mSize.width() << "x" << mSize.height() << ", texture id: " << nativeTexture->getGLID();
  }

  void OgreNode::setSize(const QSize &size)
  {
    if (size == mSize)
      return;

    mSize = size;
    mDirtyFBO = true;
    markDirty(DirtyGeometry);
  }

  bool OgreNode::onCameraChanged(EventDispatcher* sender, const Event& event)
  {
    LOG(INFO) << "Camera changed";
    mDirtyFBO = true;
    markDirty(DirtyGeometry);
    return true;
  }
}
