/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Artem Chernyshev

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

#include "ImGuiConverters.h"
#include <OgreMaterialManager.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreSubMesh.h>
#include <OgreTexture.h>
#include <OgreTextureManager.h>
#include <OgreString.h>
#include <OgreStringConverter.h>
#include <OgreViewport.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreUnifiedHighLevelGpuProgram.h>
#include <OgreRoot.h>
#include <OgreTechnique.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRenderTarget.h>

#include "Definitions.h"

#include "v1/ImguiRenderable.h"
#include "v1/ImguiRendererV1.h"
#include "Engine.h"
#include <imgui.h>
#include "ImguiDefinitions.h"

#if OGRE_VERSION >= 0x020100
#include <RenderSystems/GL3Plus/OgreGL3PlusRenderSystem.h>
#include <CommandBuffer/OgreCbDrawCall.h>
#include <CommandBuffer/OgreCbPipelineStateObject.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <OgreHlmsPbsPrerequisites.h>
#include <OgreHlms.h>

#include "v2/ViewportRenderable.h"
#endif

#if GSAGE_PLATFORM == GSAGE_APPLE
#include "OSX/Utils.h"
#endif


namespace Gsage {
#if OGRE_VERSION >= 0x020100
  const Ogre::HlmsCache c_dummyCache(0, Ogre::HLMS_MAX, Ogre::HlmsPso());
#endif

  ImguiRendererV1::ImguiRendererV1()
    : mTexUnit(0)
#if OGRE_VERSION >= 0x020100
    , mPsoCache(0)
#endif
  {
  }

  ImguiRendererV1::~ImguiRendererV1()
  {
#if OGRE_VERSION >= 0x020100
    if(mPsoCache != 0) {
      delete mPsoCache;
    }

#endif
    while(mRenderables.size() > 0)
    {
      delete mRenderables.back();
      mRenderables.pop_back();
    }
  }

  void ImguiRendererV1::initialize(Engine* engine, lua_State* L)
  {
    ImguiOgreRenderer::initialize(engine, L);
#if OGRE_VERSION >= 0x020100
    mPsoCache = OGRE_NEW Ogre::PsoCacheHelper(mSceneMgr->getDestinationRenderSystem());
    sol::state_view lua(L);
#if GSAGE_PLATFORM == GSAGE_APPLE
    lua["imgui"]["Scale"] = GetScreenScaleFactor();
#endif
#endif
  }

