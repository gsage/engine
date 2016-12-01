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

#include "OisInputPlugin.h"
#include "OisInputListener.h"
#include "GsageFacade.h"

namespace Gsage {

  const std::string PLUGIN_NAME = "OisInput";

  OisInputPlugin::OisInputPlugin()
  {
  }

  OisInputPlugin::~OisInputPlugin()
  {
  }

  const std::string& OisInputPlugin::getName() const
  {
    return PLUGIN_NAME;
  }

  bool OisInputPlugin::installImpl()
  {
    mFacade->registerInputFactory<OisInputFactory>("ois");
    return true;
  }

  void OisInputPlugin::uninstallImpl()
  {
    mFacade->removeInputFactory("ois");
  }

  OisInputPlugin* oisInputPlugin = NULL;

  extern "C" bool PluginExport dllStartPlugin(GsageFacade* facade)
  {
    if(oisInputPlugin != NULL)
    {
      return false;
    }
    oisInputPlugin = new OisInputPlugin();
    return facade->installPlugin(oisInputPlugin);
  }

  extern "C" bool PluginExport dllStopPlugin(GsageFacade* facade)
  {
    if(oisInputPlugin == NULL)
      return true;

    bool res = facade->uninstallPlugin(oisInputPlugin);
    if(!res)
      return false;
    delete oisInputPlugin;
    oisInputPlugin = NULL;
    return true;
  }
}
