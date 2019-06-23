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

#ifndef _MovableObjectWrapper_H_
#define _MovableObjectWrapper_H_

#include <OgreSceneManager.h>

#include "ogre/OgreObject.h"

namespace Gsage {

  /**
   * Provides templateless base class for MovableObjectWrapper
   */
  class IMovableObjectWrapper : public OgreObject {
    public:
      virtual void setRenderQueueGroup(const unsigned char& queueId) = 0;
      virtual unsigned char getRenderQueueGroup() = 0;
      virtual void setVisibilityFlags(unsigned int mask) = 0;
      virtual void resetVisibilityFlags() = 0;
  };

  template<typename T>
  class MovableObjectWrapper : public IMovableObjectWrapper
  {
    public:
      MovableObjectWrapper()
        : mObject(0)
      {
        BIND_ACCESSOR_OPTIONAL("renderQueueGroup", &MovableObjectWrapper::setRenderQueueGroup, &MovableObjectWrapper::getRenderQueueGroup);
      }

      virtual ~MovableObjectWrapper() {
        if(mObject) {
          mObject->detachFromParent();
          mSceneManager->destroyMovableObject(mObject);
        }
      }

      void defineUserBindings()
      {
        if(mObject) {
          mObject->Ogre::MovableObject::getUserObjectBindings().setUserAny("entity", Ogre::Any(mOwnerId));
          resetVisibilityFlags();
        }
      }

      void setRenderQueueGroup(const unsigned char& queueId)
      {
        if(mObject == 0) {
          LOG(ERROR) << "Render queue was not set";
          return;
        }

        mObject->setRenderQueueGroup((Ogre::uint8) queueId);
        LOG(TRACE) << mObjectId << " set render queue group to " << (int)queueId;
      }

      unsigned char getRenderQueueGroup()
      {
        if(mObject == 0)
          return 0;

        return mObject->getRenderQueueGroup();
      }

      void setVisibilityFlags(unsigned int mask)
      {
        mObject->setVisibilityFlags((Ogre::uint32)mask);
      }

      void resetVisibilityFlags()
      {
        mObject->setVisibilityFlags(0xFFFFFFF0);
      }
    protected:
      T* mObject;
  };
}
#endif
