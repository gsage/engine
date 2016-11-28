/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2016 Artem Chernyshev

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

#include "WindowEventListener.h"
#include "EngineEvent.h"
#include "Engine.h"

namespace Gsage {

  WindowEventListener::WindowEventListener(Ogre::RenderWindow* window, Engine* engine)
    : mEngine(engine)
  {
    fireWindowEvent(WindowEvent::CREATE, window);
  }

  WindowEventListener::~WindowEventListener()
  {
  }

  void WindowEventListener::windowResized(Ogre::RenderWindow* window)
  {
    fireWindowEvent(WindowEvent::RESIZE, window);
  }

  void WindowEventListener::windowClosed(Ogre::RenderWindow* window)
  {
    fireWindowEvent(WindowEvent::CLOSE, window);
  }

  void WindowEventListener::fireWindowEvent(const std::string& type, Ogre::RenderWindow* window)
  {
    size_t handle;
    window->getCustomAttribute("WINDOW", &handle);
    unsigned int width, height, depth;
    int left, top;
    window->getMetrics(width, height, depth, left, top);
    mEngine->fireEvent(WindowEvent(type, handle, width, height));
  }
}
