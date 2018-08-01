#ifndef _ImguiRenderable_H_
#define _ImguiRenderable_H_

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

#include <Vao/OgreVertexBufferPacked.h>
#include <OgreRenderable.h>
#include "imgui.h"

namespace Ogre {
  class VaoManager;
}

namespace Gsage {
  class ImguiRenderable : public Ogre::Renderable
  {
    public:
      ImguiRenderable(Ogre::VaoManager* vaoManager);
      virtual ~ImguiRenderable();

      /**
       * Update renderable vertex data
       *
       * @param vtxBuf Imgui vertex buffer
       * @param idxBuf Imgui index buffer
       * @param vtxCount Actual vertex count
       * @param idxCount Actual index count
       */
      void updateVertexData(
          const ImDrawVert* vtxBuf,
          const ImDrawIdx* idxBuf,
          unsigned int vtxCount,
          unsigned int idxCount
      );

      /**
       * Unused for V2
       */
      void getRenderOperation(Ogre::v1::RenderOperation& op, bool casterPass);

      /**
       * Unused for V2
       */
      void getWorldTransforms(Ogre::Matrix4* xform) const;

      /**
       * Unused for V2
       */
      virtual const Ogre::LightList& getLights(void) const;
    private:
      Ogre::VertexElement2Vec mElements;
      Ogre::VertexBufferPacked* mVertexBuffer;
      Ogre::VertexArrayObject * mVao;
      Ogre::VaoManager* mVaoManager;
  };
}

#endif
