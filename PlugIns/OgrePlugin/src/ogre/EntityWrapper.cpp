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
#include "FileLoader.h"
#include "systems/OgreRenderSystem.h"
#include "MaterialLoader.h"

#include <OgreMeshManager.h>
#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
#include <OgreSubMesh.h>

#if OGRE_VERSION_MAJOR == 2
#include <OgreOldSkeletonInstance.h>
#endif

namespace Gsage {

  const std::string EntityWrapper::TYPE = "model";

  EntityWrapper::EntityWrapper()
    : mQuery(STATIC)
    , mAnimBlendMode(OgreV1::ANIMBLEND_CUMULATIVE)
    , mResourceGroup(Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME)
  {
    BIND_ACCESSOR_WITH_PRIORITY("resourceGroup", &EntityWrapper::setResourceGroup, &EntityWrapper::getResourceGroup, 2);
    BIND_ACCESSOR_WITH_PRIORITY("mesh", &EntityWrapper::setMesh, &EntityWrapper::getMesh, 1);

    BIND_ACCESSOR("query", &EntityWrapper::setQueryFlags, &EntityWrapper::getQueryFlags);
    BIND_ACCESSOR("castShadows", &EntityWrapper::setCastShadows, &EntityWrapper::getCastShadows);
    BIND_ACCESSOR("renderQueue", &EntityWrapper::setRenderQueue, &EntityWrapper::getRenderQueue);
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

    if(mObject != 0)
    {
      mObject->setQueryFlags(mQuery);
    }
  }

  const std::string& EntityWrapper::getQueryFlags() const
  {
    return mQueryString;
  }

  void EntityWrapper::setResourceGroup(const std::string& name)
  {
    mResourceGroup = name;
  }

  const std::string& EntityWrapper::getResourceGroup() const
  {
    return mResourceGroup;
  }

  void EntityWrapper::setMesh(const std::string& model)
  {
    if(mMeshName == model) {
      return;
    }

    if(mObject) {
      for(auto entity : mAttachedEntities) {
        entity->destroy();
      }
      mAttachedEntities.clear();
      mObject->detachFromParent();
      mSceneManager->destroyEntity(mObject);
      mObject = 0;
    }

    mMeshName = model;

    OgreV1::MeshPtr mesh = OgreV1::MeshManager::getSingleton().load(
      model, mResourceGroup,
      OgreV1::HardwareBuffer::HBU_STATIC, OgreV1::HardwareBuffer::HBU_STATIC);
    for(int i = 0; i < mesh->getNumSubMeshes(); i++) {
      OgreV1::SubMesh* sm = mesh->getSubMesh(i);
      if(sm->isMatInitialised()) {
        Ogre::String materialName = sm->getMaterialName();
//#if OGRE_VERSION >= 0x020100
        if(!mObjectManager->getRenderSystem()->getMaterialLoader()->load(materialName, mesh->getGroup())) {
          LOG(ERROR) << "Failed to load material " << materialName;
        }
/*#else
        Ogre::MaterialManager::getSingletonPtr()->load(
            materialName,
            mesh->getGroup()
        );
#endif*/
      }
    }

    mObject = mSceneManager->createEntity(mesh);
    mObject->setQueryFlags(mQuery);
    mObject->getUserObjectBindings().setUserAny("entity", Ogre::Any(mOwnerId));
    auto skeleton = static_cast<OgreV1::Entity*>(mObject)->getSkeleton();
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

  void EntityWrapper::setRenderQueue(const unsigned int& queue)
  {
    if(!mObject)
      return;

    mObject->setRenderQueueGroup(queue & 0xFF);
  }

  unsigned int EntityWrapper::getRenderQueue()
  {
    return mObject ? mObject->getRenderQueueGroup() : 0;
  }

  bool EntityWrapper::attach(Ogre::MovableObject* object, const DataProxy& params)
  {
    std::string boneID = params.get("boneID", "");
    if(boneID.empty()) {
      return false;
    }

    mObject->attachObjectToBone(boneID, object);

    return true;
  }

  void EntityWrapper::attachToBone(const DataProxy& params, const std::string& entityId, DataProxy movableObjectData)
  {
    if(!mObject)
      return;

    OgreObject* object = mObjectManager->create(movableObjectData, mOwnerId, mSceneManager, movableObjectData.get("type", ""), params, this);
    mAttachedEntities.push_back(object);
  }
}
