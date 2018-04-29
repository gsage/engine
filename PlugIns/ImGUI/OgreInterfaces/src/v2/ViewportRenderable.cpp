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

#include "v2/ViewportRenderable.h"
#include "OgreStableHeaders.h"

#include <OgreRoot.h>
#include <OgreHardwareBufferManager.h>
#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreHardwareIndexBuffer.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreTextureManager.h>
#include "Logger.h"

namespace Gsage {

  ViewportRenderData::ViewportRenderData()
    : mTexUnitState(0)
    , mDatablock(0)
    , mDirty(false)
  {
    memset(&mVertexBuffer, 0, sizeof(mVertexBuffer));
    mIndexBuffer[0] = 0;
    mIndexBuffer[1] = 1;
    mIndexBuffer[2] = 2;
    mIndexBuffer[3] = 0;
    mIndexBuffer[4] = 2;
    mIndexBuffer[5] = 3;

    mVertexBuffer[0].uv = ImVec2(0.f, 1.f);
    mVertexBuffer[1].uv = ImVec2(1.f, 1.f);
    mVertexBuffer[2].uv = ImVec2(1.f, 0.f);
    mVertexBuffer[3].uv = ImVec2(0.f, 0.f);

    mVertexBuffer[0].col = 0xFFFFFFFF;
    mVertexBuffer[1].col = 0xFFFFFFFF;
    mVertexBuffer[2].col = 0xFFFFFFFF;
    mVertexBuffer[3].col = 0xFFFFFFFF;
  }

  ViewportRenderData::~ViewportRenderData()
  {
  }

  void ViewportRenderData::update(ImVec2 pos, ImVec2 size)
  {
    mDrawCmd.ElemCount = 6;
    mDrawCmd.ClipRect = ImVec4(pos.x, pos.y, size.x, size.y);
    mVertexBuffer[0].pos.x = pos.x;
    mVertexBuffer[0].pos.y = pos.y + size.y;

    mVertexBuffer[1].pos.x = pos.x + size.x;
    mVertexBuffer[1].pos.y = pos.y + size.y;

    mVertexBuffer[2].pos.x = pos.x + size.x;
    mVertexBuffer[2].pos.y = pos.y;

    mVertexBuffer[3].pos = pos;
  }

  void ViewportRenderData::setDatablock(const Ogre::String& name)
  {
    auto manager = Ogre::Root::getSingletonPtr()->getHlmsManager();
    Ogre::HlmsDatablock* datablock = manager->getDatablockNoDefault(name);
    if(datablock) {
      mDatablock = datablock;
    }

    mTextureName = name;
    mDirty = true;
  }

  Ogre::TexturePtr ViewportRenderData::getRenderTexture()
  {
    Ogre::TextureManager& texManager = Ogre::TextureManager::getSingleton();
    if(mTextureName.empty() || !texManager.resourceExists(mTextureName)){
      Ogre::TexturePtr null;
      null.setNull();
      return null;
    }

    return texManager.getByName(mTextureName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
  }
}
