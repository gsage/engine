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

#include "SDLCore.h"
#include <SDL2/SDL.h>
#include "GsageFacade.h"
#include "EngineEvent.h"

namespace Gsage {

  SDLCore::SDLCore()
  {
  }

  SDLCore::~SDLCore()
  {
  }

  bool SDLCore::initialize(const DataProxy& params, GsageFacade* facade)
  {
    if(SDL_Init(0) < 0) {
      LOG(ERROR) << "Failed to initialize SDL " << SDL_GetError();
      return false;
    }
    mInitialized = true;
    mFacade = facade;
    facade->addUpdateListener(this);
    return true;
  }

  void SDLCore::update(const float& time)
  {
    SDL_Event event;
    SDL_PumpEvents();
    while(SDL_PollEvent(&event) != 0) {
      if(event.type == SDL_QUIT) {
        mFacade->shutdown();
        return;
      }

      if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        mFacade->getEngine()->fireEvent(WindowEvent(WindowEvent::RESIZE, event.window.windowID, event.window.data1, event.window.data2));
      }

      for(auto listener : mEventListeners) {
        listener->handleEvent(&event);
      }
    }
  }

  void SDLCore::tearDown()
  {
    SDL_Quit();
    mInitialized = false;
  }

  void SDLCore::addEventListener(SDLEventListener* listener)
  {
    mEventListeners.push_back(listener);
  }
}
