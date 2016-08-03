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

#include "ogre/EntityWrapper.h"
#include "ogre/OgreObjectManager.h"

#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>

namespace Gsage {

  const std::string EntityWrapper::TYPE = "model";

  EntityWrapper::EntityWrapper()
    : mQuery(STATIC)
    , mAnimBlendMode(Ogre::ANIMBLEND_CUMULATIVE)
  {
    BIND_ACCESSOR("query", &EntityWrapper::setQueryFlags, &EntityWrapper::getQueryFlags);
    BIND_ACCESSOR("mesh", &EntityWrapper::setMesh, &EntityWrapper::getMesh);
    BIND_ACCESSOR("castShadows", &EntityWrapper::setCastShadows, &EntityWrapper::getCastShadows);
  }

  EntityWrapper::~EntityWrapper()
  {
    for(auto entity : mAttachedEntities) {
      entity->destroy();
    }
    mAttachedEntities.clear();
  }

  void EntityWrapper::setQueryFlags(const std::string& type)
  {
    mQueryString = type;
    if(type == "static")
      mQuery = STATIC;
    else if(type == "dynamic")
      mQuery = DYNAMIC;
    else
      mQuery = UNKNOWN;
  }

  const std::string& EntityWrapper::getQueryFlags() const
  {
    return mQueryString;
  }

  void EntityWrapper::setMesh(const std::string& model)
  {
    mMeshName = model;
    mObject = mSceneManager->createEntity(mObjectId, model);
    mObject->setQueryFlags(mQuery);
    mObject->getUserObjectBindings().setUserAny("entity", Ogre::Any(mOwnerId));
    Ogre::Skeleton* skeleton = mObject->getSkeleton();
    if(skeleton != 0)
      skeleton->setBlendMode(mAnimBlendMode);
    attachObject(mObject);
  }

  const std::string& EntityWrapper::getMesh() const
  {
    return mMeshName;
  }

  void EntityWrapper::setCastShadows(const bool& value)
  {
    if(!mObject)
      return;

    mObject->setCastShadows(value);
  }

  bool EntityWrapper::getCastShadows()
  {
    return mObject ? mObject->getCastShadows() : false;
  }

  void EntityWrapper::attachToBone(const std::string& boneId, const std::string& entityId, DataNode movableObjectData)
  {
    if(!mObject)
      return;

    OgreObject* object = mObjectManager->create(movableObjectData, mOwnerId, mSceneManager, boneId, mObject);
    mAttachedEntities.push_back(object);
  }
}
