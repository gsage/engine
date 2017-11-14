#ifndef _OgreView_H_
#define _OgreView_H_

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

#include <string>

#include "imgui.h"

namespace Ogre {
  class Rectangle2D;
}

namespace Gsage {
  class OgreRenderSystem;
  class ImguiOgreWrapper;
  class ImguiManager;
  /**
   * Allows displaying ogre camera into imgui window
   */
  class OgreView
  {
    public:
      OgreView(OgreRenderSystem* render);
      virtual ~OgreView();

      /**
       * Render ogre view
       */
      void render(unsigned int width, unsigned int height);

      /**
       * Sets rendered texture id
       * @param textureID RTT texture
       */
      void setTextureID(const std::string& textureID);

    private:
      std::string mTextureID;
      OgreRenderSystem* mRender;

      unsigned int mWidth;
      unsigned int mHeight;
      Ogre::Rectangle2D* mRect;
      ImVec2 mPosition;
      bool mCaptureMouse;
  };
}

#endif
