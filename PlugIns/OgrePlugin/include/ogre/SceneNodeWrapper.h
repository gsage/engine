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

#ifndef _SceneNodeWrapper_H_
#define _SceneNodeWrapper_H_

#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

#include "EventSubscriber.h"
#include "ogre/OgreObject.h"

namespace Gsage {
  class SceneNodeWrapper : public OgreObject, public EventSubscriber<SceneNodeWrapper>
  {
    public:
      enum Type { STATIC = 0x01, DYNAMIC = 0x02, UNKNOWN = 0x04 };

      static const std::string TYPE;

      SceneNodeWrapper();
      virtual ~SceneNodeWrapper();

      bool initialize(
          OgreObjectManager* objectManager,
          const DataProxy& dict,
          const std::string& ownerId,
          const std::string& type,
          Ogre::SceneManager* sceneManager,
          Ogre::SceneNode* parent);
      /**
       * Check that node has Ogre node created
       */
      bool hasNode();

      /**
       * Create Ogre::SceneNode with the specified if
       * @param id Node id
       */
      void createNode(const std::string& id);

      /**
       * Get node id
       */
      const std::string& getId() const;

      /**
       * Set node position
       * @param position Ogre::Vector3 position
       */
      void setPosition(const Ogre::Vector3& position);

      /**
       * Get node position
       */
      Ogre::Vector3 getPosition();

      /**
       * Get node position without correction that is defined in offset field
       */
      Ogre::Vector3 getPositionWithoutOffset();

      /**
       * Set SceneNode scale
       * @param scale Ogre::Vector3 scale in all dimensions
       */
      void setScale(const Ogre::Vector3& scale);

      /**
       * Get node scale
       */
      Ogre::Vector3 getScale();

      /**
       * Set node orientation
       * @param rotation Orientation quaternion
       */
      void setOrientation(const Ogre::Quaternion& rotation);

      /**
       * Get node orientation
       */
      Ogre::Quaternion getOrientation();

      void readChildren(const DataProxy& dict);

      DataProxy writeChildren();

      /**
       * Set vector that defines "face" of the node
       * @param value Ogre::Vector3 normalized/not normalized vector (it is normalized anyway)
       */
      void setOrientationVector(const Ogre::Vector3& value);

      /**
       * Get vector that defines "face" of the node
       */
      const Ogre::Vector3& getOrientationVector() const;

      /**
       * Rotate node
       * @param rotation Ogre::Quaternion that defines rotation
       */
      void rotate(const Ogre::Quaternion& rotation, const Ogre::Node::TransformSpace ts);

      /**
       * Rotate node to look at position
       * @param position Position to look at
       * @param relativeTo Coordinate space to use
       */
      void lookAt(const Ogre::Vector3& position, Ogre::Node::TransformSpace relativeTo);

      /**
       * Remove all children and destroy node
       */
      void destroy();

      /**
       * Remove all attached movable objects from the node
       * @param node Node to process
       */
      void destroyAllAttachedMovableObjects( Ogre::SceneNode* node );

      /**
       * Get child
       * @param type Type of the child
       * @param name Name, that was defined in the "name" field
       * @param traverse Traverse children splitting name by .
       */
      OgreObject* getChild(const std::string& type, const std::string& name, bool traverse = false);

      /**
       * Get child of specific type
       * @param type Type of the child
       * @param name Name, that was defined in the "name" field
       */
      template<class T>
      T* getChildOfType(const std::string& name)
      {
        return static_cast<T*>(getChild(T::TYPE, name, true));
      }

      /**
       * Rotate the node around X axis
       *
       * @param angle Rotation angle
       * @param relativeTo Transformation space
       */
      void pitch(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo=Ogre::Node::TS_LOCAL);

      /**
       * Rotate the node around Y axis
       *
       * @param angle Rotation angle
       * @param relativeTo Transformation space
       */
      void yaw(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo=Ogre::Node::TS_LOCAL);

      /**
       * Rotate the node around Z axis
       *
       * @param angle Rotation angle
       * @param relativeTo Transformation space
       */
      void roll(const Ogre::Radian &angle, Ogre::Node::TransformSpace relativeTo=Ogre::Node::TS_LOCAL);

      /**
       * Moves the node along the Cartesian axes
       *
       * @param d Vector with x,y,z values representing the translation
       * @param relativeTo Transformation space
       */
      void translate(const Ogre::Vector3& d, Ogre::Node::TransformSpace relativeTo=Ogre::Node::TS_LOCAL);
    private:
      /**
       * Handle factory removal event
       * @param event OgreObjectManager event
       */
      bool onFactoryUnregister(EventDispatcher* sender, const Event& event);

      std::string mId;

      Ogre::Vector3 mOrientationVector;

      Ogre::SceneNode* mNode;
      Ogre::Vector3 mOffset;

      typedef std::map<const std::string, OgreObject*> ObjectCollection;
      typedef std::map<const std::string, ObjectCollection> Objects;
      Objects mChildren;
  };
}
#endif
