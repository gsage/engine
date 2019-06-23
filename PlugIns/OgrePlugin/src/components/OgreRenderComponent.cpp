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

#include "components/OgreRenderComponent.h"
#include "ogre/SceneNodeWrapper.h"
#include "ogre/OgreObjectManager.h"
#include "ResourceManager.h"
#include "Entity.h"

#include <string>
#include <OgreVector3.h>
#include <OgreSceneManager.h>

namespace Gsage {

  const std::string OgreRenderComponent::SYSTEM = "render";
  /**
   * Fired on render component position change
   */
  const Event::Type OgreRenderComponent::POSITION_CHANGE = "OgreRenderComponent.POSITION_CHANGE";

  OgreRenderComponent::OgreRenderComponent() :
    mAddedToScene(false),
    mRootNode(0),
    mSceneManager(0),
    mResourceManager(0),
    mObjectManager(0)
  {
    BIND_ACCESSOR_OPTIONAL("resources", &OgreRenderComponent::setResources, &OgreRenderComponent::getResources);
    BIND_ACCESSOR("root", &OgreRenderComponent::setRootNode, &OgreRenderComponent::getRootNode);
    BIND_ACCESSOR_OPTIONAL("animations", &OgreRenderComponent::setAnimations, &OgreRenderComponent::getAnimations);
  }

  OgreRenderComponent::~OgreRenderComponent()
  {
  }

  void OgreRenderComponent::setPosition(const Ogre::Vector3& position)
  {
    if(mRootNode) {
      mRootNode->setPosition(position);
      fireEvent(Event(OgreRenderComponent::POSITION_CHANGE));
    }
  }

  void OgreRenderComponent::setOrientation(const Ogre::Quaternion& orientation)
  {
    if(mRootNode)
      mRootNode->setOrientation(orientation);
  }

  void OgreRenderComponent::rotate(const Ogre::Quaternion& rotation)
  {
    if(mRootNode)
      mRootNode->rotate(rotation, Ogre::Node::TransformSpace::TS_LOCAL);
  }

  void OgreRenderComponent::lookAt(const Ogre::Vector3& position, const Geometry::RotationAxis rotationAxis, Geometry::TransformSpace transformSpace)
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

  void OgreRenderComponent::lookAt(const Ogre::Vector3& position)
  {
    lookAt(position, Geometry::NONE);
  }

  DataProxy OgreRenderComponent::getAnimations()
  {
    DataProxy value;
    mAnimationScheduler.dump(value);
    return value;
  }

  void OgreRenderComponent::setAnimations(const DataProxy& value)
  {
    if(!mSceneManager) {
      LOG(ERROR) << "Failed to set up animations: scene manager is not defined";
      return;
    }
    mAnimationScheduler.initialize(value, mSceneManager, this);
  }

  DataProxy OgreRenderComponent::getRootNode()
  {
    DataProxy value;
    if(mRootNode && mRootNode->hasNode())
      mRootNode->dump(value);

    return value;
  }

  void OgreRenderComponent::setRootNode(const DataProxy& value)
  {
    if(!mRootNode) {
      if(!mSceneManager || !mObjectManager) {
        LOG(ERROR) << "Failed to set up node tree: scene manager or object manager is not set";
        return;
      }
      try {
        mRootNode = mObjectManager->create<SceneNodeWrapper>(value, getOwner()->getId(), mSceneManager, mSceneManager->getRootSceneNode());
      } catch(const Ogre::Exception& e) {
        LOG(ERROR) << "Failed to create render component " << e.what();
      }
    } else {
      try {
        mRootNode->read(value);
      } catch(const Ogre::Exception& e) {
        LOG(ERROR) << "Failed to create render component " << e.what();
      }
    }
  }

  SceneNodeWrapper* OgreRenderComponent::getRoot()
  {
    return mRootNode;
  }

  const Ogre::Vector3 OgreRenderComponent::getOgrePosition()
  {
    return mRootNode->getPosition();
  }

  const Ogre::Vector3 OgreRenderComponent::getOgreScale()
  {
    return mRootNode->getScale();
  }

  const Ogre::Quaternion OgreRenderComponent::getOgreOrientation()
  {
    return mRootNode->getOrientation();
  }

  const Ogre::Quaternion OgreRenderComponent::getOgreFaceOrientation()
  {
    return (getOgreOrientation() * Ogre::Vector3::NEGATIVE_UNIT_Z).getRotationTo(mRootNode->getOrientationVector());
  }

  const Ogre::Vector3 OgreRenderComponent::getOgreDirection()
  {
    return getOgreOrientation() * mRootNode->getOrientationVector();
  }

  bool OgreRenderComponent::adjustAnimationStateSpeed(const std::string& name, double speed)
  {
    return mAnimationScheduler.adjustSpeed(name, speed);
  }

  bool OgreRenderComponent::setAnimationState(const std::string& name)
  {
    return mAnimationScheduler.play(name);
  }

  bool OgreRenderComponent::playAnimation(const std::string& name, int times, double speed, double offset, bool reset)
  {
    return mAnimationScheduler.play(name, times, speed, offset, reset);
  }

  void OgreRenderComponent::resetAnimationState()
  {
    mAnimationScheduler.resetState();
  }


  void OgreRenderComponent::setResources(const DataProxy& dict)
  {
    mResources = dict;
    if(!mResourceManager || !mResourceManager->load(dict)) {
      LOG(ERROR) << "Failed to load resources for entity " << getOwner()->getId() << " " << SYSTEM << " component";
    }
  }

  const DataProxy& OgreRenderComponent::getResources() const
  {
    return mResources;
  }

  void OgreRenderComponent::setPosition(const Gsage::Vector3& position)
  {
    setPosition(GsageVector3ToOgreVector3(position));
  }

  void OgreRenderComponent::setOrientation(const Gsage::Quaternion& orientation)
  {
    setOrientation(GsageQuaternionToOgreQuaternion(orientation));
  }

  void OgreRenderComponent::rotate(const Gsage::Quaternion& rotation)
  {
    rotate(GsageQuaternionToOgreQuaternion(rotation));
  }

  void OgreRenderComponent::lookAt(const Gsage::Vector3& position, const Geometry::RotationAxis rotationAxis, Geometry::TransformSpace transformSpace)
  {
    lookAt(GsageVector3ToOgreVector3(position), rotationAxis, transformSpace);
  }

  void OgreRenderComponent::lookAt(const Gsage::Vector3& position)
  {
    lookAt(GsageVector3ToOgreVector3(position));
  }

  const Gsage::Vector3 OgreRenderComponent::getPosition()
  {
    return OgreVector3ToGsageVector3(getOgrePosition());
  }

  const Gsage::Vector3 OgreRenderComponent::getScale()
  {
    return OgreVector3ToGsageVector3(getOgreScale());
  }

  const Gsage::Quaternion OgreRenderComponent::getOrientation()
  {
    return OgreQuaternionToGsageQuaternion(getOgreOrientation());
  }

  const Gsage::Quaternion OgreRenderComponent::getFaceOrientation()
  {
    return OgreQuaternionToGsageQuaternion(getOgreFaceOrientation());
  }

  const Gsage::Vector3 OgreRenderComponent::getDirection()
  {
    return OgreVector3ToGsageVector3(getOgreDirection());
  }
}
