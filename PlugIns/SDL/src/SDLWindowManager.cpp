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

#include "SDLWindowManager.h"
#include "EngineEvent.h"

#if GSAGE_PLATFORM == GSAGE_APPLE
#include "OSXUtils.h"
#endif
#include <SDL_syswm.h>

namespace Gsage {

  SDLWindow::SDLWindow(const std::string& name, SDL_Window* window)
    : Window(name)
    , mWindow(window)
    , mGLContext(nullptr)
  {
  }

  SDLWindow::~SDLWindow()
  {
    SDL_DestroyWindow(mWindow);
  }

  unsigned long long SDLWindow::getWindowHandle()
  {
    unsigned long handle = 0;
    SDL_SysWMinfo windowInfo;
    SDL_VERSION(&windowInfo.version);
    if(SDL_GetWindowWMInfo(mWindow, &windowInfo) == SDL_FALSE) {
      LOG(ERROR) << "Failed to get window info";
      return 0;
    }
#if GSAGE_PLATFORM == GSAGE_APPLE
    handle = WindowContentViewHandle(windowInfo);
#elif GSAGE_PLATFORM == GSAGE_LINUX
    handle = (unsigned long long) windowInfo.info.x11.window;
#elif GSAGE_PLATFORM == GSAGE_WIN32
    LOG(INFO) << "Win handle " << windowInfo.info.win.window;
    handle = (unsigned long long) windowInfo.info.win.window;
#endif

    return handle;
  }

  std::tuple<int, int> SDLWindow::getPosition() const
  {
    int x, y;
    SDL_GetWindowPosition(mWindow, &x, &y);
    return std::make_tuple(x, y);
  }

  void* SDLWindow::getGLContext()
  {
    if(mGLContext == nullptr) {
      return 0;
    }

    return (void*) mGLContext;
  }

  SDLWindowManager::SDLWindowManager(const std::string& type)
    : WindowManager(type)
  {
  }

  SDLWindowManager::~SDLWindowManager()
  {
  }

  bool SDLWindowManager::initialize(const DataProxy& config)
  {
    int initializationCode = SDL_InitSubSystem(SDL_INIT_VIDEO);
    if(initializationCode < 0) {
      LOG(ERROR) << "Failed to initialize video subsystem " << SDL_GetError();
      return false;
    }

    auto windows = config.get<DataProxy>("windows");
    if(!windows.second) {
      return true;
    }

    for(auto pair : windows.first) {
      LOG(TRACE) << "Creating window " << pair.first;
      WindowPtr w = createWindow(pair.first, pair.second.get("width", 320), pair.second.get("height", 240), pair.second.get("fullscreen", false), pair.second);
      if(w == nullptr) {
        LOG(WARNING) << "Failed to create window " << pair.first;
      }
    }

    return true;
  }

  WindowPtr SDLWindowManager::createWindow(const std::string& name, unsigned int width, unsigned int height,
          bool fullscreen, const DataProxy& params)
  {
    Uint32 flags = 0;
    int x = params.get("x", SDL_WINDOWPOS_UNDEFINED);
    int y = params.get("y", SDL_WINDOWPOS_UNDEFINED);
    bool showCursor = params.get("showCursor", false);

    std::map<std::string, Uint32> flagsMap;
    flagsMap["hidden"] = SDL_WINDOW_HIDDEN;
    flagsMap["borderless"] = SDL_WINDOW_BORDERLESS;
    flagsMap["openGL"] = SDL_WINDOW_OPENGL;
    flagsMap["resizable"] = SDL_WINDOW_RESIZABLE;
    flagsMap["minimized"] = SDL_WINDOW_MINIMIZED;
    flagsMap["maximized"] = SDL_WINDOW_MAXIMIZED;
    flagsMap["inputGrabbed"] = SDL_WINDOW_INPUT_GRABBED;
    flagsMap["highDPI"] = SDL_WINDOW_ALLOW_HIGHDPI;

    for(auto pair : flagsMap) {
      if(params.get(pair.first, false)) {
        flags |= pair.second;
      }
    }

    if(fullscreen) {
      flags |= SDL_WINDOW_FULLSCREEN;
    }

    SDL_Window* window = SDL_CreateWindow(name.c_str(), x, y, width, height, flags);
    if(window == NULL) {
      return nullptr;
    }

    SDL_GLContext sdlContext = nullptr;
    if((flags & SDL_WINDOW_OPENGL) != 0) {
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
      sdlContext = SDL_GL_CreateContext(window);
      if(!sdlContext) {
        LOG(ERROR) << "Failed to create SDL GL context";
        SDL_DestroyWindow(window);
        return nullptr;
      }
    }

    SDLWindow* wrapper = new SDLWindow(name, window);
    if(sdlContext) {
      wrapper->mGLContext = sdlContext;
    }

    mWindowsMutex.lock();
    WindowPtr res = WindowPtr(wrapper);
    windowCreated(res);
    fireWindowEvent(WindowEvent::CREATE, wrapper->getWindowHandle(), width, height);
    mWindowsMutex.unlock();
    return res;
  }

  bool SDLWindowManager::destroyWindow(WindowPtr window)
  {
    mWindowsMutex.lock();
    bool res = windowDestroyed(window);
    mWindowsMutex.unlock();
    return res;
  }

}
