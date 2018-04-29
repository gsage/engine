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

#include "components/MovementComponent.h"

namespace Gsage {

  const std::string MovementComponent::SYSTEM = "movement";

  MovementComponent::MovementComponent()
    : mPath(nullptr)
    , mSurface(nullptr)
    , mSpeed(0.0f)
    , mLastMovementDistance(0.0f)
    , mAnimSpeedRatio(1.0f)
    , mAlign(Gsage::Vector3(1, 0, 1))
  {
    BIND_PROPERTY_OPTIONAL("speed", &mSpeed);
    BIND_PROPERTY_OPTIONAL("moveAnimation", &mMoveAnimationState);
    BIND_PROPERTY_OPTIONAL("animSpeedRatio", &mAnimSpeedRatio);
    BIND_PROPERTY_OPTIONAL("align", &mAlign);
  }

  MovementComponent::~MovementComponent()
  {
  }

  bool MovementComponent::reachedDestination()
  {
    if(mPath != nullptr && mPath->current() == nullptr) {
      mPath = nullptr;
      return true;
    }

    return false;
  }

  bool MovementComponent::nextPoint()
  {
    if(mPath == nullptr) {
      return false;
    }

    return mPath->next();
  }

  Gsage::Vector3* MovementComponent::currentTarget() const
  {
    if(mPath == nullptr) {
      return nullptr;
    }

    return mPath->current();
  }

  Path3D* MovementComponent::getPath()
  {
    return mPath.get();
  }

  void MovementComponent::resetPath()
  {
    if(mPath) {
      // set empty path to make animation reset properly
      mPath->update(Path3D::Vector());
    }
  }

  void MovementComponent::move(Path3DPtr path, const DataProxy& config)
  {
    mSurface = nullptr;
    mPath = path;
  }

  void MovementComponent::move(Path3DPtr path)
  {
    move(path, DataProxy::create(DataWrapper::JSON_OBJECT));
  }

  void MovementComponent::move(Path3DPtr path, SurfacePtr surface)
  {
    move(path, DataProxy::create(DataWrapper::JSON_OBJECT));
    mSurface = surface;
  }

  Gsage::Vector3* MovementComponent::getNextPoint(const Gsage::Vector3& currentPosition, double time)
  {
    Gsage::Vector3* target = currentTarget();
    if(target == nullptr) {
      return nullptr;
    }

    float speed = mSpeed * time;
    Gsage::Vector3 delta = *target - currentPosition;

    if(Gsage::Vector3::Magnitude(delta) < speed) {
      mNextPosition = *target;
      nextPoint();
    } else {
      mNextPosition = currentPosition + Vector3::Normalized(delta) * speed;
    }

    mDelta = delta;

    if(mSurface) {
      mNextPosition.Y = mSurface->getHeight(mNextPosition);
    }

    return &mNextPosition;
  }

  Gsage::Quaternion* MovementComponent::getRotation(const Gsage::Vector3& currentDirection, double time)
  {
    if(Vector3::Magnitude(mDelta) == 0) {
      return nullptr;
    }
    Gsage::Vector3 direction = currentDirection;
    direction *= mAlign;
    mDelta *= mAlign;
    mNextOrientation = Gsage::Quaternion::FromToRotation(direction, mDelta);
    return &mNextOrientation;
  }

  void MovementComponent::go(const Gsage::Vector3& target)
  {
    Path3D::Vector points = {target};
    move(std::make_shared<Path3D>(points));
  }

  void MovementComponent::go(float x, float y, float z)
  {
    Gsage::Vector3 dest(x, y, z);
    go(dest);
  }

}
