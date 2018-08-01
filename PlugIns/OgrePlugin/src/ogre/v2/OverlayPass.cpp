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

#include "ogre/v2/OverlayPass.h"

#include <Compositor/OgreCompositorWorkspaceListener.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorNode.h>

#include "Engine.h"
#include "RenderEvent.h"

namespace Gsage {
  OverlayPass::OverlayPass(Engine* engine,
      const Ogre::CompositorPassDef *def,
      const Ogre::CompositorChannel &target,
      Ogre::CompositorNode *parentNode)
    : CustomPass(engine, def, target, parentNode)
  {
  }

  OverlayPass::~OverlayPass()
  {
  }

  void OverlayPass::execute(const Ogre::Camera* lodCamera)
  {
    Ogre::CompositorWorkspaceListener *listener = mParentNode->getWorkspace()->getListener();
    if(listener)
      listener->passPreExecute(this);

    OgreRenderSystem* render = mEngine->getSystem<OgreRenderSystem>();
    if(!render) {
      return;
    }

    Gsage::RenderTarget* target = render->getRenderTarget(mViewport->getTarget());
    if(!target) {
      return;
    }

    Ogre::SceneManager *sceneManager = render->getSceneManager();
    sceneManager->_setCurrentCompositorPass(this);
    sceneManager->_setCameraInProgress(target->getCamera());
    mEngine->fireEvent(RenderEvent(RenderEvent::RENDER_QUEUE_STARTED, render, 0, target));
    mEngine->fireEvent(RenderEvent(RenderEvent::RENDER_QUEUE_ENDED, render, 0, target));
    sceneManager->_setCameraInProgress(0);
    sceneManager->_setCurrentCompositorPass(0);

    if(listener)
      listener->passPosExecute(this);
  }

  OverlayPassDef::OverlayPassDef(Engine* engine, Ogre::CompositorTargetDef* target, Ogre::CompositorNodeDef* node)
    : CustomPassDef("overlay", engine, target, node)
  {
  }

  OverlayPassDef::~OverlayPassDef()
  {
  }

}
