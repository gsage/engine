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

#ifndef _OgreObject_H_
#define _OgreObject_H_

#include "OgreConverters.h"
#include "Serializable.h"

namespace Ogre
{
  class SceneNode;
  class SceneManager;
}

namespace Gsage {

  class OgreObjectManager;
  /**
   * Abstract ogre object
   */
  class OgreObject : public Serializable<OgreObject>
  {
    public:
      OgreObject();
      virtual ~OgreObject();

      /**
       * Initialize element from the node with values
       * @param factory OgreObjectManager to enable child class creation
       * @param dict DataProxy with values
       * @param type String type of the object
       * @param parent Parent SceneNode
       * @param sceneManager SceneManager to use
       * @param ownerId Id of entity that owns the element
       */
      virtual bool initialize(
          OgreObjectManager* objectManager,
          const DataProxy& dict,
          const std::string& ownerId,
          const std::string& type,
          Ogre::SceneManager* sceneManager,
          Ogre::SceneNode* parent);

      /**
       * Initialize element from the node with values
       * @param factory OgreObjectManager to enable child class creation
       * @param dict DataProxy with values
       * @param type String type of the object
       * @param parent Parent OgreObject
       * @param sceneManager SceneManager to use
       * @param attachParams Passed down to the parent object when attaching
       * @param ownerId Id of entity that owns the element
       */
      virtual bool initialize(
          OgreObjectManager* objectManager,
          const DataProxy& dict,
          const std::string& ownerId,
          const std::string& type,
          Ogre::SceneManager* sceneManager,
          const DataProxy& attachParams,
          OgreObject* parent);
      /**
       * Destroy object
       */
      virtual void destroy();
      /**
       * Get object type
       */
      const std::string& getType() const;

      /**
       * Get object id
       */
      const std::string& getObjectId() const;

      /**
       * Attach object to the parent node
       * @param object Object to attach
       */
      void attachObject(Ogre::MovableObject* object);

      /**
       * Attach movable object to this object
       * @param object Object to attach
       */
      virtual bool attach(Ogre::MovableObject* object, const DataProxy& params);

      /**
       * Generate unique name for the object.
       */
      std::string generateName() const;

      /**
       * Sync all pending property updates
       */
      void sync();

    protected:
      struct PendingPropertyUpdate {
        std::string propertyName;
      };

      std::string mObjectId;
      std::string mOwnerId;
      std::string mType;

      OgreObjectManager* mObjectManager;

      Ogre::SceneManager* mSceneManager;
      Ogre::SceneNode* mParentNode;

      OgreObject* mParentObject;
      DataProxy mAttachParams;
  };
}
#endif
