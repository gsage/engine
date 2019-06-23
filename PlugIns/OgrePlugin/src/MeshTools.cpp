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

#include "MeshTools.h"
#include <OgreBitwise.h>

#if OGRE_VERSION >= 0x020100
#include <OgreMesh2.h>
#include <OgreSubMesh2.h>
#include <Vao/OgreAsyncTicket.h>
#include <OgreItem.h>
#endif

namespace Ogre {

  MeshInformation::MeshInformation()
    : normals(nullptr)
    , vertices(nullptr)
    , indices(nullptr)
  {
  }

  void MeshInformation::allocate(size_t vc, size_t ic, MeshInformation::Flags flags) {
    vertexCount = vc;
    indexCount = ic;

    if(flags & MeshInformation::Normals) {
      normals = new Ogre::Vector3[vertexCount];
    }

    if(flags & MeshInformation::Vertices) {
      vertices = new Ogre::Vector3[vertexCount];
    }

    if(flags & MeshInformation::Indices) {
      indices = new Ogre::uint32[indexCount];
    }
  }

  MeshInformation::~MeshInformation()
  {
    if(vertices)
      delete[] vertices;
    if(normals)
      delete[] normals;
    if(indices)
      delete[] indices;
  }

  template<> MeshTools* Ogre::Singleton<MeshTools>::msSingleton = 0;
  //-----------------------------------------------------------------------
  MeshTools* MeshTools::getSingletonPtr(void)
  {
      return msSingleton;
  }

  MeshTools& MeshTools::getSingleton(void)
  {
      assert( msSingleton );  return ( *msSingleton );
  }

  MeshTools::MeshTools()
  {
  }

  MeshTools::~MeshTools()
  {
  }

  bool MeshTools::getMeshInformation(Ogre::MovableObject* object,
    MeshInformation& dest,
    Ogre::Matrix4 transform,
    MeshInformation::Flags flags
  )
  {
    if(flags == 0) {
      return false;
    }

    if(object->getMovableType() == "Entity") {
      OgreV1::Entity* entity = (OgreV1::Entity*)object;
      getMeshInformation(
        entity->getMesh(),
        dest,
        transform,
        flags
      );
      return true;
    }
#if OGRE_VERSION >= 0x020100
    else if(object->getMovableType() == "Item") {
      Ogre::Item* item = (Ogre::Item*)object;
      getMeshInformation(
        item->getMesh(),
        dest,
        transform,
        flags
      );
      return true;
    }
#endif
    return false;
  }


  bool MeshTools::getMeshInformation(Ogre::MovableObject* object,
    MeshInformation& dest,
    const Ogre::Vector3 &position,
    const Ogre::Quaternion &orient,
    const Ogre::Vector3 &scale,
    MeshInformation::Flags flags
  )
  {
    Ogre::Matrix4 m;
    m.makeTransform(position, scale, orient);
    return getMeshInformation(object, dest, m, flags);
  }

  /**
   * Gets v1 mesh information
   */
  void MeshTools::getMeshInformation(OgreV1::MeshPtr mesh,
    MeshInformation& dest,
    const Ogre::Matrix4& transform,
    MeshInformation::Flags flags
  )
  {
    bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
    size_t next_offset = 0;
    size_t index_offset = 0;

    size_t vertex_count = 0;
    size_t index_count = 0;

    // Calculate how many vertices and indices we're going to need
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
      OgreV1::SubMesh* submesh = mesh->getSubMesh( i );

      // We only need to add the shared vertices once
      if(submesh->useSharedVertices)
      {
        if( !added_shared )
        {
          vertex_count += GET_IV_DATA(mesh->sharedVertexData)->vertexCount;
          added_shared = true;
        }
      }
      else
      {
        vertex_count += GET_IV_DATA(submesh->vertexData)->vertexCount;
      }

      // Add the indices
      index_count += GET_IV_DATA(submesh->indexData)->indexCount;
    }

    dest.allocate(vertex_count, index_count, flags);
    added_shared = false;

    // Run through the submeshes again, adding the data into the arrays
    for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
      OgreV1::SubMesh* submesh = mesh->getSubMesh(i);
      OgreV1::VertexData* vertex_data = submesh->useSharedVertices ? GET_IV_DATA(mesh->sharedVertexData) : GET_IV_DATA(submesh->vertexData);

