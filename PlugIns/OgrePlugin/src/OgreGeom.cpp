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

#include "OgreGeom.h"
#include <OgreNode.h>
#include <OgreEntity.h>
#include <OgreSceneManager.h>
#include <OgreSubMesh.h>
#include "Logger.h"
#include "MeshTools.h"

namespace Gsage {

  OgreGeom::OgreGeom(OgreGeom::OgreEntities src, Ogre::SceneNode* referenceNode)
    : mSrcEntities(src)
  {
    if(mSrcEntities.size() == 0) {
      return;
    }

    // get data
    size_t vertexCount = 0;
    size_t indexCount = 0;
    size_t indexOffset = 0;
    size_t vertexOffset = 0;

    Ogre::MeshInformation* meshData = new Ogre::MeshInformation[mSrcEntities.size()];
    size_t meshCount = 0;
    Ogre::MeshTools* meshTools = Ogre::MeshTools::getSingletonPtr();

    for(auto entity : mSrcEntities) {
      if (!entity->getParentSceneNode()) {
        continue;
      }

      Ogre::Matrix4 transform = referenceNode->_getFullTransform().inverse() * entity->getParentSceneNode()->_getFullTransform();

      Ogre::MeshInformation& info = meshData[meshCount];
      if(!meshTools->getMeshInformation(
        entity,
        info,
        transform,
        Ogre::MeshInformation::All
      )) {
        LOG(WARNING) << "Found movable object of incorrect type " << entity->getMovableType() << " in raw geom source list, skipped";
        continue;
      }

      vertexCount += info.vertexCount;
      indexCount += info.indexCount;
      meshCount++;
    }

    vertexCount *= 3;

    verts = new float[vertexCount];
    nverts = vertexCount;
    tris = new int[indexCount];
    ntris = indexCount;
    normals = new float[vertexCount];
    int offset = 0;

    for(size_t i = 0; i < meshCount; ++i) {
      Ogre::MeshInformation& info = meshData[i];
      for(size_t j = 0; j < info.indexCount; ++j) {
        tris[indexOffset++] = (int)info.indices[j] + offset;
      }

      for(size_t j = 0; j < info.vertexCount; ++j)
      {
        verts[vertexOffset] = info.vertices[j].x;
        verts[vertexOffset+1] = info.vertices[j].y;
        verts[vertexOffset+2] = info.vertices[j].z;

        normals[vertexOffset] = info.normals[j].x;
        normals[vertexOffset+1] = info.normals[j].y;
        normals[vertexOffset+2] = info.normals[j].z;
        vertexOffset += 3;
      }

      offset += info.vertexCount;
    }

    delete[] meshData;
  }

  OgreGeom::~OgreGeom()
  {
  }

}
