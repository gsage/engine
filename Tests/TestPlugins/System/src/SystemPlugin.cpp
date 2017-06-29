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

#include "SystemPlugin.h"
#include "sol.hpp"

namespace Gsage {

  const std::string TestComponent::SYSTEM = "test";
  const std::string PLUGIN_NAME = "SystemPlugin";
  const std::string TestSystem::ID = "testSystem";

  SystemPlugin::SystemPlugin()
  {
  }

  SystemPlugin::~SystemPlugin()
  {
  }

  const std::string& SystemPlugin::getName() const
  {
    return PLUGIN_NAME;
  }

  bool SystemPlugin::installImpl()
  {
    mFacade->registerSystemFactory<TestSystem>();
    return true;
  }

  void SystemPlugin::uninstallImpl()
  {
    mFacade->getEngine()->removeSystem("test");
    mFacade->removeSystemFactory<TestSystem>();
  }

  void SystemPlugin::setupLuaBindings()
  {
    return;
    if(!mLuaInterface)
      return;

    sol::state_view* l = mLuaInterface->getSolState();
    if(!l) {
      return;
    }

    l->new_usertype<TestSystem>("TestSystem");
  }
}

Gsage::SystemPlugin* systemPlugin = NULL;

extern "C" bool PluginExport dllStartPlugin(Gsage::GsageFacade* facade)
{
  if(systemPlugin != NULL)
  {
    return false;
  }
  systemPlugin = new Gsage::SystemPlugin();
  return facade->installPlugin(systemPlugin);
}

extern "C" bool PluginExport dllStopPlugin(Gsage::GsageFacade* facade)
{
  if(systemPlugin == NULL)
    return true;

  bool res = facade->uninstallPlugin(systemPlugin);
  if(!res)
    return false;
  delete systemPlugin;
  systemPlugin = NULL;
  return true;
}
