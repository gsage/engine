/*
   -----------------------------------------------------------------------------
   This source file is part of OGRE
   (Object-oriented Graphics Rendering Engine)
   For the latest info, see http://www.ogre3d.org/

   Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#include "OgreStableHeaders.h"

#include "v1/ImguiRenderable.h"

#include "OgreHardwareBufferManager.h"
#include "OgreMaterialManager.h"
#include <OgreSceneManager.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreHardwareIndexBuffer.h>
#include <OgreHardwarePixelBuffer.h>

#include "Logger.h"


namespace Gsage
{
  ImGUIRenderable::ImGUIRenderable():
    mVertexBufferSize(5000)
    ,mIndexBufferSize(10000)
  {
    initImGUIRenderable();
    //By default we want ImGUIRenderables to still work in wireframe mode
    setPolygonModeOverrideable( false );
  }

  //-----------------------------------------------------------------------------------
  void ImGUIRenderable::initImGUIRenderable(void)
  {
    // use identity projection and view matrices
    mUseIdentityProjection  = true;
    mUseIdentityView        = true;

    mRenderOp.vertexData = OGRE_NEW OgreV1::VertexData();
    mRenderOp.indexData  = OGRE_NEW OgreV1::IndexData();

    mRenderOp.vertexData->vertexCount = 0;
    mRenderOp.vertexData->vertexStart = 0;

    mRenderOp.indexData->indexCount = 0;
    mRenderOp.indexData->indexStart = 0;
#if OGRE_VERSION_MAJOR == 1
    mRenderOp.operationType             = Ogre::RenderOperation::OT_TRIANGLE_LIST;
#else
    mRenderOp.operationType             = Ogre::OT_TRIANGLE_LIST;
#endif
    mRenderOp.useIndexes                                    = true;
    mRenderOp.useGlobalInstancingVertexBufferIsAvailable    = false;

    OgreV1::VertexDeclaration* decl     = mRenderOp.vertexData->vertexDeclaration;

    // vertex declaration
    size_t offset = 0;
    decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_POSITION);
    offset += OgreV1::VertexElement::getTypeSize( Ogre::VET_FLOAT2 );
    decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0);
    offset += OgreV1::VertexElement::getTypeSize( Ogre::VET_FLOAT2 );
    decl->addElement(0, offset, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
#if OGRE_VERSION >= 0x020100
    mVertexElement2VecVec = decl->convertToV2();
#endif

    // set basic white material
    this->setMaterial("imgui/material");
  }

  //-----------------------------------------------------------------------------------

  ImGUIRenderable::~ImGUIRenderable()
  {
    OGRE_DELETE mRenderOp.vertexData;
  }

  //-----------------------------------------------------------------------------------

  void ImGUIRenderable::setMaterial( const Ogre::String& matName )
  {
    mMaterial = Ogre::MaterialManager::getSingleton().getByName( matName );
    if( mMaterial.isNull() )
    {
      OGRE_EXCEPT(Ogre::Exception::ERR_ITEM_NOT_FOUND, "Could not find material " + matName,
          "ImGUIRenderable::setMaterial");
    }

    // Won't load twice anyway
    mMaterial->load();
  }

  //-----------------------------------------------------------------------------------

  void ImGUIRenderable::setMaterial(const Ogre::MaterialPtr & material)
  {
    mMaterial = material;
  }

  //-----------------------------------------------------------------------------------

  const Ogre::MaterialPtr& ImGUIRenderable::getMaterial(void) const
  {
    return mMaterial;
  }

  //-----------------------------------------------------------------------------------

  void ImGUIRenderable::updateVertexData(ImDrawData* draw_data, unsigned int cmdIndex)
  {
    OgreV1::VertexBufferBinding* bind   = mRenderOp.vertexData->vertexBufferBinding;

    const ImDrawList* cmd_list = draw_data->CmdLists[cmdIndex];
    bool sizeChanged = false;

    if (bind->getBindings().empty() || mVertexBufferSize != cmd_list->VtxBuffer.size())
    {
      mVertexBufferSize = cmd_list->VtxBuffer.size();
      sizeChanged = true;

      bind->setBinding(0, OgreV1::HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(ImDrawVert), mVertexBufferSize, OgreV1::HardwareBuffer::HBU_WRITE_ONLY));
    }
    if (mRenderOp.indexData->indexBuffer.isNull() || mIndexBufferSize != cmd_list->IdxBuffer.size())
    {
      mIndexBufferSize = cmd_list->IdxBuffer.size();
      sizeChanged = true;

      mRenderOp.indexData->indexBuffer=
        OgreV1::HardwareBufferManager::getSingleton().createIndexBuffer(OgreV1::HardwareIndexBuffer::IT_16BIT, mIndexBufferSize, OgreV1::HardwareBuffer::HBU_WRITE_ONLY);
    }

    const ImDrawVert* vertices = &cmd_list->VtxBuffer[0];
    const ImDrawIdx* indices = &cmd_list->IdxBuffer[0];

    // Copy all vertices
    ImDrawVert* vtx_dst = (ImDrawVert*)(bind->getBuffer(0)->lock(OgreV1::HardwareBuffer::HBL_DISCARD));
    ImDrawIdx* idx_dst = (ImDrawIdx*)(mRenderOp.indexData->indexBuffer->lock(OgreV1::HardwareBuffer::HBL_DISCARD));

    memcpy(vtx_dst, vertices, mVertexBufferSize * sizeof(ImDrawVert));
    memcpy(idx_dst, indices, mIndexBufferSize * sizeof(ImDrawIdx));

    mRenderOp.vertexData->vertexStart = 0;
    mRenderOp.vertexData->vertexCount =  cmd_list->VtxBuffer.size();
    mRenderOp.indexData->indexStart = 0;
    mRenderOp.indexData->indexCount =  cmd_list->IdxBuffer.size();

    bind->getBuffer(0)->unlock();
    mRenderOp.indexData->indexBuffer->unlock();
  }

  void ImGUIRenderable::updateVertexData(const ImDrawVert* vtxBuf, const ImDrawIdx* idxBuf, unsigned int vtxCount, unsigned int idxCount)
  {
    OgreV1::VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;
    bool sizeChanged = false;

    if (bind->getBindings().empty() || mVertexBufferSize != vtxCount)
    {
      mVertexBufferSize = vtxCount;

      sizeChanged = true;
      bind->setBinding(0, OgreV1::HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(ImDrawVert), mVertexBufferSize, OgreV1::HardwareBuffer::HBU_WRITE_ONLY));
    }
    if (mRenderOp.indexData->indexBuffer.isNull() || mIndexBufferSize != idxCount)
    {
      mIndexBufferSize = idxCount;

      sizeChanged = true;
      mRenderOp.indexData->indexBuffer =
        OgreV1::HardwareBufferManager::getSingleton().createIndexBuffer(OgreV1::HardwareIndexBuffer::IT_16BIT, mIndexBufferSize, OgreV1::HardwareBuffer::HBU_WRITE_ONLY);
    }

    // Copy all vertices
    ImDrawVert* vtxDst = (ImDrawVert*)(bind->getBuffer(0)->lock(OgreV1::HardwareBuffer::HBL_DISCARD));
    ImDrawIdx* idxDst = (ImDrawIdx*)(mRenderOp.indexData->indexBuffer->lock(OgreV1::HardwareBuffer::HBL_DISCARD));

    memcpy(vtxDst, vtxBuf, mVertexBufferSize * sizeof(ImDrawVert));
    memcpy(idxDst, idxBuf, mIndexBufferSize * sizeof(ImDrawIdx));

    mRenderOp.vertexData->vertexStart = 0;
    mRenderOp.vertexData->vertexCount = vtxCount;
    mRenderOp.indexData->indexStart = 0;
    mRenderOp.indexData->indexCount = idxCount;

    bind->getBuffer(0)->unlock();
    mRenderOp.indexData->indexBuffer->unlock();
  }

  //-----------------------------------------------------------------------------------

  void ImGUIRenderable::getWorldTransforms( Ogre::Matrix4* xform ) const
  {
    *xform = Ogre::Matrix4::IDENTITY;
  }

  //-----------------------------------------------------------------------------------

  void ImGUIRenderable::getRenderOperation(OgreV1::RenderOperation& op
#if OGRE_VERSION >= 0x020100
      , bool casterPass
#endif
  )
  {
    op = mRenderOp;
  }

  //-----------------------------------------------------------------------------------

  const Ogre::LightList& ImGUIRenderable::getLights(void) const
  {
    static const Ogre::LightList l;
    return l;
  }
}
