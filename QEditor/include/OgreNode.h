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

#ifndef _OgreNode_H_
#define _OgreNode_H_

#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGTextureMaterial>
#include <QtQuick/QSGOpaqueTextureMaterial>
#include <QtQuick/QQuickWindow>
#include <Ogre.h>
#include "EventSubscriber.h"
#include "EventDispatcher.h"

namespace Ogre {
  class Root;
  class SceneManager;
  class RenderTexture;
  class Viewport;
  class RenderTarget;
}

namespace Gsage
{
  class Engine;
  class QOgreRenderSystem;

  class OgreNode : public QSGGeometryNode, public EventSubscriber<OgreNode>
  {
  public:
      OgreNode(QQuickWindow* window);
      ~OgreNode();

      void setSize(const QSize &size);
      QSize size() const { return mSize; }

      void update();
      void updateFBO();

      GLuint getOgreFboId();

      void setEngine(Engine* engine);
      void doneOgreContext();
      void activateOgreContext();

      void preprocess();

  private:
      bool onCameraChanged(EventDispatcher* sender, const Event& event);

      QSGTextureMaterial mMaterial;
      QSGOpaqueTextureMaterial mMaterialO;
      QSGGeometry mGeometry;
      QSGTexture *mTexture;
      QOgreRenderSystem *mRenderSystem;
      QQuickWindow* mQuickWindow;

      QSize mSize;

      Ogre::RenderTexture *mRenderTarget;
      Ogre::Viewport *mViewport;
      Ogre::RenderWindow *mWindow;

      GLuint mOgreFboId;

      bool mDirtyFBO;

  };
}


#endif
