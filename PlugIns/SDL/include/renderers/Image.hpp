#ifndef _IMAGE_RENDERER_H_
#define _IMAGE_RENDERER_H_

/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2019 Gsage Authors

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

#include <SDL.h>
#include "DataProxy.h"
#include "renderers/Renderer.hpp"
#include "SDLCore.h"

namespace Gsage {
  /**
   * Really simple image renderer
   */
  class ImageRenderer : public Renderer {
    public:
      ImageRenderer(const DataProxy& params, SDL_Renderer* renderer, SDLCore* core, WindowPtr window)
        : Renderer(window, core)
        , mRenderer(renderer)
        , mImage(0)
        , mTexture(0)
        , mCustomRect(false)
      {
        std::string path = params.get("image", "");
        path = mCore->getResourcePath() + GSAGE_PATH_SEPARATOR + path;

        mImage = SDL_LoadBMP(path.c_str());
        mTexture = SDL_CreateTextureFromSurface(renderer, mImage);

        mCustomRect = params.read("x", mDestRect.x) || mCustomRect;
        mCustomRect = params.read("y", mDestRect.y) || mCustomRect;
        mCustomRect = params.read("w", mDestRect.w) || mCustomRect;
        mCustomRect = params.read("h", mDestRect.h) || mCustomRect;
      }

      ~ImageRenderer()
      {
        if(mTexture)
          SDL_DestroyTexture(mTexture);
        if(mImage)
          SDL_FreeSurface(mImage);
      }

      void render() {
        float scale = mWindow->getScaleFactor();
        SDL_Rect rect = {
          (int)(mDestRect.x * scale),
          (int)(mDestRect.y * scale),
          (int)(mDestRect.w * scale),
          (int)(mDestRect.h * scale)
        };

        SDL_RenderCopy(mRenderer, mTexture, NULL, mCustomRect ? &rect : NULL);
      }
    private:
      SDL_Surface* mImage;
      SDL_Texture* mTexture;
      SDL_Renderer* mRenderer;

      SDL_Rect mDestRect;

      bool mCustomRect;
  };
}


#endif
