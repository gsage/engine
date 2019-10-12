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

#include <OgreMovableObject.h>
#include <OgreRenderable.h>

#include <Vao/OgreVaoManager.h>
#include <Vao/OgreVertexArrayObject.h>

#include <OgreSceneManager.h>

#include "ogre/v2/ManualObjectWrapper.h"
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
    auto ppoints = data.get<DataProxy>("points");
    if(!ppoints.second) {
      LOG(ERROR) << "Manual object \"" << mObjectId << "\" has no points defined";
      return;
    }

    mData = data;

    Ogre::OperationType op = data.get("renderOperation", Ogre::OT_LINE_STRIP);
    if(op != Ogre::OT_TRIANGLE_LIST && op != Ogre::OT_LINE_LIST && op != Ogre::OT_LINE_STRIP) {
      return;
    }

    if(mObject) {
      mObject->detachFromParent();
      mSceneManager->destroyManualObject(mObject);
    }

    Ogre::ManualObject* manualObject = mSceneManager->createManualObject(Ogre::SCENE_DYNAMIC);
    mObject = manualObject;

    auto mat = data.get<DataProxy>("material");

    std::string materialName = mObjectId + "Material";
    if(mat.second) {
      MaterialBuilder::parseHlms(materialName, mat.first);
    } else {
      LOG(WARNING) << "No material defined for manual object \"" << mObjectId << "\"";
    }

    manualObject->begin(materialName, op);
    size_t pc = 0;

    auto indices = data.get<DataProxy>("indices");
    bool manualIndices = indices.second;
    if(manualIndices) {
      for(auto i : indices.first) {
        auto e = i.second.getValue<int>();
        if(!e.second) {
          LOG(ERROR) << "Got malformed index " << i.second.getValueOptional<std::string>("");
          return;
        }
        manualObject->index(e.first);
      }
    }

    auto manualNormals = data.get<DataProxy>("normals");
    std::vector<Gsage::Vector3> normals;
    if(manualNormals.second) {
      size_t index = 0;
      for(auto p : manualNormals.first) {
        Gsage::Vector3 normal(0, 0, 1);
        if(!p.second.getValue(normal)) {
          LOG(WARNING) << "Malformed normal definition " << p.second.getValueOptional<std::string>("");
        }
        normals.push_back(normal);
      }
    }

    std::vector<Gsage::Vector3> points;
    points.reserve(ppoints.first.size());
    for(auto p : ppoints.first) {
      Gsage::Vector3 point(0, 0, 0);
      if(!p.second.getValue(point)) {
        LOG(WARNING) << "Malformed point definition " << p.second.getValueOptional<std::string>("");
      }
      points.push_back(point);
    }

    Gsage::Vector3 generatedNormal;
    size_t ni = 0;
    // Points list was updated, redraw manual object
    for(size_t i = 0; i < points.size(); ++i) {
      Gsage::Vector3 point = points[i];
      manualObject->position(point.X, point.Y, point.Z);
      if(ni < normals.size()) {
        Gsage::Vector3 normal = normals[ni++];
        manualObject->normal(normal.X, normal.Y, normal.Z);
      } else if(op == Ogre::OT_TRIANGLE_LIST) {
        if(i == 0 && points.size() > 2) {
          generatedNormal = Gsage::Vector3::Cross(points[i + 1] - points[i], points[i + 2] - points[i]);
        }
        manualObject->normal(generatedNormal.X, generatedNormal.Y, generatedNormal.Z);

        if(manualObject->currentVertexCount() % 3 == 0 && i + 3 < points.size()) {
          generatedNormal = Gsage::Vector3::Cross(points[i + 2] - points[i + 1], points[i + 3] - points[i + 1]);
        }
      } else {
        manualObject->normal(0, 0, 1);
      }

      if(!manualIndices) {
        switch(op) {
          case Ogre::OT_TRIANGLE_LIST:
          case Ogre::OT_TRIANGLE_STRIP:
            if(manualObject->currentVertexCount() % 3 == 0) {
              manualObject->triangle(pc, pc + 1, pc + 2);
              pc = manualObject->currentVertexCount();
            }
            break;
          case Ogre::OT_LINE_LIST:
            if(manualObject->currentVertexCount() % 2 == 0) {
              manualObject->line(pc, pc + 1);
              pc = manualObject->currentVertexCount();
            }
            break;
          default:
            manualObject->index(i);
        }
      }
    }
    manualObject->end();
    attachObject(manualObject);
    mObject = manualObject;
    defineUserBindings();
    // disable query
    mObject->setQueryFlags(0);
  }

}
