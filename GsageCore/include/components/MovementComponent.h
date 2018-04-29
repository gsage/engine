#ifndef _MovementComponent_H_
#define _MovementComponent_H_

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

#include "Component.h"
#include "Path.h"

namespace Gsage {
  class MovementSystem;

  /**
   * Interface that represents movement surface
   */
  class GSAGE_API Surface
  {
    public:
      /**
       * Get surface height at position
       *
       * @param position
       *
       * @return height
       */
      virtual float getHeight(const Gsage::Vector3& position) = 0;
  };

  typedef std::shared_ptr<Surface> SurfacePtr;

  class GSAGE_API MovementComponent : public EntityComponent
  {
    public:
      static const std::string SYSTEM;

      MovementComponent();
      virtual ~MovementComponent();

      /**
       * Shift to next point
       */
      bool nextPoint();

      /**
       * Get current path point
       */
      Gsage::Vector3* currentTarget() const;

      /**
       * Check if movement reached destination
       */
      bool reachedDestination();

      /**
       * Get current path
       *
       * @return current path
       */
      Path3D* getPath();

      /**
       * Reset current path. This will stop the movement.
       */
      void resetPath();

      /**
       * Move using path
       *
       * @param path Path to use for movement
       * @param props Follow properties
       */
      void move(Path3DPtr path, const DataProxy& config);

      /**
       * Move using path
       *
       * @param path Path to use for movement
       */
      void move(Path3DPtr path);

      /**
       * Move using path along surface
       *
       * @param path Path to use for movement
       * @param surface Surface
       */
      void move(Path3DPtr path, SurfacePtr surface);

      /**
       * Generate path and go to point
       */
      void go(const Gsage::Vector3& point);

      /**
       * Generate path and go to point
       */
      void go(float x, float y, float z);

      /**
       * Get speed
       */
      inline float getSpeed() { return mSpeed; }

      /**
       * Set speed
       */
      inline void setSpeed(float speed) { mSpeed = speed; }

      /**
       * Check if has target
       */
      inline bool hasTarget() { return mPath != nullptr; }

      /**
       * Get last point of the path
       */
      inline Gsage::Vector3 getFinalTarget() { return mPath && mPath->end() ? *mPath->end() : Gsage::Vector3::Zero(); }

      /**
       * Get move animation state
       */
      inline const std::string& getMoveAnimationState() { return mMoveAnimationState; }

      /**
       * Get move animation playback speed/movement speed ratio
       */
      inline float getAnimSpeedRatio() { return mAnimSpeedRatio; }
    private:
      friend class MovementSystem;
      /**
       * Get next point using currently defined path, current target and speed
       *
       * @param time Time multiplier
       */
      Gsage::Vector3* getNextPoint(const Gsage::Vector3& currentPosition, double time);

      /**
       * Get next orientation using latest movement delta
       *
       * @param time Time multiplier
       */
      Gsage::Quaternion* getRotation(const Gsage::Vector3& currentDirection, double time);

      Path3DPtr mPath;

      // current path movement options
      float mSpeed;
      float mLastMovementDistance;
      float mAnimSpeedRatio;

      std::string mMoveAnimationState;

      Gsage::Vector3 mAlign;
      Gsage::Vector3 mNextPosition;
      Gsage::Vector3 mDelta;

      Gsage::Quaternion mNextOrientation;

      SurfacePtr mSurface;
  };
}

#endif
