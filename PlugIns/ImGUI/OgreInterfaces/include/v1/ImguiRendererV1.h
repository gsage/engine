#ifndef _ImguiRendererV1_H_
#define _ImguiRendererV1_H_

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

#include "EventSubscriber.h"
#include "RenderEvent.h"
#include "ImguiManager.h"
#include "ImguiOgreRenderer.h"

#if OGRE_VERSION >= 0x020100
#include <OgrePsoCacheHelper.h>
#endif

namespace Gsage {
  class Engine;
  class ImGUIRenderable;

  class ImguiRendererV1 : public ImguiOgreRenderer
  {
    public:
      ImguiRendererV1();
      virtual ~ImguiRendererV1();
      void initialize(Engine* engine, lua_State* L);
    protected:
      void updateVertexData(Ogre::Viewport* vp, ImVec2 displaySize);
      void createMaterial();
      void updateFontTexture();
      void setFiltering(Ogre::TextureFilterOptions mode);
    private:
      std::vector<ImGUIRenderable*> mRenderables;

      Ogre::Pass*	mPass;
      Ogre::TextureUnitState* mTexUnit;

#if OGRE_VERSION >= 0x020100
      Ogre::PsoCacheHelper* mPsoCache;

      std::vector<Ogre::uint32> mHashes;
#endif

  };
}

#endif
