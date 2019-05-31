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

#ifndef _EntityWrapper_H_
#define _EntityWrapper_H_

#include "Definitions.h"
#include "ogre/MovableObjectWrapper.h"
#include "DataProxy.h"
#include <OgreSkeleton.h>

namespace Ogre
{
  class Entity;
}

namespace Gsage {

  class EntityWrapper : public MovableObjectWrapper<OgreV1::Entity>
  {
    public:
      static const std::string TYPE;

      enum Query { STATIC = 0x01, DYNAMIC = 0x02, UNKNOWN = 0x04 };

      EntityWrapper();
      virtual ~EntityWrapper();
      /**
       * Set model entity flags, used for raycasting and querying entities
       * @param type static means that entity is a part of location, dynamic means that entity is some actor
       */
      void setQueryFlags(const std::string& type);

      /**
       * Get query flags of the model
       */
      const std::string& getQueryFlags() const;

      /**
       * Set model resource group
       * @param name Resource group name, e.g. General
       */
      void setResourceGroup(const std::string& name);

      /**
       * Get resource group name
       */
      const std::string& getResourceGroup() const;

      /**
       * Set model mesh (this function creates entity)
       * @param mesh Mesh file name
       */
      void setMesh(const std::string& mesh);

      /**
       * Get mesh file name
       */
      const std::string& getMesh() const;

      /**
       * Set cast shadows
       * @param value Cast shadows
       */
      void setCastShadows(const bool& value);

      /**
       * Get cast shadows
       */
      bool getCastShadows();

      /**
       * Set render queue
       * @param queue queue id
       */
      void setRenderQueue(const unsigned int& queue);

      /**
       * Get render queue
       */
      unsigned int getRenderQueue();

      /**
       * Attach another entity to the bone
       * @param params should contain boneID field
       * @param entityId Id of new entity
       * @param movableObjectData MovableObject to create and attach
       */
      void attachToBone(const DataProxy& params, const std::string& entityId, DataProxy movableObjectData);

      /**
       * Attach another entity to the bone
       * @param params must contain boneID
       */
      bool attach(Ogre::MovableObject* object, const DataProxy& params);

      /**
       * Get underlying entity
       */
      OgreV1::Entity* getEntity() {
        return mObject;
      }

      /**
       * Clone and assign entity material
       *
       * @param material
       */
      void createCloneWithMaterial(Ogre::MaterialPtr material, Ogre::uint8 renderQueue);

      /**
       * Remove cloned entity
       */
      void removeClone();

      /**
       * Get AABB
       */
#if OGRE_VERSION >= 0x020100
      inline Ogre::Aabb getAabb() const {
        return mObject ? mObject->getWorldAabb() : Ogre::Aabb();
      }
#else
      inline Ogre::AxisAlignedBox getAabb() const {
        return mObject ? mObject->getBoundingBox() : Ogre::AxisAlignedBox();
      }
#endif
    private:
      OgreV1::SkeletonAnimationBlendMode mAnimBlendMode;

      std::string mMeshName;
      std::string mQueryString;
      std::string mResourceGroup;

      Query mQuery;

      typedef std::vector<OgreObject*> AttachedEntities;

      AttachedEntities mAttachedEntities;

      OgreV1::Entity* mClone;

  };
}
#endif
