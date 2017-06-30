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

#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include "ogre/CameraWrapper.h"

namespace Gsage {

  const std::string CameraWrapper::TYPE = "camera";

  CameraWrapper::CameraWrapper()
    : mTarget("")
    , mViewport(0)
    , mWindow(0)
    , mIsActive(false)
  {
    BIND_ACCESSOR_WITH_PRIORITY("name", &CameraWrapper::createCamera, &CameraWrapper::getName, 1);

    BIND_PROPERTY("target", &mTarget);
    BIND_ACCESSOR("bgColour", &CameraWrapper::setBgColour, &CameraWrapper::getBgColour);
    BIND_ACCESSOR("clipDistance", &CameraWrapper::setClipDistance, &CameraWrapper::getClipDistance);
    BIND_ACCESSOR("orientation", &CameraWrapper::setOrientation, &CameraWrapper::getOrientation);
  }

  CameraWrapper::~CameraWrapper()
  {
  }

  const Ogre::Quaternion& CameraWrapper::getOrientation() const
  {
    return static_cast<const Ogre::Camera*>(mObject)->getOrientation();
  }

  void CameraWrapper::setOrientation(const Ogre::Quaternion& orientation)
  {
    getCamera()->setOrientation(orientation);
  }

  void CameraWrapper::attach(Ogre::Viewport* viewport) {
    Ogre::Camera* camera = getCamera();
    mViewport = viewport;
    if (mViewport == 0) {
      LOG(ERROR) << "Failed to get viewport";
      return;
    }

    mViewport->setBackgroundColour(mBgColour);
    camera->setAspectRatio(
        float(mViewport->getActualWidth()) / float(mViewport->getActualHeight()));
    mViewport->setCamera(camera);
    mIsActive = true;
    LOG(INFO) << "Attached camera to viewport";
  }

  void CameraWrapper::createCamera(const std::string& name)
  {
    mObjectId = name;
    LOG(TRACE) << "Create camera with id " << name;
    // toss camera object to the movable wrapper
    mObject = mSceneManager->createCamera(name);
    attachObject(mObject);
  }

  Ogre::Camera* CameraWrapper::getCamera()
  {
    return static_cast<Ogre::Camera*>(mObject);
  }

  const std::string& CameraWrapper::getName() const
  {
    return mObjectId;
  }

  void CameraWrapper::setBgColour(const Ogre::ColourValue& value)
  {
    mBgColour = value;
    if(mViewport) {
      mViewport->setBackgroundColour(value);
    }
  }

  const Ogre::ColourValue& CameraWrapper::getBgColour() const
  {
    return mBgColour;
  }

  void CameraWrapper::setClipDistance(const float& value)
  {
    getCamera()->setNearClipDistance(value);
  }

  float CameraWrapper::getClipDistance()
  {
    return getCamera()->getNearClipDistance();
  }

  bool CameraWrapper::isActive() const
  {
    return mIsActive;
  }
}
