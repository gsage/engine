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

#include "RocketUIPlugin.h"
#include "GsageFacade.h"
#include "RocketEvent.h"
#include "lua/LuaHelpers.h"

namespace Gsage {

  const std::string PLUGIN_NAME = "RocketUI";

  RocketUIPlugin::RocketUIPlugin()
    : mUIManagerHandle(-1)
  {
  }

  const std::string& RocketUIPlugin::getName() const
  {
    return PLUGIN_NAME;
  }

  RocketUIPlugin::~RocketUIPlugin()
  {
  }

  bool RocketUIPlugin::installImpl()
  {
    mUIManagerHandle = mFacade->addUIManager(&mUIManager);
    return true;
  }

  void RocketUIPlugin::uninstallImpl()
  {
    mFacade->removeUIManager(mUIManagerHandle);
  }

  void RocketUIPlugin::setupLuaBindings()
  {
    mLuaInterface->registerEvent<RocketContextEvent>("RocketContextEvent",
      "onRocketContext",
      sol::base_classes, sol::bases<Event>(),
      "CREATE", sol::var(RocketContextEvent::CREATE),
      "name", sol::readonly(&RocketContextEvent::name)
    );
  }
}

Gsage::RocketUIPlugin* rocketUIPlugin = NULL;

extern "C" bool PluginExport dllStartPlugin(Gsage::GsageFacade* facade)
{
  if(rocketUIPlugin != NULL)
  {
    return false;
  }
  rocketUIPlugin = new Gsage::RocketUIPlugin();
  return facade->installPlugin(rocketUIPlugin);
}

extern "C" bool PluginExport dllStopPlugin(Gsage::GsageFacade* facade)
{
  if(rocketUIPlugin == NULL)
    return true;

  bool res = facade->uninstallPlugin(rocketUIPlugin);
  if(!res)
    return false;
  delete rocketUIPlugin;
  rocketUIPlugin = NULL;
  return true;
}
