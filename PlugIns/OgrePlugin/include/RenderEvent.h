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

#include <systems/OgreRenderSystem.h>

#include "EventDispatcher.h"

namespace Gsage {

  /**
   * Event that is used to update any UI overlay system
   */
  class RenderEvent : public Event
  {
    public:
      static const std::string UPDATE_UI;

      /**
       * Main update
       */
      static const std::string UPDATE;

      /**
       * Same to ogre render queue ended
       */
      static const std::string RENDER_QUEUE_ENDED;

      RenderEvent(const std::string& type, OgreRenderSystem* renderSystem, Ogre::uint8 queueID = 0, const std::string& invocation = "");
      virtual ~RenderEvent();
      /**
       * Gets a reference to the current render system
       */
      OgreRenderSystem* getRenderSystem();

      /**
       * Ogre invocation string
       */
      std::string invocation;

      /**
       * Ogre queue id
       */
      Ogre::uint8 queueID;

    private:
      OgreRenderSystem* mRenderSystem;
  };
}

#endif
