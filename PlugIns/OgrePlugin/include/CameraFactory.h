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

#ifndef _CameraFactory_H_
#define _CameraFactory_H_

#include "DictionaryConverters.h"
#include "GsageDefinitions.h"
#include "Serializable.h"
#include "EventSubscriber.h"

namespace MOC
{
  class CollisionTools;
}

namespace Ogre
{
  class SceneManager;
  class Camera;
}

namespace Gsage
{
  class Engine;
  class MouseEvent;
  class SelectEvent;
  class RenderComponent;
  class OgreRenderSystem;

  /**
   * Abstract camera controller
   */
  class CameraController : public Serializable<CameraController>, public EventSubscriber<CameraController>
  {
    public:
      CameraController();
      virtual ~CameraController();
      /**
       * Set all ogre stuff and engine
       */
      virtual void initialize(OgreRenderSystem* renderSystem, Engine* engine);
      /**
       * Camera state update
       * @param time Time const
       */
      virtual void update(const double& time) = 0;
      /**
       * Create Ogre::Camera instance
       * @param id Name of the camera (should be unique)
       */
      virtual void createCamera(const std::string& id) = 0;
      /**
       * Get camera name
       */
      const std::string& getId() const;
      /**
       * Sets camera target
       * @param id Entity id, should have render component
       * @returns true if entity with the specified id was found
       */
      virtual bool setTarget(const std::string& id);
      /**
       * Get underlying ogre camera
       */
      Ogre::Camera* getCamera();
      /**
       * Set camera clip distance
       * @param value Clip distance
       */
      void setClipDistance(const float& value);
      /**
       * Get camera clip distance
       */
      float getClipDistance();
      /**
       * Set viewport background colour
       * @param colour ColourValue
       */
      void setBgColour(const Ogre::ColourValue& colour);
      /**
       * Get viewport background colour
       */
      const Ogre::ColourValue& getBgColour() const;
    protected:
      // ----------------------------------------
      // Functions
      // ----------------------------------------

      virtual bool onMouseButton(EventDispatcher* sender, const Event& event) {return true;};
      virtual bool onMouseMove(EventDispatcher* sender, const Event& event);
      virtual bool handleKeyboardEvent(EventDispatcher* sender, const Event& event) {return true;};

      Ogre::Viewport* mViewport;
      Ogre::SceneManager* mSceneManager;
      Ogre::Camera* mCamera;

      Engine* mEngine;
      /**
       * Camera target id
       */
      std::string mTarget;
      std::string mId;

      bool mContinuousRaycast;
      Ogre::Vector3 mMousePosition;

      std::string mRolledOverObject;
  };

  /**
   * RPG like 3rd person camera
   */
  class IsometricCameraController : public CameraController
  {
    public:
      IsometricCameraController();
      virtual ~IsometricCameraController();

      static std::string TYPE;
      /**
       * Create Ogre::Camera instance
       * @param id Name of the camera (should be unique)
       */
      void createCamera(const std::string& id);
      /**
       * Update camera position, follow target
       * @param time Elapsed time
       */
      virtual void update(const double& time);
    protected:
      bool onMouseButton(EventDispatcher* sender, const Event& event);
      bool onMouseMove(EventDispatcher* sender, const Event& event);
    private:
      // ----------------------------------------
      // Fields
      // ----------------------------------------

      /**
       * Max camera distance
       */
      float mMaxDistance;
      /**
       * Min camera distance
       */
      float mMinDistance;
      /**
       * Max vertical angle
       */
      Ogre::Degree mMaxAngle;
      /**
       * Min vertical angle
       */
      Ogre::Degree mMinAngle;
      /**
       * Current vertical angle
       */
      Ogre::Degree mUAngle;
      /**
       * Current horisontal angle
       */
      Ogre::Degree mVAngle;
      /**
       * Camera target center
       */
      Ogre::Vector3 mCenter;
      Ogre::Vector3 mBottom;

      /**
       * Mouse wheel step multiplier
       */
      float mZoomStepMultiplier;
      float mDistance;

      bool mMoveCamera;

      Ogre::Vector3 mCameraOffset;
  };

  /**
   * WASD flyby camera
   */
  class WASDCameraController : public CameraController
  {
    public:
      WASDCameraController();
      virtual ~WASDCameraController();

      static std::string TYPE;
      /**
       * Create Ogre::Camera instance
       * @param id Name of the camera (should be unique)
       */
      void createCamera(const std::string& id);
      /**
       * Update camera position, follow target
       * @param time Elapsed time
       */
      virtual void update(const double& time);
    protected:
      bool onMouseButton(EventDispatcher* sender, const Event& event);
      bool onMouseMove(EventDispatcher* sender, const Event& event);
    private:
      // ----------------------------------------
      // Fields
      // ----------------------------------------

      bool handleKeyboardEvent(EventDispatcher* sender, const Event& event);

      Ogre::Vector3 mPosition;
      Ogre::Vector3 mMoveVector;
      Ogre::Vector3 mMousePosition;

      float mYawAngle;
      float mPitchAngle;
      bool mRotate;

      float mMoveSpeed;
  };

  /**
   * Lua camera
   */
  class LuaCameraController : public CameraController
  {
    public:
      LuaCameraController();
      virtual ~LuaCameraController();

      static std::string TYPE;
      /**
       * Create Ogre::Camera instance
       * @param id Name of the camera (should be unique)
       */
      void createCamera(const std::string& id);
      /**
       * Update camera position, follow target
       * @param time Elapsed time
       */
      virtual void update(const double& time);
      virtual bool read(const Dictionary& dict);
    protected:
      bool onMouseButton(EventDispatcher* sender, const Event& event);
      bool onMouseMove(EventDispatcher* sender, const Event& event);
    private:
      // ----------------------------------------
      // Fields
      // ----------------------------------------

      bool handleKeyboardEvent(EventDispatcher* sender, const Event& event);
      std::string mScript;
  };

  /**
   * Wrapper to create camera controller.
   * Abstract class to store non templated objects in container.
   */
  class ControllerFactory
  {
    public:
      virtual ~ControllerFactory() {};
      /**
       * Create controller.
       * Only one controller can be active at the time
       * @param settings Controller settings
       * @param renderSystem Ogre render system to attach to
       * @param engine Gsage core
       */
      virtual CameraController* create(const Dictionary& settings, OgreRenderSystem* renderSystem, Engine* engine) = 0;
  };

  template<typename TController>
  class ConcreteControllerFactory : public ControllerFactory
  {
    public:
      virtual ~ConcreteControllerFactory() {};
      /**
       * Create controller.
       * Only one controller can be active at the time
       * @param settings Controller settings
       * @param renderSystem Ogre render system to attach to
       * @param engine Gsage core
       */
      CameraController* create(const Dictionary& settings, OgreRenderSystem* renderSystem, Engine* engine);
  };

  /**
   * Factory for all possible camera controllers
   */
  class CameraFactory
  {
    public:
      CameraFactory();
      virtual ~CameraFactory();

      template<typename TController>
      void registerControllerType();
      /**
       * Create controller.
       * Only one controller can be active at the time
       * @param settings Controller settings
       * @param renderSystem Ogre render system to attach to
       * @param engine Gsage core
       */
      CameraController* initializeController(const Dictionary& settings, OgreRenderSystem* renderSystem, Engine* engine);
    private:
      typedef std::map<std::string, ControllerFactory*> ControllerFactories;
      ControllerFactories mControllerFactories;
      CameraController* mCameraController;
  };
}
#endif
