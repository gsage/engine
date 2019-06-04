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

#include "SDLPlugin.h"
#include "SDLWindowManager.h"
#include "SDLInputListener.h"

namespace Gsage {

  const std::string PLUGIN_NAME = "SDL";

  SDLPlugin::SDLPlugin()
  {
  }

  SDLPlugin::~SDLPlugin()
  {
  }

  const std::string& SDLPlugin::getName() const
  {
    return PLUGIN_NAME;
  }

  bool SDLPlugin::installImpl()
  {
    // TODO: get parameters from config somehow?
    mSDLCore.initialize(DataProxy(), mFacade);
    mFacade->registerWindowManager<SDLWindowManager>("SDL", &mSDLCore);
    SDLInputFactory* f = mFacade->registerInputFactory<SDLInputFactory>("SDL");
    if(!f) {
      return false;
    }

    f->setSDLCore(&mSDLCore);
    return true;
  }

  void SDLPlugin::uninstallImpl()
  {
    mFacade->removeWindowManager("SDL");
    mFacade->removeInputFactory("SDL");
    mSDLCore.tearDown();
  }

  void SDLPlugin::setupLuaBindings()
  {
  }

}

Gsage::SDLPlugin* sdlPlugin = NULL;

extern "C" bool PluginExport dllStartPlugin(Gsage::GsageFacade* facade)
{
  if(sdlPlugin != NULL)
  {
    return false;
  }
  sdlPlugin = new Gsage::SDLPlugin();
  return facade->installPlugin(sdlPlugin);
}

extern "C" bool PluginExport dllStopPlugin(Gsage::GsageFacade* facade)
{
  if(sdlPlugin == NULL)
    return true;

  bool res = facade->uninstallPlugin(sdlPlugin);
  if(!res)
    return false;
  delete sdlPlugin;
  sdlPlugin = NULL;
  return true;
}