  void ImguiRendererV1::updateVertexData(Ogre::Viewport* vp, ImVec2 displaySize)
  {
#if OGRE_VERSION >= 0x020100
    if(!mFontTex) {
#else
    if(mFontTex.isNull()) {
#endif
      return;
    }

    Ogre::Matrix4 projMatrix(2.0f / displaySize.x, 0.0f, 0.0f, -1.0f,
        0.0f, -2.0f / displaySize.y, 0.0f, 1.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

#if OGRE_VERSION_MAJOR == 2
    const Ogre::HlmsBlendblock *blendblock = mPass->getBlendblock();
    const Ogre::HlmsMacroblock *macroblock = mPass->getMacroblock();
#if OGRE_VERSION_MINOR == 0
    mSceneMgr->getDestinationRenderSystem()->_setHlmsBlendblock(blendblock);
    mSceneMgr->getDestinationRenderSystem()->_setHlmsMacroblock(macroblock);
#else
    mPsoCache->clearState();

    mPsoCache->setBlendblock(blendblock);
    mPsoCache->setMacroblock(macroblock);
    auto vertexShader = mPass->getVertexProgram();
    mPsoCache->setVertexShader(vertexShader);
    auto pixelShader = mPass->getFragmentProgram();
    mPsoCache->setPixelShader(pixelShader);

    mPsoCache->setRenderTarget(vp->getTarget());
#endif
#endif
    mPass->getVertexProgramParameters()->setNamedConstant("ProjectionMatrix", projMatrix);


    ImDrawData *drawData = ImGui::GetDrawData();
    int numberDraws = 0;

    //iterate through all lists (at the moment every window has its own)
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
      const ImDrawList* drawList = drawData->CmdLists[n];
      const ImDrawVert* vtxBuf = drawList->VtxBuffer.Data;
      const ImDrawIdx* idxBuf = drawList->IdxBuffer.Data;

      unsigned int startIdx = 0;

      int current = 0;

      for (int i = 0; i < drawList->CmdBuffer.Size; i++)
      {
        const ImDrawCmd *drawCmd = &drawList->CmdBuffer[current];
#if OGRE_VERSION < 0x020100
        if (drawCmd->UserCallbackData != NULL){
          OgreV1::Rectangle2D* rect = static_cast<OgreV1::Rectangle2D*>(drawCmd->UserCallbackData);
          mSceneMgr->_injectRenderWithPass(
              rect->getMaterial()->getTechnique(0)->getPass(0),
              rect,
              false,
              false
          );
          continue;
        }
#endif
        //create renderables if necessary
        if (numberDraws >= mRenderables.size())
        {
#if OGRE_VERSION >= 0x020100
          mHashes.push_back(mPsoCache->getRenderableHash());
#endif
          mRenderables.push_back(new ImGUIRenderable());
        }

        ImGUIRenderable* renderable = mRenderables[numberDraws];
        Ogre::TextureUnitState* texUnitState;

#if OGRE_VERSION >= 0x020100
        if (drawCmd->UserCallbackData != NULL){
          ViewportRenderData* viewport = static_cast<ViewportRenderData*>(drawCmd->UserCallbackData);
          renderable->updateVertexData(
              &viewport->mVertexBuffer[0],
              &viewport->mIndexBuffer[0],
              4,
              6
          );
          if((viewport->mTexUnitState == 0 && !viewport->mTextureName.empty()) || viewport->mDirty) {
            Ogre::TexturePtr texture = viewport->getRenderTexture();
            if(!texture.isNull()) {
              viewport->mTexUnitState = mPass->createTextureUnitState();
              viewport->mTexUnitState->setTexture(texture);
              viewport->mDirty = false;
            }
          }

          if(viewport->mTexUnitState) {
            texUnitState = viewport->mTexUnitState;
          }
        } else {
#endif
          texUnitState = mTexUnit;
          renderable->updateVertexData(vtxBuf, &idxBuf[startIdx], drawList->VtxBuffer.Size, drawCmd->ElemCount);
#if OGRE_VERSION >= 0x020100
        }
#endif

        //set scissoring
        int vpLeft, vpTop, vpWidth, vpHeight;
        vp->getActualDimensions(vpLeft, vpTop, vpWidth, vpHeight);
        float factor = 1.0f;
#if GSAGE_PLATFORM == GSAGE_APPLE
        factor = GetScreenScaleFactor();
#endif

        int scLeft = drawCmd->ClipRect.x;
        int scTop = drawCmd->ClipRect.y;
        int scRight = drawCmd->ClipRect.z;
        int scBottom = drawCmd->ClipRect.w;

        scLeft = scLeft < 0 ? 0 : (scLeft > vpWidth ? vpWidth : scLeft);
        scRight = scRight < 0 ? 0 : (scRight > vpWidth ? vpWidth : scRight);
        scTop = scTop < 0 ? 0 : (scTop > vpHeight ? vpHeight : scTop);
        scBottom = scBottom < 0 ? 0 : (scBottom > vpHeight ? vpHeight : scBottom);

#if OGRE_VERSION_MAJOR == 1
        mSceneMgr->getDestinationRenderSystem()->setScissorTest(true, scLeft, scTop, scRight, scBottom);
#else
        float left = (float)scLeft / (float)vpWidth * factor;
        float top = (float)scTop / (float)vpHeight * factor;
        float width = (float)(scRight - scLeft) / (float)vpWidth * factor;
        float height = (float)(scBottom - scTop) / (float)vpHeight * factor;

        vp->setScissors(std::max(0.0f, left), std::max(0.0f, top), std::min(1.0f, width), std::min(1.0f, height));
        mSceneMgr->getDestinationRenderSystem()->_setViewport(vp);
#endif
        //render the object
#if OGRE_VERSION_MAJOR == 1
        mSceneMgr->_injectRenderWithPass(mPass, renderable, false, false);
#elif OGRE_VERSION_MAJOR == 2

#if OGRE_VERSION_MINOR == 0
        mSceneMgr->_injectRenderWithPass(mPass, renderable, 0, false, false);
#else
        Ogre::v1::RenderOperation renderOp;
        renderable->getRenderOperation(renderOp, false);
        Ogre::VertexElement2VecVec vertexData = renderable->mVertexElement2VecVec;

        mPsoCache->setVertexFormat(vertexData,
            renderOp.operationType,
            true);

        Ogre::HlmsPso *pso = mPsoCache->getPso(mHashes[numberDraws], true);
        mSceneMgr->getDestinationRenderSystem()->_setPipelineStateObject(pso);
        mSceneMgr->getDestinationRenderSystem()->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM,
            mPass->getVertexProgramParameters(), Ogre::GPV_GLOBAL);
        if(texUnitState) {
          mSceneMgr->getDestinationRenderSystem()->_setTextureUnitSettings(0, *texUnitState);
        }

        Ogre::v1::CbRenderOp op(renderOp);
        mSceneMgr->getDestinationRenderSystem()->_setRenderOperation(&op);
        mSceneMgr->getDestinationRenderSystem()->_render(renderOp);
#endif
#endif
        //increase start index of indexbuffer
        startIdx += drawCmd->ElemCount;
        numberDraws++;
        current++;
      }
    }

    //reset Scissors
#if OGRE_VERSION_MAJOR == 1
    mSceneMgr->getDestinationRenderSystem()->setScissorTest(false);
#elif OGRE_VERSION_MAJOR == 2
    vp->setScissors(0, 0, 1, 1);
    mSceneMgr->getDestinationRenderSystem()->_setViewport(vp);
#endif

    //delete unused renderables
    while (mRenderables.size() > numberDraws)
    {
      delete mRenderables.back();
      mRenderables.pop_back();
    }
  }

