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

#include "v2/ImguiRendererV2.h"
#include <OgreSceneManager.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreCamera.h>

#include <OgreHlmsManager.h>
#include <OgreHlmsDatablock.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsTextureManager.h>
#include "ogre/v2/HlmsUnlit.h"
#include "ogre/v2/HlmsUnlitDatablock.h"

#include "v2/ImguiHlms.h"
#include "v2/ImguiHlmsDatablock.h"

#include "Engine.h"

namespace Gsage {

  ImguiRendererV2::ImguiRendererV2(Ogre::uint8 renderQueueGroup)
    : mRenderQueueGroup(renderQueueGroup)
  {
    mMovableObjectFactory = OGRE_NEW ImguiMovableObjectFactory();
  }

  ImguiRendererV2::~ImguiRendererV2()
  {
    Ogre::Root* root = Ogre::Root::getSingletonPtr();
    if(root && root->hasMovableObjectFactory(ImguiMovableObjectFactory::FACTORY_TYPE_NAME)) {
      root->removeMovableObjectFactory(mMovableObjectFactory);
    }
  }

  void ImguiRendererV2::initialize(Engine* engine, lua_State* L)
  {
    ImguiOgreRenderer::initialize(engine, L);

    LOG(INFO) << "Changing render queue " << int(mRenderQueueGroup) << " rendering mode to FAST. Make sure that you don't have any V1 rendering relying on that queue";
    mSceneMgr->getRenderQueue()->setRenderQueueMode(mRenderQueueGroup, Ogre::RenderQueue::Modes::FAST);
    Ogre::Root* root = Ogre::Root::getSingletonPtr();
    if(!root->hasMovableObjectFactory(ImguiMovableObjectFactory::FACTORY_TYPE_NAME)) {
      root->addMovableObjectFactory(mMovableObjectFactory);
    }
  }

  void ImguiRendererV2::updateVertexData(Ogre::Viewport* vp, ImVec2 displaySize)
  {
    Ogre::Matrix4 projMatrix(2.0f / displaySize.x, 0.0f, 0.0f, -1.0f,
        0.0f, -2.0f / displaySize.y, 0.0f, 1.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
    mHlms->setUseCustomProjectionMatrix(
        IMGUI_MATERIAL_NAME,
        projMatrix
    );

    ImDrawData *drawData = ImGui::GetDrawData();

    //iterate through all lists (at the moment every window has its own)
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
      const ImDrawList* drawList = drawData->CmdLists[n];

      if(mImguiMovableObjects.size() == 0) {
        mImguiMovableObjects.push_back(createImguiMovableObject());
      }

      mImguiMovableObjects[0]->updateVertexData(drawList, n);
    }

    while(mImguiMovableObjects.size() > drawData->CmdListsCount) {
      // TODO: verify that it won't crash when underlying window is closed
      mSceneMgr->destroyMovableObject(mImguiMovableObjects.back());
      mImguiMovableObjects.pop_back();
    }
  }

  void ImguiRendererV2::createMaterial()
  {
    // create and configure macroblock
    Ogre::HlmsMacroblock macroblock;
    macroblock.mCullMode = Ogre::CULL_NONE;
    macroblock.mDepthFunc = Ogre::CMPF_ALWAYS_PASS;
    macroblock.mDepthWrite = false;
    macroblock.mDepthCheck = false;
    macroblock.mAllowGlobalDefaults = false;
    macroblock.mScissorTestEnabled = true;

    // create and configure blendblock
    Ogre::HlmsBlendblock blendblock;
    blendblock.setBlendType(Ogre::SBT_TRANSPARENT_ALPHA);
    blendblock.mSeparateBlend = true;
    blendblock.mAllowGlobalDefaults = false;
    blendblock.mIsTransparent = true;
    blendblock.mBlendOperation = Ogre::SBO_ADD;
    blendblock.mBlendOperationAlpha = Ogre::SBO_ADD;
    blendblock.mSourceBlendFactor = Ogre::SBF_SOURCE_ALPHA;
    blendblock.mDestBlendFactor = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;
    blendblock.mSourceBlendFactorAlpha = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;
    blendblock.mDestBlendFactorAlpha = Ogre::SBF_ZERO;

    const std::string name = IMGUI_MATERIAL_NAME;

    // register imgui hlms

    Ogre::HlmsManager *hlmsManager = Ogre::Root::getSingletonPtr()->getHlmsManager();
    HlmsUnlit *hlmsUnlit = static_cast<HlmsUnlit*>(hlmsManager->getHlms(Ogre::HLMS_UNLIT));
    HlmsUnlitDatablock *datablock = static_cast<HlmsUnlitDatablock*>(hlmsUnlit->getDatablock(name));
    mHlms = hlmsUnlit;

    if(!datablock) {
      datablock = static_cast<HlmsUnlitDatablock*>(
        hlmsUnlit->createDatablock(name,
          name,
          macroblock,
          blendblock,
          Ogre::HlmsParamVec()));
    }

    datablock->setTexture(Ogre::PBSM_DIFFUSE, 0, mFontTex);
    datablock->setTextureUvSource(0, 0);
  }

  ImguiMovableObject* ImguiRendererV2::createImguiMovableObject()
  {
    Ogre::ObjectMemoryManager& memoryManager = mSceneMgr->_getEntityMemoryManager(Ogre::SCENE_DYNAMIC);
    Ogre::String factoryName = ImguiMovableObjectFactory::FACTORY_TYPE_NAME;

    Ogre::NameValuePairList params;
    params["renderQueue"] = Ogre::StringConverter::toString(mRenderQueueGroup);
    ImguiMovableObject* mo = static_cast<ImguiMovableObject*>(
      mSceneMgr->createMovableObject(factoryName, &memoryManager, &params)
    );
    Ogre::SceneNode* rootNode = mSceneMgr->getRootSceneNode();
    if(!rootNode) {
      OGRE_EXCEPT( Ogre::Exception::ERR_INVALID_STATE,
          "Failed to add ImguiMovableObject to scene. "
          "Root SceneNode is NULL.",
          "ImguiOgreInterface::createImguiMovableObject" );
    }

    // add movable object to the root node
    rootNode->attachObject(mo);
    mo->setDatablock(IMGUI_MATERIAL_NAME);
    return mo;
  }
}
