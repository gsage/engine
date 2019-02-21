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

#include "v2/ImguiMovableObject.h"
#include "v2/ImguiRenderable.h"
#include "ImguiDefinitions.h"
#include <OgreStringConverter.h>
#include <OgreSceneManager.h>

#include "Logger.h"

namespace Gsage {

  ImguiMovableObject::ImguiMovableObject(
      Ogre::IdType id,
      Ogre::ObjectMemoryManager* objectMemoryManager,
      Ogre::SceneManager* manager,
      Ogre::uint8 renderQueueGroup
  )
    : Ogre::MovableObject(id, objectMemoryManager, manager, renderQueueGroup)
  {
    setLocalAabb(Ogre::Aabb::BOX_INFINITE);
  }

  ImguiMovableObject::~ImguiMovableObject()
  {
  }

  void ImguiMovableObject::updateVertexData(const ImDrawList* drawList, int offset)
  {
    if(mDatablockName.empty()) {
      return;
    }

    ImGuiIO& io = ImGui::GetIO();
    Ogre::Matrix4 projMatrix(2.0f / io.DisplaySize.x, 0.0f, 0.0f, -1.0f,
        0.0f, -2.0f / io.DisplaySize.y, 0.0f, 1.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    const ImDrawVert* vtxBuf = drawList->VtxBuffer.Data;
    const ImDrawIdx* idxBuf = drawList->IdxBuffer.Data;

    unsigned int indexOffset = 0;

    for (int i = 0; i < drawList->CmdBuffer.Size; i++)
    {
      int index = offset + i;
      const ImDrawCmd *drawCmd = &drawList->CmdBuffer[i];

      if(drawList->VtxBuffer.Size == 0 || drawCmd->ElemCount == 0) {
        continue;
      }

      // add more renderables if needed
      if(mRenderables.size() <= index) {
        mRenderables.push_back(OGRE_NEW ImguiRenderable(
           mManager->getDestinationRenderSystem()->getVaoManager()
        ));
      }


      //update renderable vertex buffers
      static_cast<ImguiRenderable*>(mRenderables[index])->updateVertexData(
          vtxBuf,
          &idxBuf[indexOffset],
          drawList->VtxBuffer.Size,
          drawCmd->ElemCount
      );
      mRenderables[index]->setDatablock(mDatablockName);
      indexOffset += drawCmd->ElemCount;
    }
  }

  void ImguiMovableObject::setDatablock(const Ogre::String& name)
  {
    mDatablockName = name;
    // update datablock names in existing renderables
    Ogre::RenderableArray::iterator itRend = mRenderables.begin();
    Ogre::RenderableArray::iterator enRend = mRenderables.end();
    while( itRend != enRend ) {
      (*itRend)->setDatablock(name);
      ++itRend;
    }
  }

  //-----------------------------------------------------------------------------
  Ogre::String ImguiMovableObjectFactory::FACTORY_TYPE_NAME = "ImguiMovableObject";
  //-----------------------------------------------------------------------------
  const Ogre::String& ImguiMovableObjectFactory::getType(void) const
  {
    return FACTORY_TYPE_NAME;
  }
  //-----------------------------------------------------------------------------
  Ogre::MovableObject* ImguiMovableObjectFactory::createInstanceImpl(
      Ogre::IdType id,
      Ogre::ObjectMemoryManager *objectMemoryManager,
      Ogre::SceneManager *manager,
      const Ogre::NameValuePairList* params)
  {
    Ogre::uint8 renderQueue = RENDER_QUEUE_IMGUI;
    if (params != 0)
    {
      Ogre::NameValuePairList::const_iterator ni = params->find("renderQueue");
      if (ni != params->end())
      {
        renderQueue = Ogre::StringConverter::parseUnsignedInt(ni->second);
      }
    }

    return OGRE_NEW ImguiMovableObject(id, objectMemoryManager, manager, renderQueue);
  }
  //-----------------------------------------------------------------------------
  void ImguiMovableObjectFactory::destroyInstance( Ogre::MovableObject* obj)
  {
    OGRE_DELETE obj;
  }
}
