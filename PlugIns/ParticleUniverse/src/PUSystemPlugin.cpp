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
#include <luabind/luabind.hpp>

namespace ParticleUniverseFactory
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

  bool PUSystemPlugin::install()
  {
    OgreRenderSystem* render = mEngine->getSystem<OgreRenderSystem>();
    if(!render)
    {
      LOG(ERROR) << "Failed to install " << PLUGIN_NAME << " plugin: OgreRenderSystem not initialized";
      return false;
    }

    render->registerElement<PUSystemWrapper>();
    if(mLuaInterface && mLuaInterface->getState())
    {
      luabind::module(mLuaInterface->getState())
      [
        luabind::class_<PUSystemWrapper, OgreObject>("OgrePUParticleSystem")
          .property("template", &PUSystemWrapper::getTemplateName)
          .def("start", &PUSystemWrapper::start)
          .def("pause", (void (PUSystemWrapper::*)())&PUSystemWrapper::pause)
          .def("resume", &PUSystemWrapper::resume)
          .def("stop", &PUSystemWrapper::stop)
      ];
    }
    return true;
  }

  void PUSystemPlugin::uninstall()
  {
    OgreRenderSystem* render = mEngine->getSystem<OgreRenderSystem>();
    if(!render)
      return;

    render->unregisterElement<PUSystemWrapper>();
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
}
