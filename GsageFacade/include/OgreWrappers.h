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

#ifndef _OgreWrappers_H_
#define _OgreWrappers_H_

#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include <OgreLight.h>

#include "Logger.h"
#include "Serializable.h"

namespace Wrappers
{
  class Light : public Gsage::Serializable<Light>
  {
    public:
      Light(Ogre::SceneNode* parentNode, Ogre::SceneManager* sceneManager) :
        mSceneManager(sceneManager),
        mParentNode(parentNode)
      {
        BIND_ACCESSOR("name", &Light::create, &Light::getName);
        BIND_ACCESSOR("type", &Light::setType, &Light::getType);
        BIND_ACCESSOR("position", &Light::setPosition, &Light::getPosition);
        BIND_ACCESSOR("colourSpecular", &Light::setSpecularColour, &Light::getSpecularColour);
        BIND_ACCESSOR("colourDiffuse", &Light::setDiffuseColour, &Light::getDiffuseColour);
        BIND_ACCESSOR("direction", &Light::setDirection, &Light::getDirection);
        BIND_ACCESSOR("castShadows", &Light::setCastShadows, &Light::getCastShadows);
      }

      void create(const std::string& name)
      {
        mLight = mSceneManager->createLight(name);
        mParentNode->attachObject(mLight);
      }

      const std::string& getName() const
      {
        return mLight->getName();
      }

      void setType(const std::string& type)
      {
        mLight->setType(mapType(type));
      }

      std::string getType()
      {
        return mapType(mLight->getType());
      }

      void setPosition(const Ogre::Vector3& position)
      {
        mLight->setPosition(position);
      }

      const Ogre::Vector3& getPosition() const
      {
        return mLight->getPosition();
      }

      void setDiffuseColour(const Ogre::ColourValue& value)
      {
        mLight->setDiffuseColour(value);
      }

      const Ogre::ColourValue& getDiffuseColour() const
      {
        return mLight->getDiffuseColour();
      }

      void setSpecularColour(const Ogre::ColourValue& value)
      {
        mLight->setSpecularColour(value);
      }

      const Ogre::ColourValue& getSpecularColour() const
      {
        return mLight->getSpecularColour();
      }

      void setDirection(const Ogre::Vector3& value)
      {
        mLight->setDirection(value);
      }

      const Ogre::Vector3& getDirection() const
      {
        return mLight->getDirection();
      }

      void setCastShadows(const bool& value)
      {
        mLight->setCastShadows(value);
      }

      bool getCastShadows()
      {
        return mLight->getCastShadows();
      }
    private:
      Ogre::Light* mLight;

      Ogre::Light::LightTypes mapType(const std::string& type)
      {
        if(type == "point")
          return Ogre::Light::LightTypes::LT_POINT;
        else if(type == "directional")
          return Ogre::Light::LightTypes::LT_DIRECTIONAL;
        else if(type == "spotlight")
          return Ogre::Light::LightTypes::LT_SPOTLIGHT;

        return Ogre::Light::LightTypes::LT_POINT;
      }

      std::string mapType(const Ogre::Light::LightTypes& type)
      {
        if(type == Ogre::Light::LightTypes::LT_POINT)
          return "point";
        else if(type == Ogre::Light::LightTypes::LT_DIRECTIONAL)
          return "directional";
        else if(type == Ogre::Light::LightTypes::LT_SPOTLIGHT)
          return "spotlight";

        return "unknown";
      }
    private:
      Ogre::SceneNode* mParentNode;
      Ogre::SceneManager* mSceneManager;
  };


  class SceneNode : public Gsage::Serializable<SceneNode>
  {
    public:
      enum Type { STATIC = 0x01, DYNAMIC = 0x02, UNKNOWN = 0x04 };

      SceneNode() :
        mNode(0),
        mParentNode(0),
        mOffset(Ogre::Vector3::ZERO),
        mOrientationVector(Ogre::Vector3::UNIT_Z),
        mType(STATIC)
      {
        BIND_PROPERTY("offset", &mOffset);

        BIND_ACCESSOR("orientationVector", &SceneNode::setOrientationVector, &SceneNode::getOrientationVector);
        BIND_ACCESSOR("name", &SceneNode::createNode, &SceneNode::getId);
        BIND_ACCESSOR("position", &SceneNode::setPosition, &SceneNode::getPosition);
        BIND_ACCESSOR("scale", &SceneNode::setScale, &SceneNode::getScale);
        BIND_ACCESSOR("rotation", &SceneNode::setOrientation, &SceneNode::getOrientation);
        BIND_ACCESSOR("lights", &SceneNode::setLights, &SceneNode::getLights);
        BIND_ACCESSOR("children", &SceneNode::readChildren, &SceneNode::writeChildren);
        // Entity
        BIND_ACCESSOR("query", &SceneNode::setQueryFlags, &SceneNode::getQueryFlags);
        BIND_ACCESSOR("model", &SceneNode::setModel, &SceneNode::getModel);
      }

