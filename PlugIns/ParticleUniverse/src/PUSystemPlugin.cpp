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

#include "PUSystemPlugin.h"
#include "Logger.h"
#include "GsageFacade.h"
#include "systems/OgreRenderSystem.h"
#include "lua/LuaInterface.h"
#include "PUSystemWrapper.h"
#include "ogre/SceneNodeWrapper.h"

namespace Gsage
{
  const std::string PLUGIN_NAME = "ParticleUniverseFactory";

  PUSystemPlugin::PUSystemPlugin()
  {
  }

  PUSystemPlugin::~PUSystemPlugin()
  {
  }

  const std::string& PUSystemPlugin::getName() const
  {
    return PLUGIN_NAME;
  }

  bool PUSystemPlugin::installImpl()
  {
    addEventListener(mFacade->getEngine(), SystemChangeEvent::SYSTEM_ADDED, &PUSystemPlugin::handleSystemAdded);

    OgreRenderSystem* render = mFacade->getEngine()->getSystem<OgreRenderSystem>();
    if(!render)
    {
      LOG(ERROR) << "Failed to install " << PLUGIN_NAME << " plugin: OgreRenderSystem not initialized";
      return false;
    }

    render->registerElement<PUSystemWrapper>();
    return true;
  }

  void PUSystemPlugin::uninstallImpl()
  {
    removeEventListener(mFacade->getEngine(), SystemChangeEvent::SYSTEM_ADDED, &PUSystemPlugin::handleSystemAdded);
    OgreRenderSystem* render = mFacade->getEngine()->getSystem<OgreRenderSystem>();
    if(!render)
      return;

    render->unregisterElement<PUSystemWrapper>();
  }

  void PUSystemPlugin::setupLuaBindings()
  {
    if(mLuaInterface && mLuaInterface->getState())
    {
      lua = sol::state_view(mLuaInterface->getState());
      lua.new_usertype<PUSystemWrapper>("PUParticleSystemWrapper",
        sol::base_classes, sol::bases<OgreObject>(),
        "template", sol::property(&PUSystemWrapper::getTemplateName),
        "start", &PUSystemWrapper::start,
        "pause", (void (PUSystemWrapper::*)())&PUSystemWrapper::pause,
        "resume", &PUSystemWrapper::resume,
        "stop", &PUSystemWrapper::stop
      );

      lua["OgreSceneNode"]["getPUSystem"] = &SceneNodeWrapper::getChildOfType<PUSystemWrapper>;
    }
  }

  bool PUSystemPlugin::handleSystemAdded(const Event& event)
  {
    const SystemChangeEvent& e = static_cast<const SystemChangeEvent&>(event);
    OgreRenderSystem* renderSystem = 0;
    if(e.mSystemId == "render" && renderSystem = mFacade->getEngine()->getSystem<OgreRenderSystem>())
    {
      renderSystem->registerElement<PUSystemWrapper>();
    }
  }

}

PUSystemPlugin* puSystemPlugin = NULL;

extern "C" bool PluginExport dllStartPlugin(GsageFacade* facade)
{
  if(puSystemPlugin != NULL)
  {
    LOG(WARNING) << "Particle system factory plugin is already installed";
    return false;
  }
  puSystemPlugin = new PUSystemPlugin();
  return facade->installPlugin(puSystemPlugin);
}

extern "C" bool PluginExport dllStopPlugin(GsageFacade* facade)
{
  bool res = facade->uninstallPlugin(puSystemPlugin);
  delete puSystemPlugin;
  return res;
}