      if((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared))
      {
        if(submesh->useSharedVertices)
        {
          added_shared = true;
          shared_offset = current_offset;
        }

        // read vertices
        if(dest.vertices || dest.normals) {
          const OgreV1::VertexElement* normElem = nullptr;
          const OgreV1::VertexElement* posElem =
            vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

          OgreV1::HardwareVertexBufferSharedPtr vbuf =
            vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

          unsigned char* vertex =
            static_cast<unsigned char*>(vbuf->lock(OgreV1::HardwareBuffer::HBL_READ_ONLY));

          if(dest.normals) {
            normElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_NORMAL);
          }

          // There is _no_ baseVertexPointerToElement() which takes an Ogre::Ogre::Real or a double
          //  as second argument. So make it float, to avoid trouble when Ogre::Ogre::Real will
          //  be comiled/typedefed as double:
          //      Ogre::Ogre::Real* pOgre::Real;
          float* pReal;

          // Read into short if using 16 bit floats for vertices
          unsigned short* pShort;

          for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
          {
            Ogre::Vector3 pt;

            if(dest.vertices) {
              if(vbuf->getVertexSize() == 20) {
                posElem->baseVertexPointerToElement(vertex, &pShort);
                for( size_t k = 0; k < 3; k++) {
                  pt[k] = Ogre::Bitwise::halfToFloat(pShort[k]);
                }
              } else {
                posElem->baseVertexPointerToElement(vertex, &pReal);
                pt = Ogre::Vector3(pReal[0], pReal[1], pReal[2]);
              }

              dest.vertices[current_offset + j] = transform * pt;
            }

            if(dest.normals) {
              normElem->baseVertexPointerToElement(vertex, &pReal);
              pt = Ogre::Vector3(pReal[0], pReal[1], pReal[2]);
              dest.normals[current_offset + j] = transform * pt;
            }
          }

          vbuf->unlock();
        }
        next_offset += vertex_data->vertexCount;
      }

      // read indices
      if(dest.indices) {

        OgreV1::IndexData* index_data = GET_IV_DATA(submesh->indexData);
        size_t numTris = index_data->indexCount / 3;
        OgreV1::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

        bool use32bitindexes = (ibuf->getType() == OgreV1::HardwareIndexBuffer::IT_32BIT);

        Ogre::uint32*  pLong = static_cast<Ogre::uint32*>(ibuf->lock(OgreV1::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);


        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

        if ( use32bitindexes )
        {
          for ( size_t k = 0; k < numTris*3; ++k)
          {
            dest.indices[index_offset++] = pLong[k] + static_cast<Ogre::uint32>(offset);
          }
        }
        else
        {
          for ( size_t k = 0; k < numTris*3; ++k)
          {
            dest.indices[index_offset++] = static_cast<Ogre::uint32>(pShort[k]) +
              static_cast<Ogre::uint32>(offset);
          }
        }

        ibuf->unlock();
      }

      current_offset = next_offset;
    }
  }