      void construct(const DataNode& node, Ogre::SceneNode* parentNode, Ogre::SceneManager* sceneManager, const std::string& entityId)
      {
        mSceneManager = sceneManager;
        mParentNode = parentNode;
        mEntityId = entityId;
        read(node);
      }

      bool hasNode()
      {
        return mNode != 0;
      }

      void createNode(const std::string& id)
      {
        mId = id;
        mNode = mParentNode->createChildSceneNode(mId);
      }

      const std::string& getId() const
      {
        return mId;
      }

      void setPosition(const Ogre::Vector3& position)
      {
        mNode->setPosition(position + (mOffset * getScale()));
      }

      Ogre::Vector3 getPosition()
      {
        return getPositionWithoutOffset() - (mOffset * getScale());
      }

      Ogre::Vector3 getPositionWithoutOffset()
      {
        return mNode != 0 ? mNode->getPosition() : Ogre::Vector3::ZERO;
      }

      void setScale(const Ogre::Vector3& scale)
      {
        mNode->setScale(scale);
      }

      Ogre::Vector3 getScale()
      {
        return mNode != 0 ? mNode->getScale() : Ogre::Vector3::ZERO;
      }

      void setOrientation(const Ogre::Quaternion& rotation)
      {
        mNode->setOrientation(rotation);
      }

      Ogre::Quaternion getOrientation()
      {
        return mNode != 0 ? mNode->getOrientation() : Ogre::Quaternion();
      }

      void setLights(const DataNode& node)
      {
        for(auto& pair : node)
        {
          mLights.emplace_back(mNode, mSceneManager);
          mLights.back().read(pair.second);
        }
      }

      DataNode getLights()
      {
        DataNode values;
        for(auto& light : mLights)
        {
          DataNode item;
          light.dump(item);
          values.push_back(std::make_pair("", item));
        }
        return values;
      }

      void readChildren(const DataNode& node)
      {
        for(auto& pair : node)
        {
          mChildren.emplace_back();
          mChildren.back().construct(pair.second, mNode, mSceneManager, mEntityId);
        }
      }

      DataNode writeChildren()
      {
        DataNode values;
        for(auto& child : mChildren)
        {
          DataNode item;
          child.dump(item);
          values.push_back(std::make_pair("", item));
        }
        return values;
      }

      void setOrientationVector(const Ogre::Vector3& value)
      {
        mOrientationVector = value.normalisedCopy();
      }

      const Ogre::Vector3& getOrientationVector() const
      {
        return mOrientationVector;
      }

      void rotate(const Ogre::Quaternion& rotation)
      {
        mNode->rotate(rotation);
      }

      void lookAt(const Ogre::Vector3& position, Ogre::Node::TransformSpace relativeTo)
      {
        mNode->lookAt(position, relativeTo, mOrientationVector);
      }

      void setQueryFlags(const std::string& type)
      {
        mTypeString = type;
        if(type == "static")
          mType = STATIC;
        else if(type == "dynamic")
          mType = DYNAMIC;
        else
          mType = UNKNOWN;
      }

      const std::string& getQueryFlags() const
      {
        return mTypeString;
      }

      void setModel(const DataNode& node)
      {
        if(node.count("mesh") == 0 || node.count("name") == 0)
          return;

        mModel = node;
        std::string model = node.get<std::string>("mesh");
        std::string name = node.get<std::string>("name");
        bool shadows = node.get("castShadows", false);
        mEntity = mSceneManager->createEntity(name, model);
        mEntity->setQueryFlags(mType);
        mEntity->setCastShadows(shadows);
        mEntity->getUserObjectBindings().setUserAny("entity", Ogre::Any(mEntityId));
        Ogre::Skeleton* skeleton = mEntity->getSkeleton();
        if(skeleton != 0)
          skeleton->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
        mNode->attachObject(mEntity);
      }

      DataNode getModel()
      {
        return mModel;
      }

      void destroy()
      {
        for(auto& node : mChildren)
        {
          node.destroy();
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

      void destroyAllAttachedMovableObjects( Ogre::SceneNode* node )
      {
        if(!node) return;

        // Destroy all the attached objects
        Ogre::SceneNode::ObjectIterator itObject = node->getAttachedObjectIterator();

        while ( itObject.hasMoreElements() )
          node->getCreator()->destroyMovableObject(itObject.getNext());

        // Recurse to child SceneNodes
        Ogre::SceneNode::ChildNodeIterator itChild = node->getChildIterator();

        while ( itChild.hasMoreElements() )
        {
          Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
          destroyAllAttachedMovableObjects( pChildNode );
        }
      }
    protected:
      std::string mId;
      std::string mEntityId;

      Ogre::Vector3 mOrientationVector;

      Ogre::SceneNode* mNode;
      Ogre::SceneNode* mParentNode;
      Ogre::SceneManager* mSceneManager;
      Ogre::Vector3 mOffset;

      typedef std::vector<SceneNode> SceneNodes;
      SceneNodes mChildren;
      typedef std::vector<Light> Lights;
      Lights mLights;

      DataNode mModel;
      std::string mTypeString;
      Type mType;

      Ogre::Entity* mEntity;
  };

}

#endif
