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

#include "ogre/ImguiRenderable.h"
#include "ogre/ImguiOgreWrapper.h"
#include "Engine.h"
#include <imgui.h>

#define RENDER_QUEUE_IMGUI 101

namespace Gsage {

  ImguiOgreWrapper::ImguiOgreWrapper(Engine* engine)
    : mLastRenderedFrame(-1)
    , mEngine(engine)
    , mFrameEnded(true)
  {
    addEventListener(mEngine, RenderEvent::RENDER_QUEUE_STARTED, &ImguiOgreWrapper::render);
    addEventListener(mEngine, RenderEvent::RENDER_QUEUE_ENDED, &ImguiOgreWrapper::renderQueueEnded);
    mSceneMgr = mEngine->getSystem<OgreRenderSystem>()->getSceneManager();
    if(!mSceneMgr)
      return;
    createFontTexture();
    createMaterial();
  }

  ImguiOgreWrapper::~ImguiOgreWrapper()
  {
    while(mRenderables.size()>0)
    {
      delete mRenderables.back();
      mRenderables.pop_back();
    }
  }

  void ImguiOgreWrapper::updateVertexData(Ogre::Viewport* vp)
  {
    int currentFrame = ImGui::GetFrameCount();
    if(currentFrame == mLastRenderedFrame)
    {
      return;
    }
    mLastRenderedFrame = currentFrame;

    ImGui::Render();

    ImGuiIO& io = ImGui::GetIO();
    Ogre::Matrix4 projMatrix(2.0f / io.DisplaySize.x, 0.0f, 0.0f, -1.0f,
        0.0f, -2.0f / io.DisplaySize.y, 0.0f, 1.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

#if OGRE_VERSION_MAJOR == 2
    const Ogre::HlmsBlendblock *blendblock = mPass->getBlendblock();
    const Ogre::HlmsMacroblock *macroblock = mPass->getMacroblock();
    mSceneMgr->getDestinationRenderSystem()->_setHlmsBlendblock(blendblock);
    mSceneMgr->getDestinationRenderSystem()->_setHlmsMacroblock(macroblock);
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

      for (int i = 0; i < drawList->CmdBuffer.Size; i++)
      {
        //create renderables if necessary
        if (i >= mRenderables.size())
        {
          mRenderables.push_back(new ImGUIRenderable());
        }

        //update their vertex buffers
        const ImDrawCmd *drawCmd = &drawList->CmdBuffer[i];
        Ogre::Renderable* renderable;

        if (drawCmd->UserCallbackData != NULL){
          renderable = static_cast<Ogre::Rectangle2D*>(drawCmd->UserCallbackData);
          mSceneMgr->_injectRenderWithPass(renderable->getMaterial()->getTechnique(0)->getPass(0), renderable, false, false);
          continue;
        } else {
          mRenderables[i]->updateVertexData(vtxBuf, &idxBuf[startIdx], drawList->VtxBuffer.Size, drawCmd->ElemCount);
          renderable = mRenderables[i];
        }


        //set scissoring
        int vpLeft, vpTop, vpWidth, vpHeight;
        vp->getActualDimensions(vpLeft, vpTop, vpWidth, vpHeight);

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
#elif OGRE_VERSION_MAJOR == 2
        float left = (float)scLeft / (float)vpWidth;
        float top = (float)scTop / (float)vpHeight;
        float width = (float)(scRight - scLeft) / (float)vpWidth;
        float height = (float)(scBottom - scTop) / (float)vpHeight;
        vp->setScissors(left, top, width, height);
        mSceneMgr->getDestinationRenderSystem()->_setViewport(vp);
#endif

        //render the object
#if OGRE_VERSION_MAJOR == 1
        mSceneMgr->_injectRenderWithPass(mPass, renderable, false, false);
#elif OGRE_VERSION_MAJOR == 2
        mSceneMgr->_injectRenderWithPass(mPass, renderable, 0, false, false);
#endif

        //increase start index of indexbuffer
        startIdx += drawCmd->ElemCount;
        numberDraws++;
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

  bool ImguiOgreWrapper::renderQueueEnded(EventDispatcher* sender, const Event& e)
  {
    RenderEvent event = static_cast<const RenderEvent&>(e);
    if(event.queueID != RENDER_QUEUE_IMGUI)
    {
      return true;
    }

    Ogre::Viewport* vp = event.renderTarget->getViewport();

    if(vp == NULL || !vp->getTarget()->isPrimary() || !vp->getOverlaysEnabled())
      return true;

    if(mFrameEnded) {
      return true;
    }

    mFrameEnded = true;
    updateVertexData(vp);
    return true;
  }

  //-----------------------------------------------------------------------------------

  void ImguiOgreWrapper::createMaterial()
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
        "out vec4 ocol;\n"
        "void main()\n"
        "{\n"
        "gl_Position = ProjectionMatrix* vec4(vertex.xy, 0.f, 1.f);\n"
        "Texcoord  = uv0;\n"
        "ocol = colour;\n"
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

    bool useGLSL120 = render->getDriverVersion().major < 3 or OGRE_VERSION_MAJOR == 1;

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

    Ogre::MaterialPtr imguiMaterial = Ogre::MaterialManager::getSingleton().create("imgui/material", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mPass = imguiMaterial->getTechnique(0)->getPass(0);
    mPass->setFragmentProgram("imgui/FP");
    mPass->setVertexProgram("imgui/VP");
    mPass->setCullingMode(Ogre::CULL_NONE);
    mPass->setDepthFunction(Ogre::CMPF_ALWAYS_PASS);
    mPass->setLightingEnabled(false);
    mPass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    mPass->setSeparateSceneBlendingOperation(Ogre::SBO_ADD, Ogre::SBO_ADD);
    mPass->setSeparateSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA, Ogre::SBF_ZERO);

    Ogre::TextureUnitState* texUnit = mPass->createTextureUnitState();
    texUnit->setTexture(mFontTex);
    texUnit->setTextureFiltering(Ogre::TFO_NONE);
  }

  void ImguiOgreWrapper::createFontTexture()
  {
    std::string workdir = mEngine->env().get("workdir", ".");
    auto additionalFonts = mEngine->settings().get<DataProxy>("imgui.fonts");

    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();

    unsigned char* pixels;
    int width, height;
    std::vector<ImFont*> fonts;

    if(additionalFonts.second) {
      for(auto pair : additionalFonts.first) {
        auto file = pair.second.get<std::string>("file");
        if(!file.second) {
          LOG(ERROR) << "Malformed font syntax, \'file\' field is mandatory";
          continue;
        }

        auto size = pair.second.get<float>("size");
        if(!size.second) {
          LOG(ERROR) << "Malformed font syntax, \'size\' field is mandatory";
          continue;
        }

        ImFontConfig config;
        config.OversampleH = pair.second.get("oversampleH", 0);
        config.OversampleV = pair.second.get("oversampleV", 0);

        ImFont* pFont = io.Fonts->AddFontFromFileTTF((workdir + GSAGE_PATH_SEPARATOR + file.first).c_str(), size.first, &config);
        if(pair.first == "default") {
          io.Fonts->AddFontDefault();
        }
        fonts.push_back(pFont);
      }
    }

    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    mFontTex = Ogre::TextureManager::getSingleton().createManual("ImguiFontTex",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D,
        width,
        height,
        1,
        1,
        Ogre::PF_R8G8B8A8
    );

    const Ogre::PixelBox& lockBox = mFontTex->getBuffer()->lock(Ogre::Image::Box(0, 0, width, height), Ogre::HardwareBuffer::HBL_DISCARD);
    size_t texDepth = Ogre::PixelUtil::getNumElemBytes(lockBox.format);

    memcpy(lockBox.data, pixels, width * height * texDepth);
    mFontTex->getBuffer()->unlock();

    io.MouseDrawCursor = true;
    OgreRenderSystem* render = mEngine->getSystem<OgreRenderSystem>();
    io.DisplaySize = ImVec2((float)render->getWidth(), (float)render->getHeight());
    ImGui::NewFrame();
  }

  bool ImguiOgreWrapper::render(EventDispatcher* sender, const Event& event)
  {
    RenderEvent e = static_cast<const RenderEvent&>(event);
    if(e.queueID != RENDER_QUEUE_IMGUI)
    {
      return true;
    }

    ImGuiIO& io = ImGui::GetIO();
    const std::string& name = e.renderTarget->getName();
    if(mMousePositions.count(name) != 0) {
      io.MousePos.x = mMousePositions[name].x;
      io.MousePos.y = mMousePositions[name].y;
    }

    mFrameEnded = false;
    auto now = std::chrono::high_resolution_clock::now();

    double frameTime = std::chrono::duration_cast<std::chrono::duration<double>>(now - mPreviousUpdateTime).count();

    OgreRenderSystem* render = e.getRenderSystem();

    // Setup display size (every frame to accommodate for window resizing)
    io.DisplaySize = ImVec2((float)e.renderTarget->getWidth(), (float)e.renderTarget->getHeight());

    // Start the frame
    ImGui::NewFrame();
    renderViews();
    return true;
  }
}
