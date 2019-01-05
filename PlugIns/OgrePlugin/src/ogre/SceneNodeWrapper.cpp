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
  static long counter = 0;

  const std::string SceneNodeWrapper::TYPE = "node";

  SceneNodeWrapper::SceneNodeWrapper() :
    mNode(0),
    mOffset(Ogre::Vector3::ZERO),
    mOrientationVector(Ogre::Vector3::UNIT_Z),
    mOrientation(Ogre::Quaternion::IDENTITY)
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
          const DataProxy& dict,
          const std::string& ownerId,
          const std::string& type,
          Ogre::SceneManager* sceneManager,
          Ogre::SceneNode* parent)
  {
    if(dict.count("name") == 0)
    {
      mObjectId = ownerId;
#if OGRE_VERSION_MAJOR == 2
      // TODO: static/dynamic nodes
      mNode = parent->createChildSceneNode();
#else
      mNode = parent->createChildSceneNode(mObjectId);
#endif
      mNode->setOrientation(mOrientation);
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
    mObjectId = id;
    std::string name = generateName();
#if OGRE_VERSION_MAJOR == 2
      // TODO: static/dynamic nodes
    mNode = mParentNode->createChildSceneNode();
#else
    mNode = mParentNode->createChildSceneNode(name);
#endif
    mNode->setOrientation(mOrientation);
    LOG(TRACE) << "Creating scene node \"" << name << "\"";
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

  void SceneNodeWrapper::setOrientation(const Ogre::Quaternion& orientation)
  {
    mOrientation = orientation;
    if(mNode) {
      mNode->setOrientation(mOrientation);
    }
  }

  Ogre::Quaternion SceneNodeWrapper::getOrientation()
  {
    return mOrientation;
  }

  void SceneNodeWrapper::readChildren(const DataProxy& dict)
  {
    std::map<std::pair<std::string, std::string>, bool> visited;
    for(auto& pair : dict)
    {
      const std::string& type = pair.second.get("type", "");

      if(type == "")
      {
        LOG(ERROR) << "Skipped malformed child definition: no type defined";
        continue;
      }

      // autogenerate object id if it's not defined
      auto objectId = pair.second.get<std::string>("name");
      std::string id = objectId.first;
      if (!objectId.second) {
        std::stringstream ss("");
        ss << type << counter++;
        id = ss.str();
        pair.second.put("name", id);
      }

      if(mChildren.count(type) == 0)
      {
        mChildren[type] = ObjectCollection();
      }

      visited[std::make_pair(type, id)] = true;
      if(mChildren[type].count(id) != 0)
      {
        mChildren[type][id]->read(pair.second);
        continue;
      }
      LOG(TRACE) << "Creating child " << type << " \"" << id << "\"";
      OgreObject* child = mObjectManager->create(pair.second, mOwnerId, mSceneManager, mNode);
      if(!child) {
        LOG(ERROR) << "Failed to create child " << type << " \"" << id << "\"";
        continue;
      }

      mChildren[type][id] = child;
    }

    for(auto pair : mChildren) {
      for(auto childsOfType : pair.second) {
        auto key = std::make_pair(pair.first, childsOfType.first);
        if(visited.count(key) == 0) {
          mChildren[key.first][key.second]->destroy();
          mChildren[key.first].erase(childsOfType.first);

          if(mChildren[key.first].size() == 0) {
            mChildren.erase(key.first);
          }
        }
      }
    }
  }

  DataProxy SceneNodeWrapper::writeChildren()
  {
    DataProxy values;
    for(auto& pair : mChildren)
    {
      for(auto& childPair : pair.second)
      {
        DataProxy item;
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
    mOrientation = mNode->getOrientation();
  }

  void SceneNodeWrapper::lookAt(const Ogre::Vector3& position, Ogre::Node::TransformSpace relativeTo)
  {
    mNode->lookAt(position, relativeTo, mOrientationVector);
    mOrientation = mNode->getOrientation();
  }

  void SceneNodeWrapper::destroy()
  {
    removeEventListener(mObjectManager, OgreObjectManagerEvent::FACTORY_UNREGISTERED, &SceneNodeWrapper::onFactoryUnregister);
    for(auto& subscription : mConnections)
    {
      LOG(TRACE) << subscription.first.second << ", om: " << mObjectManager << ", sp: " << subscription.first.first;
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
    auto itChild = node->getChildIterator();

    while ( itChild.hasMoreElements() )
    {
      Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
      destroyAllAttachedMovableObjects(pChildNode);
    }
  }

  OgreObject* SceneNodeWrapper::getChild(const std::string& type, const std::string& name, bool traverse)
  {
    std::string key = name;
    SceneNodeWrapper* child = this;
    if(traverse) {
      std::vector<std::string> parts = split(name, '.');
      for(auto iter = parts.begin(); iter != parts.end() - 1; iter++) {
        SceneNodeWrapper* tmp = child->getChildOfType<SceneNodeWrapper>(*iter);
        if(!tmp) {
          return 0;
        }
        child = tmp;
      }
      key = parts[parts.size() - 1];
    }

    if(child->mChildren.count(type) == 0 || child->mChildren[type].count(key) == 0)
      return 0;

    return child->mChildren[type][key];
  }

  void SceneNodeWrapper::pitch(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo)
  {
    mNode->pitch(angle, relativeTo);
    mOrientation = mNode->getOrientation();
  }

  void SceneNodeWrapper::yaw(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo)
  {
    mNode->yaw(angle, relativeTo);
    mOrientation = mNode->getOrientation();
  }

  void SceneNodeWrapper::roll(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo)
  {
    mNode->roll(angle, relativeTo);
    mOrientation = mNode->getOrientation();
  }

  void SceneNodeWrapper::translate(const Ogre::Vector3& d) {
    translate(d, Ogre::Node::TS_LOCAL);
    mOrientation = mNode->getOrientation();
  }

  void SceneNodeWrapper::translate(const Ogre::Vector3& d, Ogre::Node::TransformSpace relativeTo)
  {
    mNode->translate(d, relativeTo);
    mOrientation = mNode->getOrientation();
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