#if OGRE_VERSION >= 0x020100
  /**
   * Gets v2 mesh information
   */
  void MeshTools::getMeshInformation(Ogre::MeshPtr mesh,
    MeshInformation& dest,
    const Ogre::Matrix4& transform,
    MeshInformation::Flags flags
  )
  {
  	//First, we compute the total number of vertices and indices and init the buffers.
    size_t numVertices = 0;
    size_t numIndices = 0;

  	Ogre::Mesh::SubMeshVec::const_iterator subMeshIterator = mesh->getSubMeshes().begin();

  	while (subMeshIterator != mesh->getSubMeshes().end())
  	{
  		Ogre::SubMesh *subMesh = *subMeshIterator;
  		numVertices += subMesh->mVao[0][0]->getVertexBuffers()[0]->getNumElements();
  		numIndices += subMesh->mVao[0][0]->getIndexBuffer()->getNumElements();

  		subMeshIterator++;
  	}

  	size_t vertex_count = numVertices;
    size_t index_count = numIndices;

    size_t addedVertices = 0;
    size_t addedIndices = 0;

    size_t index_offset = 0;
    size_t subMeshOffset = 0;

    dest.allocate(numVertices, numIndices, flags);

  	// Read Submeshes
  	subMeshIterator = mesh->getSubMeshes().begin();
  	while (subMeshIterator != mesh->getSubMeshes().end())
  	{
  		Ogre::SubMesh *subMesh = *subMeshIterator;
  		Ogre::VertexArrayObjectArray vaos = subMesh->mVao[0];

  		if (!vaos.empty())
  		{
  			//Get the first LOD level
  			Ogre::VertexArrayObject *vao = vaos[0];
  			bool indices32 = (vao->getIndexBuffer()->getIndexType() == Ogre::IndexBufferPacked::IT_32BIT);

  			const Ogre::VertexBufferPackedVec &vertexBuffers = vao->getVertexBuffers();
  			Ogre::IndexBufferPacked *indexBuffer = vao->getIndexBuffer();

  			//request async read from buffer
  			Ogre::VertexArrayObject::ReadRequestsArray requests;
        std::vector<Ogre::Vector3*> dst;
        if(dest.vertices)
          dst.push_back(dest.vertices);

  			requests.push_back(Ogre::VertexArrayObject::ReadRequests(Ogre::VES_POSITION));
        if(dest.normals) {
          requests.push_back(Ogre::VertexArrayObject::ReadRequests(Ogre::VES_NORMAL));
          dst.push_back(dest.normals);
        }

  			vao->readRequests(requests);
  			vao->mapAsyncTickets(requests);
        size_t subMeshVerticiesNum = requests[0].vertexBuffer->getNumElements();

        for(size_t requestIndex = 0; requestIndex < dst.size(); ++requestIndex) {
          if (requests[requestIndex].type == Ogre::VET_HALF4)
          {
            for (size_t i = 0; i < subMeshVerticiesNum; ++i)
            {
              const Ogre::uint16* pos = reinterpret_cast<const Ogre::uint16*>(requests[requestIndex].data);
              Ogre::Vector3 vec;
              vec.x = Ogre::Bitwise::halfToFloat(pos[0]);
              vec.y = Ogre::Bitwise::halfToFloat(pos[1]);
              vec.z = Ogre::Bitwise::halfToFloat(pos[2]);
              requests[requestIndex].data += requests[requestIndex].vertexBuffer->getBytesPerElement();
              dst[requestIndex][i + subMeshOffset] = transform * vec;
            }
          }
          else if (requests[requestIndex].type == Ogre::VET_FLOAT3)
          {
            for (size_t i = 0; i < subMeshVerticiesNum; ++i)
            {
              const float* pos = reinterpret_cast<const float*>(requests[requestIndex].data);
              Ogre::Vector3 vec;
              vec.x = *pos++;
              vec.y = *pos++;
              vec.z = *pos++;
              requests[requestIndex].data += requests[requestIndex].vertexBuffer->getBytesPerElement();
              dst[requestIndex][i + subMeshOffset] = transform * vec;
            }
          }
          else
          {
            LOG(ERROR) << "Vertex Buffer type not recognised in MeshTools::getMeshInformation";
          }
        }

  			subMeshOffset += subMeshVerticiesNum;
  			vao->unmapAsyncTickets(requests);

  			////Read index data
  			if (indexBuffer)
  			{
  				Ogre::AsyncTicketPtr asyncTicket = indexBuffer->readRequest(0, indexBuffer->getNumElements());

  				unsigned int *pIndices = 0;
  				if (indices32)
  				{
  					pIndices = (unsigned*)(asyncTicket->map());
  				}
  				else
  				{
  					unsigned short *pShortIndices = (unsigned short*)(asyncTicket->map());
  					pIndices = new unsigned int[indexBuffer->getNumElements()];
  					for (size_t k = 0; k < indexBuffer->getNumElements(); k++) pIndices[k] = static_cast<unsigned int>(pShortIndices[k]);
  				}
  				unsigned int bufferIndex = 0;

  				for (size_t i = addedIndices; i < addedIndices + indexBuffer->getNumElements(); i++)
  				{
            if(dest.indices)
  					  dest.indices[i] = pIndices[bufferIndex] + index_offset;

  					bufferIndex++;
  				}
  				addedIndices += indexBuffer->getNumElements();

  				if (!indices32) delete[] pIndices;

  				asyncTicket->unmap();
  			}
  			index_offset += vertexBuffers[0]->getNumElements();
  		}
  		subMeshIterator++;
  	}
  }
#endif


}
