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

#include "ogre/OgreObject.h"
#include "ogre/OgreObjectManager.h"
#include "Logger.h"

#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include <OgreSceneManager.h>

namespace Gsage {

  static long counter = 0;

  OgreObject::OgreObject()
    : mParentNode(0)
    , mParentEntity(0)
    , mObjectManager(0)
    , mSceneManager(0)
  {
    // setting couple of these props priorities to maximum
    BIND_PROPERTY_WITH_PRIORITY("type", &mType, 10000);
    BIND_PROPERTY_WITH_PRIORITY("name", &mObjectId, 10000);
  }

  OgreObject::~OgreObject()
  {
  }

  bool OgreObject::initialize(
          OgreObjectManager* objectManager,
          const DataProxy& dict,
          const std::string& ownerId,
          const std::string& type,
          Ogre::SceneManager* sceneManager,
          const std::string& boneId,
          Ogre::Entity* parentEntity)
  {
    mBoneId = boneId;
    mParentEntity = parentEntity;
    return initialize(objectManager, dict, ownerId, type, sceneManager, 0);
  }

  bool OgreObject::initialize(
          OgreObjectManager* objectManager,
          const DataProxy& dict,
          const std::string& ownerId,
          const std::string& type,
          Ogre::SceneManager* sceneManager,
          Ogre::SceneNode* parent)
  {
    mType = type;
    mOwnerId = ownerId;
    mObjectManager = objectManager;
    mSceneManager = sceneManager;
    mParentNode = parent;

    if(sceneManager == 0)
    {
      LOG(ERROR) << "Failed to create ogre object, sceneManager is not initialized";
      return false;
    }

    auto objectId = dict.get<std::string>("name");
    if (!objectId.second && mObjectId.empty()) {
      std::stringstream ss("");
      ss << mType << counter++;
      mObjectId = ss.str();
    }

    read(dict);
    return true;
  }

  void OgreObject::destroy()
  {
    mObjectManager->destroy(this);
  }

  const std::string& OgreObject::getType() const
  {
    return mType;
  }

  const std::string& OgreObject::getObjectId() const
  {
    return mObjectId;
  }

  void OgreObject::attachObject(Ogre::MovableObject* object)
  {
    if(mParentNode != 0)
      mParentNode->attachObject(object);
    else if(!mBoneId.empty() && mParentEntity != 0)
      mParentEntity->attachObjectToBone(mBoneId, object);
    else
      LOG(ERROR) << "Failed to attach movable object, no attach configuration found";
  }

  std::string OgreObject::generateName() const
  {
    if(mParentNode == mSceneManager->getRootSceneNode()) {
      return mObjectId;
    } else if(mParentEntity) {
      return mParentEntity->getName() + "." + mObjectId;
    }
    return mParentNode->getName() + "." + mObjectId;
  }
}
