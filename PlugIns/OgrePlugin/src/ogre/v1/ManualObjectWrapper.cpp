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

#include "ogre/v1/ManualObjectWrapper.h"
#include "ogre/MaterialBuilder.h"

namespace Gsage {

  const std::string ManualObjectWrapper::TYPE = "manualObject";

  ManualObjectWrapper::ManualObjectWrapper()
  {
    BIND_ACCESSOR("data", &ManualObjectWrapper::setData, &ManualObjectWrapper::getData);
  }

  ManualObjectWrapper::~ManualObjectWrapper()
  {
  }

  const DataProxy& ManualObjectWrapper::getData() const
  {
    return mData;
  }

  void ManualObjectWrapper::setData(const DataProxy& data)
  {
    auto points = data.get<DataProxy>("points");
    if(!points.second) {
      LOG(ERROR) << "Manual object \"" << mObjectId << "\" has no points defined";
      return;
    }

    mData = data;
    if(mSceneManager->hasManualObject(mObjectId)) {
      mSceneManager->destroyManualObject(mObjectId);
    }

    Ogre::ManualObject* manualObject = mSceneManager->createManualObject(mObjectId);

    auto mat = data.get<DataProxy>("material");

    std::string materialName = mObjectId + "Material";
    if(mat.second) {
      MaterialBuilder::parse(materialName, mat.first);
    } else {
      LOG(WARNING) << "No material defined for manual object \"" << mObjectId << "\"";
    }

    Ogre::RenderOperation::OperationType op = data.get("renderOperation", Ogre::RenderOperation::OT_LINE_STRIP);

    manualObject->begin(materialName, op);

    // Points list was updated, redraw manual object
    for(auto p : points.first) {
      auto e = p.second.getValue<Gsage::Vector3>();
      if(!e.second) {
        LOG(WARNING) << "Skipped malformed point definition " << p.second.getValueOptional<std::string>("");
      }

      Gsage::Vector3 point = e.first;
      manualObject->position(point.X, point.Y, point.Z);
    }
    manualObject->end();
    attachObject(manualObject);
    mObject = manualObject;
  }

}
