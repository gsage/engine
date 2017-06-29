#ifndef _CameraWrapper_H_
#define _CameraWrapper_H_
#
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

#include "ogre/MovableObjectWrapper.h"

namespace Ogre {
  class Camera;
  class Viewport;
  class RenderWindow;
}

namespace Gsage {
  class CameraWrapper : public MovableObjectWrapper<Ogre::Camera>
  {
    public:
      static const std::string TYPE;
      CameraWrapper();
      virtual ~CameraWrapper();

      /**
       * Sets camera orientation
       *
       * @param orientation Ogre::Quaternion
       */
      void setOrientation(const Ogre::Quaternion& orientation);

      /**
       * Gets camera orientation
       */
      const Ogre::Quaternion& getOrientation() const;

      /**
       * Activate this camera
       */
      void attach(Ogre::Viewport* viewport);

      /**
       * Create camera
       * @param name: Camera name
       */
      void createCamera(const std::string& name);

      /**
       * Get camera ogre object
       */
      Ogre::Camera* getCamera();

      /**
       * Get camera name
       */
      const std::string& getName() const;

      /**
       * Set camera clip distance
       * @param value Clip distance
       */
      void setClipDistance(const float& value);
      /**
       * Get camera clip distance
       */
      float getClipDistance();
      /**
       * Set viewport background colour
       * @param colour ColourValue
       */
      void setBgColour(const Ogre::ColourValue& colour);
      /**
       * Get viewport background colour
       */
      const Ogre::ColourValue& getBgColour() const;

      /**
       * Check if camera is active
       */
      bool isActive() const;
    private:
      std::string mTarget;

      Ogre::ColourValue mBgColour;
      Ogre::Viewport* mViewport;
      Ogre::RenderWindow* mWindow;

      bool mIsActive;
  };
}

#endif
