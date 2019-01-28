#ifndef _SDLWindowManager_H_
#define _SDLWindowManager_H_

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

#include "WindowManager.h"
#include <mutex>
#include <SDL2/SDL.h>

namespace Gsage {
  class SDLWindowManager;

  class SDLWindow : public Window
  {
    public:
      SDLWindow(const std::string& name, SDL_Window* window);
      virtual ~SDLWindow();

      /**
       * Get window handle
       */
      unsigned long long getWindowHandle();

      /**
       * Get GL context
       */
      void* getGLContext();

      /**
       * Get window position
       */
      std::tuple<int, int> getPosition() const;

      /**
       * Sets window position
       *
       * @param x
       * @param y
       */
      virtual void setPosition(int x, int y);

      /**
       * Get window size
       */
      virtual std::tuple<int, int> getSize();

      /**
       * Set window size
       *
       * @param width
       * @param height
       */
      void setSize(int width, int height);

      /**
       * Get display size information
       * Selects the display which contains current window position
       */
      virtual std::tuple<int, int, int, int> getDisplayBounds();

    private:
      friend class SDLWindowManager;
      SDL_Window* mWindow;
      SDL_GLContext mGLContext;
  };

  /**
   * SDL Window Manager implementation
   */
  class SDLWindowManager : public WindowManager
  {
    public:
      SDLWindowManager(const std::string& type);
      virtual ~SDLWindowManager();

      /**
       * Inialize SDL system
       *
       * @param config SDLWindowManager
       */
      bool initialize(const DataProxy& config);

      /**
       * Create a window
       *
       * @param create a window
       * @param width window width
       * @param height window height
       * @param fullscreen create a fullscreen window
       * @param params additional parameters
       * @returns newly created window pointer
       */
      WindowPtr createWindow(const std::string& name, unsigned int width, unsigned int height,
          bool fullscreen, const DataProxy& params);

      /**
       * Close a window
       *
       * @param window window pointer to destroy
       * @returns true if succeed
       */
      virtual bool destroyWindow(WindowPtr window);
    private:
      std::mutex mWindowsMutex;
  };
}

#endif
