/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Artem Chernyshev

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


#include "OgreView.h"

#include "ImguiManager.h"

#include "systems/OgreRenderSystem.h"

#include "RenderTarget.h"

#include "v1/ImguiRendererV1.h"

namespace Gsage {

  float convertCoordinate(const float coord, const float size)
  {
    return (coord / size) * 2 - 1;
  }

  OgreView::OgreView(OgreRenderSystem* render, ImVec4 bgColour)
    : mRender(render)
    , mWidth(1)
    , mHeight(1)
    , mBgColour(bgColour)
  {
    mViewport = new ViewportRenderData();
  }

  OgreView::~OgreView()
  {
    if(mTexture) {
      removeEventListener(mTexture.get(), Texture::RECREATE, &OgreView::onTextureEvent);
      removeEventListener(mTexture.get(), Texture::DESTROY, &OgreView::onTextureEvent);
      removeEventListener(mTexture.get(), Texture::UV_UPDATE, &OgreView::onTextureEvent);
    }
    delete mViewport;
  }

  void OgreView::render(unsigned int width, unsigned int height)
  {
    if(width <= 0 || height <= 0)
      return;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->PathClear();
    drawList->AddRectFilled(ImVec2(pos.x, pos.y), pos + ImGui::GetContentRegionAvail(), ImGui::ColorConvertFloat4ToU32(mBgColour));

    if(mTextureID.empty())
      return;

    bool widthChange = mWidth != width;
    bool heightChange = mHeight != height;
    mWidth = width;
    mHeight = height;

    RenderTargetPtr renderTarget = mRender->getRenderTarget(mTextureID);

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 size = io.DisplaySize;
    bool posChange = pos.x != mPosition.x || pos.y != mPosition.y;

    mPosition = pos;

    if(renderTarget) {
      if(widthChange || heightChange || renderTarget->getWidth() != mWidth && renderTarget->getHeight() != mHeight) {
        renderTarget->setDimensions(mWidth, mHeight);
      }

      renderTarget->setPosition(pos.x, pos.y);
    }

    if(widthChange || heightChange || posChange) {
      ImVec2 size = ImGui::GetContentRegionAvail();
      if(mTexture) {
        Gsage::Vector2 texSize = mTexture->getSrcSize();
        size = ImVec2(texSize.X, texSize.Y);
      }
      mViewport->updatePos(pos);
      mViewport->updateSize(size);
      if(mTexture) {
        mViewport->updateUVs(mTexture->getUVs());
      }
      mViewport->updateVertexBuffer();
    }

    drawList->AddCallback(NULL, mViewport);
    if(!mTexture && (widthChange || heightChange)) {
      setTextureID(mTextureID);
    }
  }

  void OgreView::setTextureID(const std::string& textureID)
  {
    mViewport->setDatablock(textureID);
    mTextureID = textureID;
  }

  void OgreView::setTexture(TexturePtr texture)
  {
    if(mTexture) {
      removeEventListener(mTexture.get(), Texture::RECREATE, &OgreView::onTextureEvent);
      removeEventListener(mTexture.get(), Texture::DESTROY, &OgreView::onTextureEvent);
      removeEventListener(mTexture.get(), Texture::UV_UPDATE, &OgreView::onTextureEvent);
    }
    mTexture = texture;
    setTextureID(texture->getName());
    addEventListener(texture.get(), Texture::RECREATE, &OgreView::onTextureEvent);
    addEventListener(texture.get(), Texture::DESTROY, &OgreView::onTextureEvent);
    addEventListener(texture.get(), Texture::UV_UPDATE, &OgreView::onTextureEvent);
  }

  bool OgreView::setTexture(const std::string& id)
  {
    TexturePtr tex = mRender->getTexture(id);
    if(!tex) {
      return false;
    }

    setTexture(tex);
    return true;
  }

  bool OgreView::onTextureEvent(EventDispatcher* sender, const Event& event)
  {
    if(event.getType() == Texture::RECREATE) {
      if(mTexture->hasData()) {
        setTextureID(mTexture->getName());
      }
    } else if(event.getType() == Texture::UV_UPDATE) {
      mViewport->updateUVs(mTexture->getUVs());
    } else if(event.getType() == Texture::DESTROY) {
      mViewport->resetDatablock();
      return true;
    }
    Gsage::Vector2 texSize = mTexture->getSrcSize();
    ImVec2 size(texSize.X, texSize.Y);
    mViewport->updateSize(size);
    mViewport->updateVertexBuffer();
    return true;
  }
}
