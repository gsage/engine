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
#include "ogre/MovableObjectWrapper.h"
#include <OgreRoot.h>
#include "Logger.h"

namespace Gsage {
  static long counter = 0;

  const std::string SceneNodeWrapper::TYPE = "node";

  SceneNodeWrapper::SceneNodeWrapper() :
    mNode(0),
    mOffset(Ogre::Vector3::ZERO),
    mOrientationVector(Ogre::Vector3::UNIT_Z)
#if OGRE_VERSION >= 0x020100
    , mDirtyProperties(0)
#endif
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
    if (mParentNode == 0 && mParentObject == 0) {
      LOG(ERROR) << "Failed to create node \"" << mObjectId << "\" parent node is not set";
      return;
    }

    if(!mParentNode) {
      if(mParentObject->getType() == "node") {
        mParentNode = static_cast<SceneNodeWrapper*>(mParentObject)->mNode;
      } else {
        mParentNode = mSceneManager->getRootSceneNode();
      }
    }

    mObjectId = id;
    std::string name = generateName();

    if(mNode == 0) {
#if OGRE_VERSION_MAJOR == 2
        // TODO: static/dynamic nodes
      mNode = mParentNode->createChildSceneNode();
#else
      mNode = mParentNode->createChildSceneNode(name);
#endif
      LOG(TRACE) << "Creating scene node \"" << name << "\"";
    } else {
      LOG(TRACE) << "Updating scene node \"" << name << "\"";
    }
  }

  const std::string& SceneNodeWrapper::getId() const
  {
    return mObjectId;
  }

  void SceneNodeWrapper::setPosition(const Ogre::Vector3& position)
  {
    setPositionWithoutOffset(position + (mOffset * getScale()));
  }

  void SceneNodeWrapper::setPositionWithoutOffset(const Ogre::Vector3& position)
  {
    assert(mNode != 0);
    mNode->setPosition(position);
#if OGRE_VERSION >= 0x020100
    // mark position property dirty
    markDirty(SceneNodeWrapper::Position);
#endif
  }

  Ogre::Vector3 SceneNodeWrapper::getPosition()
  {
    return getPositionWithoutOffset() - (mOffset * getScale());
  }

  Ogre::Vector3 SceneNodeWrapper::getPositionWithoutOffset()
  {
    if(mNode == 0) {
      return Ogre::Vector3::ZERO;
    }
#if OGRE_VERSION >= 0x020100
    updateProperty(SceneNodeWrapper::Position);
#endif
    return mNode->getPosition();
  }

  void SceneNodeWrapper::setScale(const Ogre::Vector3& scale)
  {
    mNode->setScale(scale);
#if OGRE_VERSION >= 0x020100
    // mark scale property dirty
    markDirty(SceneNodeWrapper::Scale);
#endif
  }

  Ogre::Vector3 SceneNodeWrapper::getScale()
  {
    if(mNode == 0) {
      return Ogre::Vector3::ZERO;
    }

#if OGRE_VERSION >= 0x020100
    updateProperty(SceneNodeWrapper::Scale);
#endif
    return mNode->getScale();
  }

  void SceneNodeWrapper::setOrientation(const Ogre::Quaternion& orientation)
  {
    mNode->setOrientation(orientation);
#if OGRE_VERSION >= 0x020100
    // mark orientation property dirty
    markDirty(SceneNodeWrapper::Orientation);
#endif
  }

  Ogre::Quaternion SceneNodeWrapper::getOrientation()
  {
    if(mNode == 0) {
      return Ogre::Quaternion();
    }
#if OGRE_VERSION >= 0x020100
    updateProperty(SceneNodeWrapper::Orientation);
#endif
    return mNode->getOrientation();
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
      DataProxy params;
      OgreObject* child = mObjectManager->create(pair.second, mOwnerId, mSceneManager, type, params, this);
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
#if OGRE_VERSION >= 0x020100
    updateProperty(SceneNodeWrapper::Orientation);
#endif
    mNode->rotate(rotation, ts);
#if OGRE_VERSION >= 0x020100
    markDirty(SceneNodeWrapper::Orientation);
#endif
  }

  void SceneNodeWrapper::rotate(const Ogre::Vector3& axis, const Ogre::Degree& degree, const Ogre::Node::TransformSpace ts)
  {
#if OGRE_VERSION >= 0x020100
    updateProperty(SceneNodeWrapper::Orientation);
#endif
    mNode->rotate(axis, degree, ts);
#if OGRE_VERSION >= 0x020100
    markDirty(SceneNodeWrapper::Orientation);
#endif
  }

  void SceneNodeWrapper::lookAt(const Ogre::Vector3& position, Ogre::Node::TransformSpace relativeTo)
  {
#if OGRE_VERSION >= 0x020100
    updateProperty(SceneNodeWrapper::Orientation);
#endif
    mNode->lookAt(position, relativeTo, mOrientationVector);
#if OGRE_VERSION >= 0x020100
    markDirty(SceneNodeWrapper::Orientation);
#endif
  }

  void SceneNodeWrapper::destroy()
  {
    removeEventListener(mObjectManager, OgreObjectManagerEvent::FACTORY_UNREGISTERED, &SceneNodeWrapper::onFactoryUnregister);
    for(auto& pair : mChildren)
    {
      for(auto& objectPair : pair.second)
      {
        objectPair.second->destroy();
      }
    }

    if(mParentNode != 0)
      mParentNode->removeChild(mNode);

    OgreObject::destroy();

    if(mNode != 0)
    {
      destroyAllAttachedMovableObjects(mNode);
      mNode->removeAndDestroyAllChildren();
      mSceneManager->destroySceneNode(mNode);
    }

  }

  bool SceneNodeWrapper::attach(Ogre::MovableObject* object, const DataProxy& params)
  {
    mNode->attachObject(object);
    return true;
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
    std::vector<std::string> parts = split(name, '.');
    if(traverse && parts.size() > 1) {
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

  IMovableObjectWrapper* SceneNodeWrapper::getMovableObject(const std::string& type, const std::string& name) {
    OgreObject* obj = getChild(type, name, true);
    if(!obj) {
      return 0;
    }

    return static_cast<IMovableObjectWrapper*>(obj);
  }

  void SceneNodeWrapper::pitch(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo)
  {
#if OGRE_VERSION >= 0x020100
    updateProperty(SceneNodeWrapper::Orientation);
#endif
    mNode->pitch(angle, relativeTo);
#if OGRE_VERSION >= 0x020100
    markDirty(SceneNodeWrapper::Orientation);
#endif
  }

  void SceneNodeWrapper::yaw(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo)
  {
#if OGRE_VERSION >= 0x020100
    updateProperty(SceneNodeWrapper::Orientation);
#endif
    mNode->yaw(angle, relativeTo);
#if OGRE_VERSION >= 0x020100
    markDirty(SceneNodeWrapper::Orientation);
#endif
  }

  void SceneNodeWrapper::roll(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo)
  {
#if OGRE_VERSION >= 0x020100
    updateProperty(SceneNodeWrapper::Orientation);
#endif
    mNode->roll(angle, relativeTo);
#if OGRE_VERSION >= 0x020100
    markDirty(SceneNodeWrapper::Orientation);
#endif
  }

  void SceneNodeWrapper::translate(const Ogre::Vector3& d) {
    translate(d, Ogre::Node::TS_LOCAL);
  }

  void SceneNodeWrapper::translate(const Ogre::Vector3& d, Ogre::Node::TransformSpace relativeTo)
  {
    mNode->translate(d, relativeTo);
#if OGRE_VERSION >= 0x020100
    markDirty(SceneNodeWrapper::Position);
#endif
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
