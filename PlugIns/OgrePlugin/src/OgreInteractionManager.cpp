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
/*
#include "OgreInteractionManager.h"
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>

#include "systems/OgreRenderSystem.h"
#include "ogre/SceneNodeWrapper.h"

#include "Engine.h"
#include "OgreSelectEvent.h"
#include "OgreInteractionEvent.h"
#include "KeyboardEvent.h"
#include "CollisionTools.h"

namespace Gsage {

  OgreInteractionManager::OgreInteractionManager()
    : mEngine(0)
    , mRenderSystem(0)
    , mCollisionTools(0)
    , mContinuousRaycast(false)
    , mWidth(1)
    , mHeight(1)
    , mCamera(0)
  {
  }

  OgreInteractionManager::~OgreInteractionManager()
  {
    if(mCollisionTools != 0)
      delete mCollisionTools;
  }

  void OgreInteractionManager::initialize(OgreRenderSystem* renderSystem, Engine* engine) {
    mEngine = engine;
    mRenderSystem = renderSystem;
    mCollisionTools = new MOC::CollisionTools(mRenderSystem->getSceneManager());

    addEventListener(mEngine, OgreInteractionEvent::MOUSE_DOWN, &OgreInteractionManager::onMouseButton);
    addEventListener(mEngine, OgreInteractionEvent::MOUSE_UP, &OgreInteractionManager::onMouseButton);
    addEventListener(mEngine, OgreInteractionEvent::MOUSE_MOVE, &OgreInteractionManager::onMouseMove);
  }

  bool OgreInteractionManager::onMouseButton(EventDispatcher* sender, const Event& event)
  {
    const OgreInteractionEvent& e = (OgreInteractionEvent&) event;
    mWidth = e.width;
    mHeight = e.height;
    mCamera = e.camera;
    if(e.button == MouseEvent::Left)
    {
      if(e.getType() == OgreInteractionEvent::MOUSE_DOWN)
      {
        doRaycasting(e.mouseX, e.mouseY, 0xFFFF, true);
      }
      else
      {
        mContinuousRaycast = false;
      }
    }
    return true;
  }

  bool OgreInteractionManager::onMouseMove(EventDispatcher* sender, const Event& event)
  {
    const OgreInteractionEvent& e = (OgreInteractionEvent&) event;


    mWidth = e.width;
    mHeight = e.height;
    mCamera = e.camera;
    return true;
  }

  void OgreInteractionManager::doRaycasting(const float& offsetX, const float& offsetY, unsigned int flags, bool select)
  {
    Ogre::MovableObject* target;
    Ogre::Vector3 result;
    float closestDistance = 0.1f;

    if(!mCamera)
    {
      return;
    }

    bool hitObject = mCollisionTools->raycastFromCamera(mWidth, mHeight, mCamera, Ogre::Vector2(offsetX, offsetY), result, target, closestDistance, flags);

    if(!hitObject)
    {
      if(!mRolledOverObject.empty())
      {
        mEngine->fireEvent(OgreSelectEvent(SelectEvent::ROLL_OUT, 0x00, mRolledOverObject, result));
        mRolledOverObject.clear();
      }
      return;
    }

    const Ogre::Any& entityId = target->getUserObjectBindings().getUserAny("entity");
    if(entityId.isEmpty())
    {
      return;
    }

    if((target->getQueryFlags() & SceneNodeWrapper::STATIC) == SceneNodeWrapper::STATIC && select)
       mContinuousRaycast = true;

    const std::string& id = entityId.get<const std::string>();
    if(mRolledOverObject != id)
    {
      mEngine->fireEvent(OgreSelectEvent(OgreSelectEvent::ROLL_OUT, target->getQueryFlags(), mRolledOverObject, result));
      mRolledOverObject = id;
      mEngine->fireEvent(OgreSelectEvent(OgreSelectEvent::ROLL_OVER, target->getQueryFlags(), id, result));
    }

    if(select)
      mEngine->fireEvent(OgreSelectEvent(OgreSelectEvent::OBJECT_SELECTED, target->getQueryFlags(), id, result));
  }

  void OgreInteractionManager::update(const double& time)
  {
    if(!mEngine) {
      return;
    }

  }
}*/
