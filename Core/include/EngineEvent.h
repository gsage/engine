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

#ifndef _EngineEvent_H_
#define _EngineEvent_H_

#include <OgreVector3.h>
#include "EventDispatcher.h"

namespace Gsage {
  /**
   * Engine related event
   */
  class EngineEvent : public Event
  {
    public:
      /**
       * Engine issued stopping
       */
      static const std::string HALT;
      EngineEvent(const std::string& type);
      virtual ~EngineEvent();
  };

  /**
   * Some entity was selected
   */
  class SelectEvent : public Event
  {
    public:
      /**
       * Object selected
       */
      static const std::string OBJECT_SELECTED;
      /**
       * Object is rolled over
       */
      static const std::string ROLL_OVER;
      /**
       * Object is rolled out
       */
      static const std::string ROLL_OUT;
      SelectEvent(const std::string& type, const unsigned int& flags, const Ogre::Vector3& intersection, const std::string& entityId) 
        : mFlags(flags)
        , mIntersection(intersection)
        , mEntityId(entityId)
        , Event(type)
      {
      }
      virtual ~SelectEvent() {};

      /**
       * Hit point
       */
      Ogre::Vector3 mIntersection;

      /**
       * Check entity flags
       * @param flags Flags mask
       */
      bool hasFlags(const unsigned int& flags) const
      {
        return (mFlags & flags) == flags;
      }

      /**
       * Get selected entity id
       */
      const std::string& getEntityId() const { return mEntityId; }

    private:
      unsigned int mFlags;
      std::string mEntityId;
  };
}

#endif

