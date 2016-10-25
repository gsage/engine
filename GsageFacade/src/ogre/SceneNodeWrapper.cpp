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

#include "ogre/SceneNodeWrapper.h"
#include "ogre/OgreObjectManager.h"
#include <OgreRoot.h>
#include "Logger.h"

namespace Gsage {

  const std::string SceneNodeWrapper::TYPE = "node";

  SceneNodeWrapper::SceneNodeWrapper() :
    mNode(0),
    mOffset(Ogre::Vector3::ZERO),
    mOrientationVector(Ogre::Vector3::UNIT_Z)
  {
    BIND_PROPERTY("offset", &mOffset);

    BIND_ACCESSOR("orientationVector", &SceneNodeWrapper::setOrientationVector, &SceneNodeWrapper::getOrientationVector);
    BIND_ACCESSOR("name", &SceneNodeWrapper::createNode, &SceneNodeWrapper::getId);
    BIND_ACCESSOR("position", &SceneNodeWrapper::setPosition, &SceneNodeWrapper::getPosition);
    BIND_ACCESSOR("scale", &SceneNodeWrapper::setScale, &SceneNodeWrapper::getScale);
    BIND_ACCESSOR("rotation", &SceneNodeWrapper::setOrientation, &SceneNodeWrapper::getOrientation);
    BIND_ACCESSOR("children", &SceneNodeWrapper::readChildren, &SceneNodeWrapper::writeChildren);
  }

  SceneNodeWrapper::~SceneNodeWrapper()
  {
  }

  bool SceneNodeWrapper::initialize(
          OgreObjectManager* objectManager,
          const Dictionary& dict,
          const std::string& ownerId,
          const std::string& type,
          Ogre::SceneManager* sceneManager,
          Ogre::SceneNode* parent)
  {
    if(dict.count("name") == 0)
    {
      mObjectId = ownerId;
      mNode = parent->createChildSceneNode(mObjectId);
    }
    addEventListener(objectManager, OgreObjectManagerEvent::FACTORY_UNREGISTERED, &SceneNodeWrapper::onFactoryUnregister);
    return OgreObject::initialize(objectManager, dict, ownerId, type, sceneManager, parent);
  }

  bool SceneNodeWrapper::hasNode()
  {
    return mNode != 0;
  }

  void SceneNodeWrapper::createNode(const std::string& id)
  {
    if (mParentNode == 0) {
      LOG(ERROR) << "Failed to create node \"" << mObjectId << "\" parent node is not set";
      return;
    }
    mNode = mParentNode->createChildSceneNode(mObjectId);
  }

  const std::string& SceneNodeWrapper::getId() const
  {
    return mObjectId;
  }

  void SceneNodeWrapper::setPosition(const Ogre::Vector3& position)
  {
    assert(mNode != 0);
    mNode->setPosition(position + (mOffset * getScale()));
  }

  Ogre::Vector3 SceneNodeWrapper::getPosition()
  {
    return getPositionWithoutOffset() - (mOffset * getScale());
  }

  Ogre::Vector3 SceneNodeWrapper::getPositionWithoutOffset()
  {
    return mNode != 0 ? mNode->getPosition() : Ogre::Vector3::ZERO;
  }

  void SceneNodeWrapper::setScale(const Ogre::Vector3& scale)
  {
    mNode->setScale(scale);
  }

  Ogre::Vector3 SceneNodeWrapper::getScale()
  {
    return mNode != 0 ? mNode->getScale() : Ogre::Vector3::ZERO;
  }

  void SceneNodeWrapper::setOrientation(const Ogre::Quaternion& rotation)
  {
    mNode->setOrientation(rotation);
  }

  Ogre::Quaternion SceneNodeWrapper::getOrientation()
  {
    return mNode != 0 ? mNode->getOrientation() : Ogre::Quaternion();
  }

  void SceneNodeWrapper::readChildren(const Dictionary& dict)
  {
    for(auto& pair : dict)
    {
      LOG(INFO) << "Creating child " << pair.second.get("type", "unknown");
      OgreObject* child = mObjectManager->create(pair.second, mOwnerId, mSceneManager, mNode);
      if(!child)
        continue;
      const std::string& type = child->getType();
      if(mChildren.count(type) == 0)
      {
        mChildren[type] = ObjectCollection();
      }

      const std::string& id = child->getObjectId();
      if(mChildren[type].count(id) != 0)
      {
        LOG(ERROR) << "Failed to create child of type \"" << type << "\", another object with id \"" << id << "\" already exists";
        child->destroy();
        continue;
      }
      mChildren[type][id] = child;
    }
  }

  Dictionary SceneNodeWrapper::writeChildren()
  {
    Dictionary values;
    for(auto& pair : mChildren)
    {
      for(auto& childPair : pair.second)
      {
        Dictionary item;
        childPair.second->dump(item);
        values.push(item);
      }
    }
    return values;
  }

  void SceneNodeWrapper::setOrientationVector(const Ogre::Vector3& value)
  {
    mOrientationVector = value.normalisedCopy();
  }

  const Ogre::Vector3& SceneNodeWrapper::getOrientationVector() const
  {
    return mOrientationVector;
  }

  void SceneNodeWrapper::rotate(const Ogre::Quaternion& rotation, const Ogre::Node::TransformSpace ts)
  {
    mNode->rotate(rotation, ts);
  }

  void SceneNodeWrapper::lookAt(const Ogre::Vector3& position, Ogre::Node::TransformSpace relativeTo)
  {
    mNode->lookAt(position, relativeTo, mOrientationVector);
  }

  void SceneNodeWrapper::destroy()
  {
    removeEventListener(mObjectManager, OgreObjectManagerEvent::FACTORY_UNREGISTERED, &SceneNodeWrapper::onFactoryUnregister);
    for(auto& subscription : mConnections)
    {
      LOG(INFO) << subscription.first.second << ", om: " << mObjectManager << ", sp: " << subscription.first.first;
    }
    for(auto& pair : mChildren)
    {
      for(auto& objectPair : pair.second)
      {
        objectPair.second->destroy();
      }
    }

    if(mParentNode != 0)
      mParentNode->removeChild(mNode);

    if(mNode != 0)
    {
      destroyAllAttachedMovableObjects(mNode);
      mNode->removeAndDestroyAllChildren();
      mSceneManager->destroySceneNode(mNode);
    }

  }

  void SceneNodeWrapper::destroyAllAttachedMovableObjects( Ogre::SceneNode* node )
  {
    if(!node) return;

    // Destroy all the attached objects
    Ogre::SceneNode::ObjectIterator itObject = node->getAttachedObjectIterator();

    while ( itObject.hasMoreElements() )
      node->getCreator()->destroyMovableObject(itObject.getNext());

    // Recurse to child SceneNode
    Ogre::SceneNode::ChildNodeIterator itChild = node->getChildIterator();

    while ( itChild.hasMoreElements() )
    {
      Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
      destroyAllAttachedMovableObjects( pChildNode );
    }
  }

  OgreObject* SceneNodeWrapper::getChild(const std::string& type, const std::string& name)
  {
    if(mChildren.count(type) == 0 || mChildren[type].count(name) == 0)
      return 0;

    return mChildren[type][name];
  }

  bool SceneNodeWrapper::onFactoryUnregister(EventDispatcher* sender, const Event& event)
  {
    const OgreObjectManagerEvent& e = static_cast<const OgreObjectManagerEvent&>(event);
    if(mChildren.count(e.getId()) == 0)
      return true;
    mChildren.erase(e.getId());
    return true;
  }
}
