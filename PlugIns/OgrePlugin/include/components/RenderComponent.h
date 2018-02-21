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

#ifndef _RenderComponent_H_
#define _RenderComponent_H_

#include "AnimationScheduler.h"
#include "Component.h"
#include "Serializable.h"
#include "EventDispatcher.h"
#include <OgreNode.h>
#include "Definitions.h"
#include "GeometryPrimitives.h"
#include "components/BaseRenderComponent.h"

namespace Ogre
{
  class Vector3;
}

namespace Gsage {

  class SceneNodeWrapper;
  class OgreRenderSystem;

  /**
   * Ogre render system component
   */
  class GSAGE_OGRE_PLUGIN_API RenderComponent : public EntityComponent, public EventDispatcher, public BaseRenderComponent
  {
    public:
      enum RotationAxis {
        X_AXIS,
        Y_AXIS,
        Z_AXIS,
        NONE
      };
      static const std::string SYSTEM;
      static const Event::Type POSITION_CHANGE;

      RenderComponent();
      virtual ~RenderComponent();
      /**
       * Set render component position
       * @param position New position
       */
      void setPosition(const Ogre::Vector3& position);
      /**
       * Set render component orientation (equal to Ogre::SceneNode::setOrientation)
       * @param orientation Orientation quaternion (absolute)
       */
      void setOrientation(const Ogre::Quaternion& orientation);
      /**
       * Rotates render component
       * @param rotation Rotation quaternion (relative)
       */
      void rotate(const Ogre::Quaternion& rotation);
      /**
       * Equalient to Ogre::Node lookAt
       * @param position Position to look at
       * @param rotationAxis rotate only in one axis
       * @param transformSpace ogre transform space
       */
      void lookAt(const Ogre::Vector3& position, const Geometry::RotationAxis rotationAxis, Geometry::TransformSpace transformSpace = Geometry::TS_WORLD);
      /**
       * Equalient to Ogre::Node lookAt
       * @param position Position to look at
       */
      void lookAt(const Ogre::Vector3& position);
      /**
       * Get current position
       */
      const Ogre::Vector3 getOgrePosition();
      /**
       * Get current scale
       */
      const Ogre::Vector3 getOgreScale();
      /**
       * Get current direction vector, uses orientationVector from config to detect front of the object
       */
      const Ogre::Vector3 getOgreDirection();
      /**
       * Get object orientation
       */
      const Ogre::Quaternion getOgreOrientation();
      /**
       * Get object orientation with respect to the direction vector
       */
      const Ogre::Quaternion getOgreFaceOrientation();

      // BaseRenderComponent implementation

      /**
       * Rotates render component using Gsage::Quaternion
       * @param rotation Rotation quaternion (relative)
       */
      void rotate(const Gsage::Quaternion& rotation);
      /**
       * Equalient to Ogre::Node lookAt
       * @param position Position to look at
       * @param rotationAxis rotate only in one axis
       * @param transformSpace ogre transform space
       */
      void lookAt(const Gsage::Vector3& position, const Geometry::RotationAxis rotationAxis, Geometry::TransformSpace transformSpace = Geometry::TS_WORLD);
      /**
       * @param position Position to look at
       */
      void lookAt(const Gsage::Vector3& position);
      /**
       * Set position by using Gsage::Vector
       * @param position New position
       */
      void setPosition(const Gsage::Vector3& position);
      /**
       * Set orientation using Gsage::Quaternion
       * @param orientation Orientation quaternion (absolute)
       */
      void setOrientation(const Gsage::Quaternion& orientation);
      /**
       * Get current position
       */
      const Gsage::Vector3 getPosition();
      /**
       * Get current scale
       */
      const Gsage::Vector3 getScale();
      /**
       * Get current direction vector, uses orientationVector from config to detect front of the object
       */
      const Gsage::Vector3 getDirection();
      /**
       * Get object orientation
       */
      const Gsage::Quaternion getOrientation();
      /**
       * Get object orientation with respect to the direction vector
       */
      const Gsage::Quaternion getFaceOrientation();

      /**
       * Adjusts animation speed for state
       * @param name Animation state name
       * @param speed animation speed
       */
      bool adjustAnimationStateSpeed(const std::string& name, double speed);
      /**
       * Sets animation state
       * @param name Animation name
       * @returns true if state was found
       */
      bool setAnimationState(const std::string& name);
      /**
       * Plays animation
       *
       * @param name Animation group name
       * @param times Repeat times, -1 means loop
       * @param speed Animation speed, 1 -- is normal speed
       * @param offset Animation start offset
       * @param reset Starts animation immediately
       *
       * @returns true if animation group exists
       */
      bool playAnimation(const std::string& name, int times = -1, double speed = 1, double offset = 0, bool reset = false);
      /**
       * Resets animation state to default
       */
      void resetAnimationState();
      /**
       * Reads component root node
       */
      DataProxy getRootNode();
      /**
       * Reads component animations
       */
      DataProxy getAnimations();

      /**
       * Get root SceneNodeWrapper
       */
      SceneNodeWrapper* getRoot();

      /**
       * Read additional resources paths from the DataProxy
       * @param resources DataProxy with all resources settings
       */
      void setResources(const DataProxy& resources);

      /**
       * Get resources
       */
      const DataProxy& getResources() const;
    private:
      friend class OgreRenderSystem;
      bool mAddedToScene;

      AnimationScheduler mAnimationScheduler;
      SceneNodeWrapper* mRootNode;

      DataProxy mResources;
  };
}

#endif
