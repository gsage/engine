/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2016 Artem Chernyshev

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

#include "ogre/RocketOgreWrapper.h"
#include "Engine.h"

#include "ogre/RenderInterfaceOgre3D.h"
#include "ogre/SystemInterfaceOgre3D.h"

namespace Gsage {

  RocketOgreWrapper::RocketOgreWrapper(Engine* engine)
    : mEngine(engine)
    , mRenderInterface(0)
    , mSystemInterface(0)
  {
    addEventListener(engine, RenderEvent::UPDATE_UI, &RocketOgreWrapper::render);
    OgreRenderSystem* render = engine->getSystem<OgreRenderSystem>();

    if(render)
    {
      setUp(render->getWidth(), render->getHeight());
    }
  }

  RocketOgreWrapper::~RocketOgreWrapper()
  {
    if(mContext)
      mContext->RemoveReference();
    Rocket::Core::Shutdown();

    if(mRenderInterface)
      delete mRenderInterface;

    if(mSystemInterface)
      delete mSystemInterface;
  }

  bool RocketOgreWrapper::render(EventDispatcher* sender, const Event& event)
  {
    RenderEvent e = static_cast<const RenderEvent&>(event);
    if(!mRenderInterface || !mSystemInterface)
      setUp(e.getRenderSystem()->getWidth(),
            e.getRenderSystem()->getHeight());

    mContext->Update();
    configureRenderSystem(e);
    mContext->Render();
    return true;
  }

  void RocketOgreWrapper::configureRenderSystem(RenderEvent& event)
  {
    OgreRenderSystem* gsageRendering = event.getRenderSystem();
    Ogre::RenderSystem* renderSystem = gsageRendering->getRenderSystem();

    // Set up the projection and view matrices.
    Ogre::Matrix4 projectionMatrix;
    float zNear = -1;
    float zFar = 1;

    projectionMatrix = Ogre::Matrix4::ZERO;

    // Set up matrices.
    projectionMatrix[0][0] = 2.0f / gsageRendering->getWidth();
    projectionMatrix[0][3]= -1.0000000f;
    projectionMatrix[1][1]= -2.0f / gsageRendering->getHeight();
    projectionMatrix[1][3]= 1.0000000f;
    projectionMatrix[2][2]= -2.0f / (zFar - zNear);
    projectionMatrix[3][3]= 1.0000000f;
    renderSystem->_setProjectionMatrix(projectionMatrix);
    renderSystem->_setViewMatrix(Ogre::Matrix4::IDENTITY);

    // Disable lighting, as all of Rocket's geometry is unlit.
    renderSystem->setLightingEnabled(false);
    // Disable depth-buffering; all of the geometry is already depth-sorted.
    renderSystem->_setDepthBufferParams(false, false);
    // Rocket generates anti-clockwise geometry, so enable clockwise-culling.
    renderSystem->_setCullingMode(Ogre::CULL_CLOCKWISE);
    // Disable fogging.
    renderSystem->_setFog(Ogre::FOG_NONE);
    // Enable writing to all four channels.
    renderSystem->_setColourBufferWriteEnabled(true, true, true, true);
    // Unbind any vertex or fragment programs bound previously by the application.
    renderSystem->unbindGpuProgram(Ogre::GPT_FRAGMENT_PROGRAM);
    renderSystem->unbindGpuProgram(Ogre::GPT_VERTEX_PROGRAM);

    // Set texture settings to clamp along both axes.
    Ogre::TextureUnitState::UVWAddressingMode addressingMode;
    addressingMode.u = Ogre::TextureUnitState::TAM_CLAMP;
    addressingMode.v = Ogre::TextureUnitState::TAM_CLAMP;
    addressingMode.w = Ogre::TextureUnitState::TAM_CLAMP;
    renderSystem->_setTextureAddressingMode(0, addressingMode);

    // Set the texture coordinates for unit 0 to be read from unit 0.
    renderSystem->_setTextureCoordSet(0, 0);
    // Disable texture coordinate calculation.
    renderSystem->_setTextureCoordCalculation(0, Ogre::TEXCALC_NONE);
    // Enable linear filtering; images should be rendering 1 texel == 1 pixel, so point filtering could be used
    // except in the case of scaling tiled decorators.
    renderSystem->_setTextureUnitFiltering(0, Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_POINT);
    // Disable texture coordinate transforms.
    renderSystem->_setTextureMatrix(0, Ogre::Matrix4::IDENTITY);
    // Reject pixels with an alpha of 0.
    renderSystem->_setAlphaRejectSettings(Ogre::CMPF_GREATER, 0, false);
    // Disable all texture units but the first.
    renderSystem->_disableTextureUnitsFrom(1);

    // Enable simple alpha blending.
    renderSystem->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);

    // Disable depth bias.
    renderSystem->_setDepthBias(0, 0);
  }

  void RocketOgreWrapper::setUp(unsigned int width, unsigned int height)
  {
    mRenderInterface = new RenderInterfaceOgre3D(width, height);
    mSystemInterface = new SystemInterfaceOgre3D();

    Rocket::Core::SetRenderInterface(mRenderInterface);
    Rocket::Core::SetSystemInterface(mSystemInterface);

    Rocket::Core::Initialise();
    Rocket::Controls::Initialise();
    mContext = Rocket::Core::CreateContext("main", Rocket::Core::Vector2i(width, height));
  }

}
