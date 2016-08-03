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

#ifndef _RenderEvent_H_
#define _RenderEvent_H_

#include <OgreRenderSystem.h>
#include <OgreRenderWindow.h>

#include "EventDispatcher.h"

namespace Gsage {

  /**
   * Event that is used to update any UI overlay system
   */
  class RenderEvent : public Event
  {
    public:
      static const std::string UPDATE_UI;

      RenderEvent(const std::string& type, Ogre::RenderSystem* renderSystem, Ogre::RenderWindow* window);
      virtual ~RenderEvent();
      /**
       * Gets a reference to the current render system
       */
      Ogre::RenderSystem* getRenderSystem();
      /**
       * Gets a reference to the current render window
       */
      Ogre::RenderWindow* getRenderWindow();

    private:
      Ogre::RenderSystem* mRenderSystem;
      Ogre::RenderWindow* mRenderWindow;
  };
}

#endif
