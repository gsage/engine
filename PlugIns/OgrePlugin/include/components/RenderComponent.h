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
#include <OgreNode.h>

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
  class RenderComponent : public EntityComponent
  {
    public:
      enum RotationAxis {
        X_AXIS,
        Y_AXIS,
        Z_AXIS,
        NONE
      };
      static const std::string SYSTEM;

      RenderComponent();
      virtual ~RenderComponent();
      /**
       * Set render component position
       * @param x X coordinate
       * @param y Y coordinate
       * @param z Z coordinate
       */
      void setPosition(const float& x, const float& y, const float& z);
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
      void lookAt(const Ogre::Vector3& position, const RotationAxis rotationAxis, Ogre::Node::TransformSpace transformSpace = Ogre::Node::TS_WORLD);

      /**
       * Equalient to Ogre::Node lookAt
       * @param position Position to look at
       */
      void lookAt(const Ogre::Vector3& position);
      /**
       * Get current position
       */
      const Ogre::Vector3 getPosition();
      /**
       * Get current scale
       */
      const Ogre::Vector3 getScale();
      /**
       * Get current direction vector, uses orientationVector from config to detect front of the object
       */
      const Ogre::Vector3 getDirection();
      /**
       * Get object orientation
       */
      const Ogre::Quaternion getOrientation();
      /**
       * Get object orientation with respect to the direction vector
       */
      const Ogre::Quaternion getFaceOrientation();
      /**
       * Adjusts animation speed for state
       * @param name Animation state name
       * @param speed animation speed
       */
      bool adjustAnimationStateSpeed(const std::string& name, const float& speed);
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
      bool playAnimation(const std::string& name, const float& speed = 1, const float& times = -1, const float& offset = 0, bool reset = false);
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