  //-----------------------------------------------------------------------------------

  void ImguiRendererV1::createMaterial()
  {
    static const char* vertexShaderSrcD3D11 =
    {
      "cbuffer vertexBuffer : register(b0) \n"
        "{\n"
        "float4x4 ProjectionMatrix; \n"
        "};\n"
        "struct VS_INPUT\n"
        "{\n"
        "float2 pos : POSITION;\n"
        "float4 col : COLOR0;\n"
        "float2 uv  : TEXCOORD0;\n"
        "};\n"
        "struct PS_INPUT\n"
        "{\n"
        "float4 pos : SV_POSITION;\n"
        "float4 col : COLOR0;\n"
        "float2 uv  : TEXCOORD0;\n"
        "};\n"
        "PS_INPUT main(VS_INPUT input)\n"
        "{\n"
        "PS_INPUT output;\n"
        "output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\n"
        "output.col = input.col;\n"
        "output.uv  = input.uv;\n"
        "return output;\n"
        "}"
    };

    static const char* pixelShaderSrcD3D11 =
    {
      "struct PS_INPUT\n"
        "{\n"
        "float4 pos : SV_POSITION;\n"
        "float4 col : COLOR0;\n"
        "float2 uv  : TEXCOORD0;\n"
        "};\n"
        "sampler sampler0;\n"
        "Texture2D texture0;\n"
        "\n"
        "float4 main(PS_INPUT input) : SV_Target\n"
        "{\n"
        "float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \n"
        "return out_col; \n"
        "}"
    };
    static const char* vertexShaderSrcD3D9 =
    {
      "uniform float4x4 ProjectionMatrix; \n"
        "struct VS_INPUT\n"
        "{\n"
        "float2 pos : POSITION;\n"
        "float4 col : COLOR0;\n"
        "float2 uv  : TEXCOORD0;\n"
        "};\n"
        "struct PS_INPUT\n"
        "{\n"
        "float4 pos : POSITION;\n"
        "float4 col : COLOR0;\n"
        "float2 uv  : TEXCOORD0;\n"
        "};\n"
        "PS_INPUT main(VS_INPUT input)\n"
        "{\n"
        "PS_INPUT output;\n"
        "output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\n"
        "output.col = input.col;\n"
        "output.uv  = input.uv;\n"
        "return output;\n"
        "}"
    };

    static const char* pixelShaderSrcSrcD3D9 =
    {
      "struct PS_INPUT\n"
        "{\n"
        "float4 pos : SV_POSITION;\n"
        "float4 col : COLOR0;\n"
        "float2 uv  : TEXCOORD0;\n"
        "};\n"
        "sampler2D sampler0;\n"
        "\n"
        "float4 main(PS_INPUT input) : SV_Target\n"
        "{\n"
        "float4 out_col = input.col.bgra * tex2D(sampler0, input.uv); \n"
        "return out_col; \n"
        "}"
    };

    static const char* vertexShaderSrcGLSL150 =
    {
      "#version 150\n"
        "uniform mat4 ProjectionMatrix; \n"
        "in vec2 vertex;\n"
        "in vec2 uv0;\n"
        "in vec4 colour;\n"
        "out vec2 Texcoord;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "gl_Position = ProjectionMatrix* vec4(vertex.xy, 0.f, 1.f);\n"
        "Texcoord  = uv0;\n"
        "col = colour;\n"
        "}"
    };

    static const char* vertexShaderSrcGLSL120 =
    {
      "#version 120\n"
        "uniform mat4 ProjectionMatrix; \n"
        "attribute vec2 vertex;\n"
        "attribute vec2 uv0;\n"
        "attribute vec4 colour;\n"
        "varying vec2 Texcoord;\n"
        "varying vec4 col;\n"
        "void main()\n"
        "{\n"
        "gl_Position = ProjectionMatrix* vec4(vertex.xy, 0.f, 1.f);\n"
        "Texcoord  = uv0;\n"
        "col = colour;\n"
        "}"
    };

    static const char* pixelShaderSrcGLSL150 =
    {
      "#version 150\n"
        "in vec2 Texcoord;\n"
        "in vec4 col;\n"
        "uniform sampler2D sampler0;\n"
        "out vec4 out_col;\n"
        "void main()\n"
        "{\n"
        "out_col = col * texture(sampler0, Texcoord); \n"
        "}"
    };

    static const char* pixelShaderSrcGLSL120 =
    {
      "#version 120\n"
        "varying vec2 Texcoord;\n"
        "varying vec4 col;\n"
        "uniform sampler2D sampler0;\n"
        "varying vec4 out_col;\n"
        "void main()\n"
        "{\n"
        "gl_FragColor = col * texture2D(sampler0, Texcoord); \n"
        "}"
    };

    static const char* vertexShaderSrcMetal =
    {
        "#include <metal_stdlib> \n"
        "using namespace metal; \n"

        "struct VS_INPUT\n"
        "{\n"
          "float2 position	[[attribute(VES_POSITION)]];\n"
          "float2 uv0		[[attribute(VES_TEXTURE_COORDINATES0)]];\n"
          "float4 color		[[attribute(VES_DIFFUSE)]];\n"
        "};\n"

        "struct PS_INPUT\n"
        "{\n"
          "float2 uv0;\n"

          "float4 gl_Position [[position]];\n"
          "float4 color;\n"
        "};\n"

        "vertex PS_INPUT main_metal\n"
        "(\n"
          "VS_INPUT input [[stage_in]],\n"

          "constant float4x4 &ProjectionMatrix [[buffer(PARAMETER_SLOT)]]\n"
        ")\n"
        "{\n"
          "PS_INPUT outVs;\n"
          "outVs.gl_Position	= ( ProjectionMatrix * float4(input.position.xy, 0.f, 1.f) ).xyzw;\n"
          "outVs.uv0			= input.uv0;\n"
          "outVs.color = input.color;\n"

          "return outVs;\n"
        "}"
    };

    static const char* pixelShaderSrcMetal =
    {
      "#include <metal_stdlib>\n"
      "using namespace metal;\n"
      "struct PS_INPUT\n"
      "{\n"
        "float2 uv0;\n"
        "float4 color;\n"
      "};\n"
      "fragment float4 main_metal\n"
      "(\n"
        "PS_INPUT inPs [[stage_in]],\n"
        "texture2d<float>	tex	[[texture(0)]],\n"
        "sampler				samplerState	[[sampler(0)]]\n"
      ")\n"
      "{\n"
          "float4 col = inPs.color * tex.sample(samplerState, inPs.uv0);\n"
          "return col;\n"
      "}\n"
    };

    //create the default shadows material
    Ogre::HighLevelGpuProgramManager& mgr = Ogre::HighLevelGpuProgramManager::getSingleton();

    Ogre::HighLevelGpuProgramPtr vertexShaderUnified = mgr.getByName("imgui/VP");
    Ogre::HighLevelGpuProgramPtr pixelShaderUnified = mgr.getByName("imgui/FP");

    Ogre::HighLevelGpuProgramPtr vertexShaderD3D11 = mgr.getByName("imgui/VP/D3D11");
    Ogre::HighLevelGpuProgramPtr pixelShaderD3D11 = mgr.getByName("imgui/FP/D3D11");

    Ogre::HighLevelGpuProgramPtr vertexShaderD3D9 = mgr.getByName("imgui/VP/D3D9");
    Ogre::HighLevelGpuProgramPtr pixelShaderD3D9 = mgr.getByName("imgui/FP/D3D9");

    Ogre::HighLevelGpuProgramPtr vertexShaderGL = mgr.getByName("imgui/VP/GL");
    Ogre::HighLevelGpuProgramPtr pixelShaderGL = mgr.getByName("imgui/FP/GL");

    Ogre::HighLevelGpuProgramPtr vertexShaderMetal = mgr.getByName("imgui/VP/Metal");
    Ogre::HighLevelGpuProgramPtr pixelShaderMetal = mgr.getByName("imgui/FP/Metal");

    if(vertexShaderUnified.isNull())
    {
      vertexShaderUnified = mgr.createProgram(
          "imgui/VP",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
          "unified",
          Ogre::GPT_VERTEX_PROGRAM);
    }

    if(pixelShaderUnified.isNull())
    {
      pixelShaderUnified = mgr.createProgram(
          "imgui/FP",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
          "unified",
          Ogre::GPT_FRAGMENT_PROGRAM);
    }

    Ogre::UnifiedHighLevelGpuProgram* vertexShaderPtr = static_cast<Ogre::UnifiedHighLevelGpuProgram*>(vertexShaderUnified.get());
    Ogre::UnifiedHighLevelGpuProgram* pixelShaderPtr = static_cast<Ogre::UnifiedHighLevelGpuProgram*>(pixelShaderUnified.get());

    if (vertexShaderD3D11.isNull())
    {
      vertexShaderD3D11 = mgr.createProgram("imgui/VP/D3D11",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
          "hlsl",
          Ogre::GPT_VERTEX_PROGRAM);
      vertexShaderD3D11->setParameter("target", "vs_4_0");
      vertexShaderD3D11->setParameter("entry_point", "main");
      vertexShaderD3D11->setSource(vertexShaderSrcD3D11);
      vertexShaderD3D11->load();

      vertexShaderPtr->addDelegateProgram(vertexShaderD3D11->getName());
    }

    if (pixelShaderD3D11.isNull())
    {
      pixelShaderD3D11 = mgr.createProgram(
          "imgui/FP/D3D11",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
          "hlsl",
          Ogre::GPT_FRAGMENT_PROGRAM);
      pixelShaderD3D11->setParameter("target", "ps_4_0");
      pixelShaderD3D11->setParameter("entry_point", "main");
      pixelShaderD3D11->setSource(pixelShaderSrcD3D11);
      pixelShaderD3D11->load();

      pixelShaderPtr->addDelegateProgram(pixelShaderD3D11->getName());
    }

    if (vertexShaderD3D9.isNull())
    {
      vertexShaderD3D9 = mgr.createProgram(
          "imgui/VP/D3D9",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
          "hlsl",
          Ogre::GPT_VERTEX_PROGRAM);
      vertexShaderD3D9->setParameter("target", "vs_2_0");
      vertexShaderD3D9->setParameter("entry_point", "main");
      vertexShaderD3D9->setSource(vertexShaderSrcD3D9);
      vertexShaderD3D9->load();

      vertexShaderPtr->addDelegateProgram(vertexShaderD3D9->getName());
    }

    if (pixelShaderD3D9.isNull())
    {
      pixelShaderD3D9 = mgr.createProgram(
          "imgui/FP/D3D9",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
          "hlsl",
          Ogre::GPT_FRAGMENT_PROGRAM);
      pixelShaderD3D9->setParameter("target", "ps_2_0");
      pixelShaderD3D9->setParameter("entry_point", "main");
      pixelShaderD3D9->setSource(pixelShaderSrcSrcD3D9);
      pixelShaderD3D9->load();

      pixelShaderPtr->addDelegateProgram(pixelShaderD3D9->getName());
    }

    Ogre::RenderSystem* render = Ogre::Root::getSingletonPtr()->getRenderSystem();

    bool useGLSL120 = render->getDriverVersion().major < 3 || OGRE_VERSION_MAJOR == 1;

    if (vertexShaderGL.isNull())
    {
      vertexShaderGL = mgr.createProgram(
          "imgui/VP/GL",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
          "glsl",
          Ogre::GPT_VERTEX_PROGRAM);

      // select appropriate vertex shader for opengl 2
      if(useGLSL120) {
        vertexShaderGL->setSource(vertexShaderSrcGLSL120);
      } else {
        vertexShaderGL->setSource(vertexShaderSrcGLSL150);
      }

      vertexShaderGL->load();
      vertexShaderPtr->addDelegateProgram(vertexShaderGL->getName());
    }

    if (pixelShaderGL.isNull())
    {
      pixelShaderGL = mgr.createProgram(
          "imgui/FP/GL",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
          "glsl",
          Ogre::GPT_FRAGMENT_PROGRAM);
      // select appropriate pixel shader for opengl 2
      if(useGLSL120) {
        pixelShaderGL->setSource(pixelShaderSrcGLSL120);
      } else {
        pixelShaderGL->setSource(pixelShaderSrcGLSL150);
      }
      pixelShaderGL->load();
      pixelShaderGL->setParameter("sampler0", "int 0");

      pixelShaderPtr->addDelegateProgram(pixelShaderGL->getName());
    }

    if (vertexShaderMetal.isNull())
    {
      vertexShaderMetal = mgr.createProgram(
          "imgui/VP/Metal",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
          "metal",
          Ogre::GPT_VERTEX_PROGRAM);

      vertexShaderMetal->setSource(vertexShaderSrcMetal);
      vertexShaderMetal->load();

      vertexShaderPtr->addDelegateProgram(vertexShaderMetal->getName());
    }

    if (pixelShaderMetal.isNull())
    {
      pixelShaderMetal = mgr.createProgram(
          "imgui/FP/Metal",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
          "metal",
          Ogre::GPT_FRAGMENT_PROGRAM);
      pixelShaderMetal->setSource(pixelShaderSrcMetal);
      pixelShaderMetal->setParameter("shader_reflection_pair_hint", "imgui/VP/Metal");
      pixelShaderMetal->load();

      pixelShaderPtr->addDelegateProgram(pixelShaderMetal->getName());
    }

    Ogre::MaterialPtr imguiMaterial = Ogre::MaterialManager::getSingleton().create("imgui/material", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mPass = imguiMaterial->getTechnique(0)->getPass(0);
    mPass->setFragmentProgram("imgui/FP");
    mPass->setVertexProgram("imgui/VP");
#if OGRE_VERSION < 0x020100
    mPass->setCullingMode(Ogre::CULL_NONE);
    mPass->setDepthFunction(Ogre::CMPF_ALWAYS_PASS);
    mPass->setLightingEnabled(false);
    mPass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    mPass->setSeparateSceneBlendingOperation(Ogre::SBO_ADD, Ogre::SBO_ADD);
    mPass->setSeparateSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA, Ogre::SBF_ZERO);
#else
    Ogre::HlmsMacroblock macroblock;
    macroblock.mCullMode = Ogre::CULL_NONE;
    macroblock.mDepthFunc = Ogre::CMPF_ALWAYS_PASS;
    macroblock.mDepthWrite = false;
    macroblock.mDepthCheck = false;
    macroblock.mAllowGlobalDefaults = false;
    macroblock.mScissorTestEnabled = true;
    mPass->setMacroblock(macroblock);

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
    mPass->setBlendblock(blendblock);
#endif
  }

  void ImguiRendererV1::updateFontTexture()
  {
    Ogre::TextureUnitState* texUnit = mPass->createTextureUnitState();
    texUnit->setTexture(mFontTex);
#if OGRE_VERSION < 0x020100
    texUnit->setTextureFiltering(Ogre::TFO_NONE);
#else
    Ogre::HlmsSamplerblock samplerblock;
    samplerblock.setFiltering(Ogre::TFO_NONE);
    texUnit->setSamplerblock(samplerblock);
#endif
    mTexUnit = texUnit;
  }
}
