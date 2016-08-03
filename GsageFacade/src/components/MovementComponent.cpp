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

#include "components/MovementComponent.h"

namespace Gsage {

  const std::string MovementComponent::SYSTEM = "movement";

  MovementComponent::MovementComponent() :
    mAgentId(0),
    mCurrentPoint(-1),
    mHasTarget(false),
    mAligned(false),
    mTargetReset(false)
  {
    BIND_PROPERTY("speed", &mSpeed);
    BIND_PROPERTY("moveAnimation", &mMoveAnimationState);
    BIND_PROPERTY("animSpeedRatio", &mAnimSpeedRatio);
  }

  MovementComponent::~MovementComponent()
  {
  }

  void MovementComponent::setTarget(const float& x, const float& y, const float& z)
  {
    setTarget(Ogre::Vector3(x, y, z));
  }

  void MovementComponent::setTarget(const Ogre::Vector3& position)
  {
    resetTarget();
    mTarget = position;
    mHasTarget = true;
  }

  bool MovementComponent::hasTarget()
  {
    return mHasTarget;
  }

  bool MovementComponent::hasPath()
  {
    return mCurrentPoint != -1;
  }

  void MovementComponent::setPath(const MovementComponent::Path& path)
  {
    mCurrentPoint = std::min((size_t)1, path.size() - 1);
    mPath = path;
  }

  void MovementComponent::resetTarget()
  {
    mCurrentPoint = -1;
    mPath.clear();
    mHasTarget = false;
    mTargetReset = true;
  }

  const Ogre::Vector3& MovementComponent::getFinalTarget()
  {
    return mTarget;
  }

  const Ogre::Vector3& MovementComponent::getCurrentTarget()
  {
    return mPath[mCurrentPoint];
  }

  bool MovementComponent::nextPoint()
  {
    return ++mCurrentPoint < mPath.size();
  }

  const float& MovementComponent::getLastMovementDistance()
  {
    return mLastMovementDistance;
  }

  void MovementComponent::setLastMovementDistance(const float& value)
  {
    mLastMovementDistance = value;
  }
}
