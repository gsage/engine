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

#include "components/RenderComponent.h"
#include "ogre/SceneNodeWrapper.h"

#include <string>
#include <OgreVector3.h>

namespace Gsage {

  const std::string RenderComponent::SYSTEM = "render";
  /**
   * Fired on render component position change
   */
  const std::string RenderComponent::POSITION_CHANGE = "RenderComponent.POSITION_CHANGE";

  RenderComponent::RenderComponent() :
    mAddedToScene(false),
    mRootNode(0)
  {
    BIND_ACCESSOR_OPTIONAL("resources", &RenderComponent::setResources, &RenderComponent::getResources);
    BIND_GETTER("root", &RenderComponent::getRootNode);
    BIND_GETTER("animations", &RenderComponent::getAnimations);
  }

  RenderComponent::~RenderComponent()
  {
  }

  void RenderComponent::setPosition(const Ogre::Vector3& position)
  {
    if(mRootNode) {
      mRootNode->setPosition(position);
      fireEvent(Event(RenderComponent::POSITION_CHANGE));
    }
  }

  void RenderComponent::setOrientation(const Ogre::Quaternion& orientation)
  {
    if(mRootNode)
      mRootNode->setOrientation(orientation);
  }

  void RenderComponent::rotate(const Ogre::Quaternion& rotation)
  {
    if(mRootNode)
      mRootNode->rotate(rotation, Ogre::Node::TransformSpace::TS_LOCAL);
  }

  void RenderComponent::lookAt(const Ogre::Vector3& position, const Geometry::RotationAxis rotationAxis, Geometry::TransformSpace transformSpace)
  {
    if(!mRootNode)
      return;

    Ogre::Vector3 axis = Ogre::Vector3::ZERO;
    Ogre::Node::TransformSpace tSpace;

    switch(transformSpace) {
      case Geometry::TS_LOCAL:
        tSpace = Ogre::Node::TS_LOCAL;
        break;
      case Geometry::TS_WORLD:
        tSpace = Ogre::Node::TS_WORLD;
        break;
      default:
        tSpace = Ogre::Node::TS_WORLD;
    }

    switch(rotationAxis) {
      case Geometry::X_AXIS:
        axis.x = 1;
        break;
      case Geometry::Y_AXIS:
        axis.y = 1;
        break;
      case Geometry::Z_AXIS:
        axis.z = 1;
        break;
      default:
        mRootNode->lookAt(position, tSpace);
        return;
    }

    mRootNode->lookAt(position * (Ogre::Vector3::UNIT_SCALE - axis) + (mRootNode->getPositionWithoutOffset() * axis), tSpace);
  }

  void RenderComponent::lookAt(const Ogre::Vector3& position)
  {
    lookAt(position, Geometry::NONE);
  }

  DataProxy RenderComponent::getAnimations()
  {
    DataProxy value;
    mAnimationScheduler.dump(value);
    return value;
  }

  DataProxy RenderComponent::getRootNode()
  {
    DataProxy value;
    if(mRootNode && mRootNode->hasNode())
      mRootNode->dump(value);

    return value;
  }

  SceneNodeWrapper* RenderComponent::getRoot()
  {
    return mRootNode;
  }

  const Ogre::Vector3 RenderComponent::getOgrePosition()
  {
    return mRootNode->getPosition();
  }

  const Ogre::Vector3 RenderComponent::getOgreScale()
  {
    return mRootNode->getScale();
  }

  const Ogre::Quaternion RenderComponent::getOgreOrientation()
  {
    return mRootNode->getOrientation();
  }

  const Ogre::Quaternion RenderComponent::getOgreFaceOrientation()
  {
    return (getOgreOrientation() * Ogre::Vector3::NEGATIVE_UNIT_Z).getRotationTo(mRootNode->getOrientationVector());
  }

  const Ogre::Vector3 RenderComponent::getOgreDirection()
  {
    return getOgreOrientation() * mRootNode->getOrientationVector();
  }

  bool RenderComponent::adjustAnimationStateSpeed(const std::string& name, double speed)
  {
    return mAnimationScheduler.adjustSpeed(name, speed);
  }

  bool RenderComponent::setAnimationState(const std::string& name)
  {
    return mAnimationScheduler.play(name);
  }

  bool RenderComponent::playAnimation(const std::string& name, int times, double speed, double offset, bool reset)
  {
    return mAnimationScheduler.play(name, times, speed, offset, reset);
  }

  void RenderComponent::resetAnimationState()
  {
    mAnimationScheduler.resetState();
  }


  void RenderComponent::setResources(const DataProxy& dict)
  {
    mResources = dict;
  }

  const DataProxy& RenderComponent::getResources() const
  {
    return mResources;
  }

  void RenderComponent::setPosition(const Gsage::Vector3& position)
  {
    setPosition(GsageVector3ToOgreVector3(position));
  }

  void RenderComponent::setOrientation(const Gsage::Quaternion& orientation)
  {
    setOrientation(GsageQuaternionToOgreQuaternion(orientation));
  }

  void RenderComponent::rotate(const Gsage::Quaternion& rotation)
  {
    rotate(GsageQuaternionToOgreQuaternion(rotation));
  }

  void RenderComponent::lookAt(const Gsage::Vector3& position, const Geometry::RotationAxis rotationAxis, Geometry::TransformSpace transformSpace)
  {
    lookAt(GsageVector3ToOgreVector3(position), rotationAxis, transformSpace);
  }

  void RenderComponent::lookAt(const Gsage::Vector3& position)
  {
    lookAt(GsageVector3ToOgreVector3(position));
  }

  const Gsage::Vector3 RenderComponent::getPosition()
  {
    return OgreVector3ToGsageVector3(getOgrePosition());
  }

  const Gsage::Vector3 RenderComponent::getScale()
  {
    return OgreVector3ToGsageVector3(getOgreScale());
  }

  const Gsage::Quaternion RenderComponent::getOrientation()
  {
    return OgreQuaternionToGsageQuaternion(getOgreOrientation());
  }

  const Gsage::Quaternion RenderComponent::getFaceOrientation()
  {
    return OgreQuaternionToGsageQuaternion(getOgreFaceOrientation());
  }

  const Gsage::Vector3 RenderComponent::getDirection()
  {
    return OgreVector3ToGsageVector3(getOgreDirection());
  }
}
