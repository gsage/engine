/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2018 Artem Chernyshev and contributors

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

#include "ogre/LineWrapper.h"
#include "GeometryPrimitives.h"
#include "OgreConverters.h"

namespace Gsage {

  const std::string LineWrapper::TYPE = "line";

  LineWrapper::LineWrapper()
  {
    BIND_PROPERTY("diffuse", &mDiffuseColor);
    BIND_PROPERTY("ambient", &mAmbientColor);
    BIND_PROPERTY("illumination", &mSelfIlluminationColor);

    BIND_ACCESSOR("points", &LineWrapper::setPoints, &LineWrapper::getPoints);
  }

  LineWrapper::~LineWrapper()
  {
  }

  void LineWrapper::setPoints(const DataProxy& points)
  {
    mPoints = points;
    if(mSceneManager->hasManualObject(mObjectId)) {
      mSceneManager->destroyManualObject(mObjectId);
    }

    Ogre::ManualObject* manualObject = mSceneManager->createManualObject(mObjectId);

    Ogre::MaterialPtr manualObjectMaterial = Ogre::MaterialManager::getSingleton().create(mObjectId + "lineMaterial", "General");
    manualObjectMaterial->setReceiveShadows(false);
    manualObjectMaterial->getTechnique(0)->setLightingEnabled(true);
    manualObjectMaterial->getTechnique(0)->getPass(0)->setDiffuse(mDiffuseColor);
    manualObjectMaterial->getTechnique(0)->getPass(0)->setAmbient(mAmbientColor);
    manualObjectMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(mSelfIlluminationColor);

    manualObject->begin(mObjectId + "lineMaterial", Ogre::RenderOperation::OT_LINE_STRIP);
    // Points list was updated, redraw the line
    for(auto p : points) {
      auto e = p.second.getValue<Gsage::Vector3>();
      if(!e.second) {
        LOG(WARNING) << "Skipped malformed point definition " << p.second.getValueOptional<std::string>("");
      }

      Gsage::Vector3 point = e.first;
      manualObject->position(point.X, point.Y, point.Z);
    }
    manualObject->end();
    mParentNode->attachObject(manualObject);
    mObject = manualObject;
  }

  const DataProxy& LineWrapper::getPoints() const
  {
    return mPoints;
  }

}
