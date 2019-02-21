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

#include "v2/ImguiRenderable.h"
#include "OgreStableHeaders.h"

#include "OgreHardwareBufferManager.h"
#include "OgreMaterialManager.h"
#include <OgreSceneManager.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreHardwareIndexBuffer.h>
#include <OgreHardwarePixelBuffer.h>
#include "Logger.h"

#include "imgui.h"

namespace Gsage {

  ImguiRenderable::ImguiRenderable(Ogre::VaoManager* vaoManager)
    : mVertexBuffer(0)
    , mVaoManager(vaoManager)
    , mVao(0)
  {
    mElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT2, Ogre::VES_POSITION));
    mElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES));
    mElements.push_back(Ogre::VertexElement2(Ogre::VET_COLOUR, Ogre::VES_DIFFUSE));
  }

  ImguiRenderable::~ImguiRenderable()
  {

  }

  void ImguiRenderable::updateVertexData(
    const ImDrawVert* vtxBuf,
    const ImDrawIdx* idxBuf,
    unsigned int vtxCount,
    unsigned int idxCount
  )
  {
    // recreate vertex buffer if vtxCount changes
    if(mVertexBuffer == nullptr || mVertexBuffer->getNumElements() != vtxCount) {
      if(mVertexBuffer != nullptr) {
        if(mVertexBuffer->getMappingState() != 0) {
          mVertexBuffer->unmap(Ogre::UO_UNMAP_ALL);
        }
        mVaoManager->destroyVertexBuffer(mVertexBuffer);
      }

      mVertexBuffer = mVaoManager->createVertexBuffer(mElements,
          vtxCount,
          Ogre::BT_DYNAMIC_PERSISTENT_COHERENT,
          NULL, false);
    }

    Ogre::VertexBufferPackedVec vertexBuffers;
    vertexBuffers.push_back(mVertexBuffer);

    auto declSize = Ogre::v1::VertexElement::getTypeSize(Ogre::VET_FLOAT2) \
                  + Ogre::v1::VertexElement::getTypeSize(Ogre::VET_FLOAT2) \
                  + Ogre::v1::VertexElement::getTypeSize(Ogre::VET_COLOUR);

    ImDrawVert* vertexData = static_cast<ImDrawVert*>(mVertexBuffer->map(0, mVertexBuffer->getNumElements()));
    assert(vertexData);
    memcpy(vertexData, vtxBuf, declSize * vtxCount);
    mVertexBuffer->unmap(Ogre::UO_KEEP_PERSISTENT);

    Ogre::IndexBufferPacked *indexBuffer = mVaoManager->createIndexBuffer(
        Ogre::IndexBufferPacked::IT_16BIT,
        idxCount,
        Ogre::BT_DYNAMIC_PERSISTENT_COHERENT,
        NULL,
        false);

    ImDrawIdx* indexData = static_cast<ImDrawIdx*>(indexBuffer->map(0, indexBuffer->getNumElements()));
    assert(indexData);
    memcpy(indexData, idxBuf, idxCount * sizeof(ImDrawIdx));
    indexBuffer->unmap(Ogre::UO_KEEP_PERSISTENT);

    if(mVao) {
      mVaoPerLod[0].clear();
      mVaoPerLod[1].clear();
    }

    mVao = mVaoManager->createVertexArrayObject(
        vertexBuffers,
        indexBuffer,
        Ogre::OT_TRIANGLE_LIST
    );

    mVaoPerLod[0].push_back(mVao);
    mVaoPerLod[1].push_back(mVao);
  }

  void ImguiRenderable::getRenderOperation(Ogre::v1::RenderOperation& op, bool casterPass)
  {
    OGRE_EXCEPT( Ogre::Exception::ERR_NOT_IMPLEMENTED,
        "ImguiMovableObject does not implement getRenderOperation. "
        "Use ImguiMovableObject::setRenderQueueGroup to change the group.",
        "ImguiRenderable::getRenderOperation" );
  }

  const Ogre::LightList& ImguiRenderable::getLights(void) const
  {
    static const Ogre::LightList l;
    return l;
  }

  void ImguiRenderable::getWorldTransforms( Ogre::Matrix4* xform ) const
  {
    OGRE_EXCEPT( Ogre::Exception::ERR_NOT_IMPLEMENTED,
        "ImguiMovableObject does not implement getWorldTransforms. "
        "Use ImguiMovableObject::setRenderQueueGroup to change the group.",
        "ImguiRenderable::getWorldTransforms" );
  }

}

