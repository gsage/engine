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
    if(mRootNode) 
      mRootNode->setPosition(position);
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

  void RenderComponent::lookAt(const Ogre::Vector3& position)
  {
    if(mRootNode) 
      mRootNode->lookAt(position * Ogre::Vector3(1, 0, 1) + (mRootNode->getPositionWithoutOffset() * Ogre::Vector3::UNIT_Y), Ogre::Node::TS_WORLD);
  }

  Dictionary RenderComponent::getAnimations()
  {
    Dictionary value;
    mAnimationScheduler.dump(value);
    return value;
  }

  Dictionary RenderComponent::getRootNode()
  {
    Dictionary value;
    if(mRootNode && mRootNode->hasNode())
      mRootNode->dump(value);

    return value;
  }

  SceneNodeWrapper* RenderComponent::getRoot()
  {
    return mRootNode;
  }

  const Ogre::Vector3 RenderComponent::getPosition()
  {
    return mRootNode->getPosition();
  }

  const Ogre::Vector3 RenderComponent::getScale()
  {
    return mRootNode->getScale();
  }

  const Ogre::Quaternion RenderComponent::getOrientation()
  {
    return mRootNode->getOrientation();
  }

  const Ogre::Quaternion RenderComponent::getFaceOrientation()
  {
    return (getOrientation() * Ogre::Vector3::NEGATIVE_UNIT_Z).getRotationTo(mRootNode->getOrientationVector());
  }

  const Ogre::Vector3 RenderComponent::getDirection()
  {
    return getOrientation() * mRootNode->getOrientationVector();
  }

  bool RenderComponent::adjustAnimationStateSpeed(const std::string& name, const float& speed)
  {
    return mAnimationScheduler.adjustSpeed(name, speed);
  }

  bool RenderComponent::setAnimationState(const std::string& name)
  {
    return mAnimationScheduler.play(name);
  }

  bool RenderComponent::playAnimation(const std::string& name, const float& times, const float& speed, const float& offset, bool reset)
  {
    return mAnimationScheduler.play(name, times, speed, offset, reset);
  }

  void RenderComponent::resetAnimationState()
  {
    mAnimationScheduler.resetState();
  }


  void RenderComponent::setResources(const Dictionary& dict)
  {
    mResources = dict;
  }

  const Dictionary& RenderComponent::getResources() const
  {
    return mResources;
  }
}
