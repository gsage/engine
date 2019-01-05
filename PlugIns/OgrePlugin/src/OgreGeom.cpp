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

namespace Gsage {

  OgreGeom::OgreGeom(OgreGeom::OgreEntities src, Ogre::SceneNode* referenceNode)
    : mSrcEntities(src)
  {
    if(mSrcEntities.size() == 0) {
      return;
    }

    // get data
    bool addedShared = false;
    size_t vertexCount = 0;
    size_t indexCount = 0;
    size_t indexOffset = 0;
    size_t currentOffset = 0;
    size_t sharedOffset = 0;
    size_t nextOffset = 0;

    for(auto entity : mSrcEntities) {
      addedShared = false;

      OgreV1::MeshPtr mesh = entity->getMesh();

      // Calculate how many vertices and indices we're going to need
      for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i) {
        OgreV1::SubMesh* submesh = mesh->getSubMesh(i);
        // We only need to add the shared vertices once
        if(submesh->useSharedVertices)
        {
          if( !addedShared )
          {
            vertexCount += GET_IV_DATA(mesh->sharedVertexData)->vertexCount;
            addedShared = true;
          }
        }
        else
        {
          vertexCount += GET_IV_DATA(submesh->vertexData)->vertexCount;
        }
        // Add the indices
        indexCount += GET_IV_DATA(submesh->indexData)->indexCount;
      }
    }

    vertexCount *= 3;

    verts = new float[vertexCount];
    nverts = vertexCount;
    tris = new int[indexCount];
    ntris = indexCount;
    normals = new float[vertexCount];

    for(auto entity : mSrcEntities) {
      OgreV1::MeshPtr mesh = entity->getMesh();

      if (!entity->getParentSceneNode()) {
        continue;
      }

      LOG(INFO) << mesh->getName();

      addedShared = false;

      // Run through the submeshes again, adding the data into the arrays
      for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
      {
        OgreV1::SubMesh* submesh = mesh->getSubMesh(i);

        OgreV1::VertexData* vertexData = submesh->useSharedVertices ? GET_IV_DATA(mesh->sharedVertexData) : GET_IV_DATA(submesh->vertexData);

        if ((!submesh->useSharedVertices) || (submesh->useSharedVertices && !addedShared))
        {
          if(submesh->useSharedVertices)
          {
            addedShared = true;
            sharedOffset = currentOffset;
          }

          const OgreV1::VertexElement* posElem =
            vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

          OgreV1::HardwareVertexBufferSharedPtr vbuf =
            vertexData->vertexBufferBinding->getBuffer(posElem->getSource());

          unsigned char* vertex =
            static_cast<unsigned char*>(vbuf->lock(OgreV1::HardwareBuffer::HBL_READ_ONLY));

          const OgreV1::VertexElement* normElem =
            vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_NORMAL);

          float* pReal;

          Ogre::Matrix4 transform = referenceNode->_getFullTransform().inverse() * entity->getParentSceneNode()->_getFullTransform();

          int vertexOffset = currentOffset * 3;

          for( size_t j = 0; j < vertexData->vertexCount; ++j, vertex += vbuf->getVertexSize())
          {
            posElem->baseVertexPointerToElement(vertex, &pReal);
            Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
            Ogre::Vector3 vert = transform * pt;

            verts[vertexOffset] = vert.x;
            verts[vertexOffset+1] = vert.y;
            verts[vertexOffset+2] = vert.z;

            normElem->baseVertexPointerToElement(vertex, &pReal);
            pt = Ogre::Vector3(pReal[0], pReal[1], pReal[2]);
            vert = transform * pt;
            normals[vertexOffset] = vert.x;
            normals[vertexOffset+1] = vert.y;
            normals[vertexOffset+2] = vert.z;
            vertexOffset += 3;
          }

          vbuf->unlock();
          nextOffset += vertexData->vertexCount;
        }

        OgreV1::IndexData* indexData = GET_IV_DATA(submesh->indexData);
        size_t numTris = indexData->indexCount;
        OgreV1::HardwareIndexBufferSharedPtr ibuf = indexData->indexBuffer;

        bool use32bitindexes = (ibuf->getType() == OgreV1::HardwareIndexBuffer::IT_32BIT);

        unsigned long* pLong = static_cast<unsigned long*>(ibuf->lock(OgreV1::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);

        size_t offset = (submesh->useSharedVertices)? sharedOffset : currentOffset;

        if(use32bitindexes)
        {
          for (size_t k = 0; k < numTris; ++k)
          {
            tris[indexOffset++] = pLong[k] + static_cast<unsigned long>(offset);
          }
        }
        else
        {
          for (size_t k = 0; k < numTris; ++k)
          {
            tris[indexOffset++] = static_cast<unsigned long>(pShort[k]) +
              static_cast<unsigned long>(offset);
          }
        }

        ibuf->unlock();
        currentOffset = nextOffset;
      }
    }
  }

  OgreGeom::~OgreGeom()
  {
  }

}
