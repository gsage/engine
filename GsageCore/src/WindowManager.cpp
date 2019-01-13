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

#include "WindowManager.h"
#include "EngineEvent.h"

namespace Gsage {

  WindowManager::WindowManager(const std::string& type)
    : mType(type)
  {
  }

  WindowManager::~WindowManager()
  {
    mWindows.clear();
  }

  const std::string& WindowManager::getType() const
  {
    return mType;
  }

  void WindowManager::fireWindowEvent(Event::ConstType type, unsigned long handle, unsigned int width, unsigned int height)
  {
    fireEvent(WindowEvent(type, handle, width, height));
  }

  WindowPtr WindowManager::getWindow(const std::string& name)
  {
    if(mWindows.count(name) == 0) {
      return nullptr;
    }

    return mWindows[name];
  }

  void WindowManager::windowCreated(WindowPtr window)
  {
    mWindows[window->getName()] = window;
  }

  bool WindowManager::windowDestroyed(WindowPtr window)
  {
    if(mWindows.count(window->getName()) > 0) {
      fireWindowEvent(WindowEvent::CLOSE, window->getWindowHandle());
      mWindows.erase(window->getName());
      return true;
    }
    return false;
  }
}
