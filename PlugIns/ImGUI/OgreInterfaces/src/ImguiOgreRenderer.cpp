/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2018 Artem Chernyshev and contributors

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

#include "ImguiOgreRenderer.h"
#include "sol.hpp"
#include "Gizmo.h"
#include "OgreView.h"
#include "systems/OgreRenderSystem.h"
#include "Engine.h"
#include "RenderEvent.h"
#include "ImguiDefinitions.h"

namespace Gsage {

  ImguiOgreRenderer::ImguiOgreRenderer()
    : mSceneMgr(0)
    , mUpdateFontTex(false)
  {
  }

  ImguiOgreRenderer::~ImguiOgreRenderer()
  {
    removeEventListener(mEngine, RenderEvent::RENDER_QUEUE_ENDED, &ImguiOgreRenderer::renderQueueEnded);
  }

  void ImguiOgreRenderer::initialize(Engine* engine, lua_State* L)
  {
    ImguiRenderer::initialize(engine, L);

    OgreRenderSystem* renderSystem = mEngine->getSystem<OgreRenderSystem>();
    mSceneMgr = renderSystem->getSceneManager();
    if(!mSceneMgr)
      return;

    createMaterial();

    // now, as imgui is ready for rendering, we can register subscribe to events
    addEventListener(mEngine, RenderEvent::RENDER_QUEUE_ENDED, &ImguiOgreRenderer::renderQueueEnded);
  }

  void ImguiOgreRenderer::createFontTexture(unsigned char* pixels, int width, int height)
  {
    mFontTexLock.lock();
    mFontPixels = pixels;
    mFontTexWidth = width;
    mFontTexHeight = height;
    mUpdateFontTex = true;
    mFontTexLock.unlock();
  }

  bool ImguiOgreRenderer::renderQueueEnded(EventDispatcher* sender, const Event& e)
  {
    RenderEvent event = static_cast<const RenderEvent&>(e);
#if OGRE_VERSION < 0x020100
    if(event.queueID != RENDER_QUEUE_IMGUI)
    {
      return true;
    }
#endif

    Ogre::Viewport* vp = event.renderTarget->getViewport();

    if(vp == NULL || !vp->getTarget()->isPrimary()
#if OGRE_VERSION < 0x020100
       || !vp->getOverlaysEnabled()
#endif
      )
      return true;

    const std::string& name = event.renderTarget->getName();

    mFontTexLock.lock();
    if(mUpdateFontTex)
    {
      mFontTex = Ogre::TextureManager::getSingleton().createManual("ImguiFontTex",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
          Ogre::TEX_TYPE_2D,
          mFontTexWidth,
          mFontTexHeight,
          1,
          1,
          Ogre::PF_R8G8B8A8,
          Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE
      );

      const Ogre::PixelBox& lockBox = mFontTex->getBuffer()->lock(Ogre::Image::Box(0, 0, mFontTexWidth, mFontTexHeight), OgreV1::HardwareBuffer::HBL_DISCARD);
      size_t texDepth = Ogre::PixelUtil::getNumElemBytes(lockBox.format);

      memcpy(lockBox.data, mFontPixels, mFontTexWidth * mFontTexHeight * texDepth);
      mFontTex->getBuffer()->unlock();
      updateFontTexture();
      mUpdateFontTex = false;
    }
    mFontTexLock.unlock();

    mContextLock.lock();
    if(mContexts.count(name) == 0) {
      initializeContext(name);
    }

    mContexts[name].size = ImVec2(
        (float)event.renderTarget->getWidth(),
        (float)event.renderTarget->getHeight()
    );

    if(!mContexts[name].context) {
      mContextLock.unlock();
      return true;
    }
    ImGui::SetCurrentContext(mContexts[name].context);
    updateVertexData(vp, mContexts[name].size);
    mContextLock.unlock();
    return true;
  }
}
