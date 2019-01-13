#ifndef _WindowManager_H_
#define _WindowManager_H_

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

#include "DataProxy.h"
#include "EventDispatcher.h"

namespace Gsage {
  /**
   * Abstract window class
   */
  class Window
  {
    public:
      Window(const std::string& name) : mName(name) {};
      /**
       * Get window handle
       */
      virtual unsigned long long getWindowHandle() = 0;

      /**
       * Get openGL context
       */
      virtual void* getGLContext() = 0;

      /**
       * Gets window name
       */
      inline const std::string& getName() const { return mName; }

      /**
       * Gets window position
       */
      virtual std::tuple<int, int> getPosition() const = 0;

    private:
      std::string mName;
  };

  /**
   * Window pointer
   */
  typedef std::shared_ptr<Window> WindowPtr;

  /**
   * Abstract window manager
   */
  class WindowManager : public EventDispatcher
  {
    public:
      WindowManager(const std::string& type);
      virtual ~WindowManager();

      /**
       * Get window manager type
       * @returns this window manager type
       */
      const std::string& getType() const;

      /**
       * Initialize WindowManager
       *
       * @param config Window manager configuration map
       */
      virtual bool initialize(const DataProxy& config) = 0;

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
      virtual WindowPtr createWindow(const std::string& name, unsigned int width, unsigned int height,
          bool fullscreen, const DataProxy& params) = 0;

      /**
       * Close a window
       *
       * @param window window pointer to destroy
       * @returns true if succeed
       */
      virtual bool destroyWindow(WindowPtr window) = 0;

      /**
       * Fire window event
       *
       * @param type event type
       * @param handle handle of the target window
       * @param width current window window
       * @param height current window height
       */
      virtual void fireWindowEvent(Event::ConstType type, unsigned long handle, unsigned int width = 0, unsigned int height = 0);

      /**
       * Get window by name
       *
       * @param name Window name
       *
       * @returns WindowPtr if found
       */
      virtual WindowPtr getWindow(const std::string& name);
    protected:
      const std::string mType;
      std::map<std::string, WindowPtr> mWindows;

      void windowCreated(WindowPtr window);
      bool windowDestroyed(WindowPtr window);
  };

  typedef std::shared_ptr<WindowManager> WindowManagerPtr;
}

#endif
