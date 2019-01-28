#ifndef _ViewportRenderData_H_
#define _ViewportRenderData_H_

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

#include "OgrePrerequisites.h"
#include <OgreRenderable.h>
#include <OgreRenderOperation.h>
#include <OgreMovableObject.h>
#include <OgreHlmsDatablock.h>

#include "Definitions.h"
#include "imgui.h"
#include "systems/RenderSystem.h"

namespace Gsage {
  class ImguiRendererV1;

  class ViewportRenderData
  {
    public:
      ViewportRenderData();
      virtual ~ViewportRenderData();

      /**
       * Update position
       */
      void updatePos(ImVec2 pos);

      /**
       * Update size
       */
      void updateSize(ImVec2 size);

      /**
       * Update UV
       */
      void updateUVs(const Texture::UVs& uvs);

      /**
       * Update vertex buffer
       */
      void updateVertexBuffer();

      /**
       * Set viewport datablock
       */
      void setDatablock(const Ogre::String& name);

      /**
       * Get RTT to render
       */
      Ogre::TexturePtr getRenderTexture();
    private:
      friend class ImguiRendererV1;

      ImVec2 pos;
      ImVec2 size;

      ImDrawCmd  mDrawCmd;
      ImDrawVert mVertexBuffer[4];
      ImDrawIdx  mIndexBuffer[6];
      Ogre::HlmsDatablock* mDatablock;
      Ogre::String mTextureName;

      Ogre::TextureUnitState* mTexUnitState;
      ImVec2 mPos;
      ImVec2 mSize;

      bool mDirty;
  };
}

#endif
