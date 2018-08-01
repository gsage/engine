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
  {
  }

  ImguiOgreRenderer::~ImguiOgreRenderer()
  {
  }

  void ImguiOgreRenderer::initialize(Engine* engine, lua_State* L)
  {
    ImguiRenderer::initialize(engine, L);
    sol::state_view lua(L);
    lua["imgui"]["createOgreView"] = [this] () -> std::shared_ptr<OgreView>{
      OgreRenderSystem* render = mEngine->getSystem<OgreRenderSystem>();
      if(render == 0) {
        return std::shared_ptr<OgreView>(nullptr);
      }
      return std::shared_ptr<OgreView>(new OgreView(render));
    };
    lua.new_usertype<OgreView>("OgreView",
        "setTextureID", &OgreView::setTextureID,
        "render", &OgreView::render
    );

    sol::table gizmo = lua.create_table();

    gizmo["IsOver"] = [] () -> bool { return ImGuizmo::IsOver(); };
    gizmo["IsUsing"] = [] () -> bool { return ImGuizmo::IsUsing(); };
    gizmo["ROTATE"] = ImGuizmo::OPERATION::ROTATE;
    gizmo["TRANSLATE"] = ImGuizmo::OPERATION::TRANSLATE;
    gizmo["SCALE"] = ImGuizmo::OPERATION::SCALE;

    gizmo["WORLD"] = ImGuizmo::MODE::WORLD;
    gizmo["LOCAL"] = ImGuizmo::MODE::LOCAL;

    lua["imgui"]["gizmo"] = gizmo;

    lua["imgui"]["createGizmo"] = [this] () -> std::shared_ptr<Gizmo>{
      OgreRenderSystem* render = mEngine->getSystem<OgreRenderSystem>();
      if(render == 0) {
        return std::shared_ptr<Gizmo>(nullptr);
      }
      return std::shared_ptr<Gizmo>(new Gizmo(render));
    };
    lua.new_usertype<Gizmo>("Gizmo",
        "setTarget", &Gizmo::setTarget,
        "enable", &Gizmo::enable,
        "render", &Gizmo::render,
        "operation", sol::property(&Gizmo::getOperation, &Gizmo::setOperation),
        "mode", sol::property(&Gizmo::getMode, &Gizmo::setMode),
        "drawCoordinatesEditor", &Gizmo::drawCoordinatesEditor
    );

    addEventListener(mEngine, RenderEvent::RENDER_QUEUE_STARTED, &ImguiOgreRenderer::renderQueueStarted);
    addEventListener(mEngine, RenderEvent::RENDER_QUEUE_ENDED, &ImguiOgreRenderer::renderQueueEnded);

    OgreRenderSystem* renderSystem = mEngine->getSystem<OgreRenderSystem>();
    mSceneMgr = renderSystem->getSceneManager();
    if(!mSceneMgr)
      return;

    createMaterial();
  }

  void ImguiOgreRenderer::createFontTexture(unsigned char* pixels, int width, int height)
  {
    mFontTex = Ogre::TextureManager::getSingleton().createManual("ImguiFontTex",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D,
        width,
        height,
        1,
        1,
        Ogre::PF_R8G8B8A8
    );

    const Ogre::PixelBox& lockBox = mFontTex->getBuffer()->lock(Ogre::Image::Box(0, 0, width, height), OgreV1::HardwareBuffer::HBL_DISCARD);
    size_t texDepth = Ogre::PixelUtil::getNumElemBytes(lockBox.format);

    memcpy(lockBox.data, pixels, width * height * texDepth);
    mFontTex->getBuffer()->unlock();
  }

  bool ImguiOgreRenderer::renderQueueStarted(EventDispatcher* sender, const Event& event)
  {
    RenderEvent e = static_cast<const RenderEvent&>(event);
    const std::string& name = e.renderTarget->getName();
#if OGRE_VERSION < 0x020100
    if(e.queueID != RENDER_QUEUE_IMGUI)
    {
      return true;
    }
#else
    if(mRenderTargetWhitelist.size() > 0 && \
       mRenderTargetWhitelist.count(name) == 0) {
      return true;
    }
#endif

    newFrame(name, (float)e.renderTarget->getWidth(), (float)e.renderTarget->getHeight());
    return true;
  }

  bool ImguiOgreRenderer::renderQueueEnded(EventDispatcher* sender, const Event& e)
  {
    RenderEvent event = static_cast<const RenderEvent&>(e);
#if OGRE_VERSION < 0x020100
    if(event.queueID != RENDER_QUEUE_IMGUI)
    {
      return true;
    }
#else
    if(mRenderTargetWhitelist.size() > 0 && \
       mRenderTargetWhitelist.count(event.renderTarget->getName()) == 0) {
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

    if(!endFrame()) {
      return true;
    }

    ImVec2 displaySize((float)event.renderTarget->getWidth(), (float)event.renderTarget->getHeight());
    updateVertexData(vp, displaySize);
    return true;
  }
}
