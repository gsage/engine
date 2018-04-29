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

#include "ImguiOgrePlugin.h"
#include "GsageFacade.h"
#include "ImguiLuaInterface.h"
#include <imgui.h>

#include "ImguiManager.h"
#include "Definitions.h"
#include "ImguiDefinitions.h"

#if OGRE_VERSION >= 0x020100
#include "v2/ImguiRendererV2.h"
#endif
#include "v1/ImguiRendererV1.h"
#include "systems/OgreRenderSystem.h"

#include "sol.hpp"

namespace Gsage {

  const std::string PLUGIN_NAME = "ImGUIOgreRenderer";

  ImguiOgrePlugin::ImguiOgrePlugin()
  {
  }

  const std::string& ImguiOgrePlugin::getName() const
  {
    return PLUGIN_NAME;
  }

  ImguiOgrePlugin::~ImguiOgrePlugin()
  {
  }

  bool ImguiOgrePlugin::installImpl()
  {
    ImguiManager* m = static_cast<ImguiManager*>(mFacade->getUIManager(ImguiManager::TYPE));
    if(m) {
      DataProxy settings = mFacade->getEngine()->settings();
      Ogre::uint8 renderQueueGroup = settings.get("imgui.renderQueueGroup", RENDER_QUEUE_IMGUI);
      bool forceV1Renderer = settings.get("imgui.forceV1Renderer", true);

      m->addRendererFactory(
          OgreRenderSystem::ID,
          [renderQueueGroup, forceV1Renderer] () -> ImguiRenderer* {
#if OGRE_VERSION >= 0x020100
            if(!forceV1Renderer) {
              LOG(WARNING) << "V2 renderer does not work properly, use V1";
              return new ImguiRendererV2(renderQueueGroup);
            }
#endif
            return new ImguiRendererV1();
          }
      );
    }
    return true;
  }

  void ImguiOgrePlugin::uninstallImpl()
  {
    ImguiManager* m = static_cast<ImguiManager*>(mFacade->getUIManager(ImguiManager::TYPE));
    m->removeRendererFactory(OgreRenderSystem::ID);
  }

  void ImguiOgrePlugin::setupLuaBindings()
  {
  }
}

Gsage::ImguiOgrePlugin* imguiPlugin = NULL;

extern "C" bool PluginExport dllStartPlugin(Gsage::GsageFacade* facade)
{
  if(imguiPlugin != NULL)
  {
    return false;
  }
  imguiPlugin = new Gsage::ImguiOgrePlugin();
  return facade->installPlugin(imguiPlugin);
}

extern "C" bool PluginExport dllStopPlugin(Gsage::GsageFacade* facade)
{
  if(imguiPlugin == NULL)
    return true;

  bool res = facade->uninstallPlugin(imguiPlugin);
  if(!res)
    return false;
  delete imguiPlugin;
  imguiPlugin = NULL;
  return true;
}
