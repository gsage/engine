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

#ifndef _MovementComponent_H_
#define _MovementComponent_H_

#include <OgreVector3.h>

#include "Component.h"

namespace Gsage {

  class RecastMovementSystem;

  /**
   * Recast movement component
   */

  class MovementComponent : public EntityComponent
  {
    public:
      static const std::string SYSTEM;

      typedef std::vector<Ogre::Vector3> Path;

      MovementComponent();
      virtual ~MovementComponent();
      /**
       * Set entity destination
       * @param x X coordinate
       * @param y Y coordinate
       * @param z Z coordinate
       */
      void setTarget(const float& x, const float& y, const float& z);
      /**
       * Set entity destination
       * @param position Vector3 position of target
       */
      void setTarget(const Ogre::Vector3& position);

      /**
       * Get final destination the path
       */
      const Ogre::Vector3& getFinalTarget();
      /**
       * Get current intermediate point of the path
       */
      const Ogre::Vector3& getCurrentTarget();
      /**
       * Final target is set
       */
      bool hasTarget();
      /**
       * Path was set
       */
      bool hasPath();
      /**
       * Reset path, current path point and target
       */
      void resetTarget();
      /**
       * Set point sequence for moving
       */
      void setPath(const Path& path);
      /**
       * Changes current intermediate point
       * @returns True if there is more points
       */
      bool nextPoint();
      /**
       * Get last movement distance
       */
      const float& getLastMovementDistance();
      /**
       * Set component movement speed
       *
       * @param value Speed in unit per second
       */
      void setSpeed(const double& value) { mSpeed = value; }
      /**
       * Get component movement speed
       */
      const double& getSpeed() const { return mSpeed; }
    private:
      friend class RecastMovementSystem;
      /**
       * Set last movement distance
       */
      void setLastMovementDistance(const float& distance);

      double mSpeed;
      std::string mMoveAnimationState;
      float mAnimSpeedRatio;

      bool mAligned;
      bool mHasTarget;
      bool mTargetReset;

      float mLastMovementDistance;

      int mAgentId;
      int mCurrentPoint;

      Ogre::Vector3 mTarget;

      Path mPath;
  };
}

#endif
