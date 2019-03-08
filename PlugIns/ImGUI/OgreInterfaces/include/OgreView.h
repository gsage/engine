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
#include "Definitions.h"
#include "EventSubscriber.h"
#include "systems/RenderSystem.h"
#include "ViewportRenderable.h"

namespace Gsage {
  class OgreRenderSystem;
  class ImguiOgreWrapper;
  class ImguiManager;
  /**
   * Allows displaying ogre camera into imgui window.
   *
   * \verbatim embed:rst:leading-asterisk
   *
   * Lua usage example:
   *
   * .. code-block:: lua
   *
   *  -- create ogre viewport
   *  viewport = imgui.createOgreView("#000000FF")
   *
   *  local textureID = "myOgreView"
   *
   *  -- render camera to texture
   *  local cam = camera:create("free")
   *  cam:renderToTexture(textureID, {
   *    autoUpdated = false
   *  })
   *
   *  -- set ogre view texture
   *  viewport:setTextureID(textureID)
   *  -- update on each imgui render call
   *  viewport:render(320, 240)
   *
   * \endverbatim
   */
  class OgreView : public EventSubscriber<OgreView>
  {
    public:
      OgreView(OgreRenderSystem* render, ImVec4 bgColour);
      virtual ~OgreView();

      /**
       * Render ogre view
       */
      void render(unsigned int width, unsigned int height);

      /**
       * Sets rendered manual texture object
       * @param texture Texture object
       */
      void setTexture(TexturePtr texture);

      /**
       * Sets texture by texture id
       * @param id texture id
       * @returns true if succeed
       */
      bool setTexture(const std::string& id);

    private:
      /**
       * Sets rendered texture id
       * @param textureID RTT texture
       */
      void setTextureID(const std::string& textureID);
      bool onTextureEvent(EventDispatcher* sender, const Event& event);

      std::string mTextureID;
      TexturePtr mTexture;
      OgreRenderSystem* mRender;

      unsigned int mWidth;
      unsigned int mHeight;
      ViewportRenderData* mViewport;
      ImVec2 mPosition;
      ImVec4 mBgColour;
  };
}

#endif
