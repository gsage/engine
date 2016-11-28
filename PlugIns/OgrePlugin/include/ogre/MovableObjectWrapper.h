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

  template<typename T>
  class MovableObjectWrapper : public OgreObject
  {
    public:
      MovableObjectWrapper()
        : mObject(0)
      {
        BIND_ACCESSOR_OPTIONAL("renderQueueGroup", &MovableObjectWrapper::setRenderQueueGroup, &MovableObjectWrapper::getRenderQueueGroup);
      }

      virtual ~MovableObjectWrapper() {
        mObject->detachFromParent();
        mSceneManager->destroyMovableObject(mObject);
      }

      void setRenderQueueGroup(const unsigned char& queueId)
      {
        if(mObject == 0) {
          LOG(ERROR) << "Render queue was not set";
          return;
        }

        mObject->setRenderQueueGroup((Ogre::uint8) queueId);
        LOG(INFO) << mObjectId << " set render queue group to " << queueId;
      }

      unsigned char getRenderQueueGroup()
      {
        if(mObject == 0)
          return 0;

        return mObject->getRenderQueueGroup();
      }
    protected:
      T* mObject;
      std::string mAttachedTo;
  };
}
#endif
