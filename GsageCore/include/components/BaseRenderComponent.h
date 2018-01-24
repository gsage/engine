#ifndef _BaseRenderComponent_H_
#define _BaseRenderComponent_H_

/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2018 Artem Chernyshev and contributors

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

#include "GeometryPrimitives.h"

namespace Gsage {
  class BaseRenderComponent
  {
    public:
      BaseRenderComponent();
      virtual ~BaseRenderComponent();
      /**
       * Set render component position
       * @param x X coordinate
       * @param y Y coordinate
       * @param z Z coordinate
       */
      void setPosition(float x, float y, float z);
      /**
       * Set position by using Gsage::Vector
       * @param position New position
       */
      virtual void setPosition(const Gsage::Vector3& position) = 0;
      /**
       * Set orientation using Gsage::Quaternion
       * @param orientation Orientation quaternion (absolute)
       */
      virtual void setOrientation(const Gsage::Quaternion& orientation) = 0;
      /**
       * Rotates render component using Gsage::Quaternion
       * @param rotation Rotation quaternion (relative)
       */
      virtual void rotate(const Gsage::Quaternion& rotation) = 0;
      /**
       * @param position Position to look at
       * @param rotationAxis rotate only in one axis
       * @param transformSpace ogre transform space
       */
      virtual void lookAt(const Gsage::Vector3& position, const Geometry::RotationAxis rotationAxis, Geometry::TransformSpace transformSpace = Geometry::TransformSpace::TS_WORLD) = 0;

      /**
       * @param position Position to look at
       */
      virtual void lookAt(const Gsage::Vector3& position) = 0;
      /**
       * Get current position
       */
      virtual const Gsage::Vector3 getPosition() = 0;
      /**
       * Get current scale
       */
      virtual const Gsage::Vector3 getScale() = 0;
      /**
       * Get current direction vector, uses orientationVector from config to detect front of the object
       */
      virtual const Gsage::Vector3 getDirection() = 0;
      /**
       * Get object orientation
       */
      virtual const Gsage::Quaternion getOrientation() = 0;
      /**
       * Get object orientation with respect to the direction vector
       */
      virtual const Gsage::Quaternion getFaceOrientation() = 0;
      /**
       * Adjusts animation speed for state
       * @param name Animation state name
       * @param speed animation speed
       */
      virtual bool adjustAnimationStateSpeed(const std::string& name, double speed) = 0;
      /**
       * Sets animation state
       * @param name Animation name
       * @returns true if state was found
       */
      virtual bool setAnimationState(const std::string& name) = 0;
      /**
       * Plays animation
       *
       * @param name Animation group name
       * @param times Repeat times, -1 means loop
       * @param speed Animation speed, 1 -- is normal speed
       * @param offset Animation start offset
       * @param reset Starts animation immediately
       *
       * @returns true if animation group exists
       */
      virtual bool playAnimation(const std::string& name, int times = -1, double speed = 1, double offset = 0, bool reset = false) = 0;
      /**
       * Resets animation state to default
       */
      virtual void resetAnimationState() = 0;
  };
}

#endif
