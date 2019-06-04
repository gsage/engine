#ifndef _RENDERER_FACTORY_H_
#define _RENDERER_FACTORY_H_

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
#include "renderers/Image.hpp"

namespace Gsage {
  class SDLCore;

  class RendererFactory {
    public:
      RendererPtr create(const DataProxy& params, SDL_Renderer* renderer, SDLCore* core, WindowPtr window) {
        std::string type;
        if(!params.read("type", type)) {
          return nullptr;
        }

        RendererPtr res = nullptr;

        if(type == "image") {
          res = RendererPtr(new ImageRenderer(params, renderer, core, window));
        }

        return res;
      }
  };
}

#endif
