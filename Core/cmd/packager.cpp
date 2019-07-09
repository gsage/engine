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

#define POCO_NO_UNWINDOWS
#include "GsageFacade.h"

#include <stdio.h>
#include <thread>

#include "Logger.h"

#ifndef RESOURCES_FOLDER
#define RESOURCES_FOLDER "./resources"
#endif

#include "sol.hpp"

#ifdef __cplusplus
extern "C" {
#endif

    int main(int argc, char *argv[])
    {
      Gsage::GsageFacade facade;
      if(argc == 1) {
        LOG(ERROR) << "Usage: executable config.json";
        return 1;
      }

      std::string coreConfig(argv[1]);
      Gsage::DataProxy env;
      env.put("workdir", RESOURCES_FOLDER);
      env.put<int>("configEncoding", Gsage::FileLoader::Json);
      Gsage::FileLoader::init(env);
      Gsage::DataProxy config;
      if(!Gsage::FileLoader::getSingletonPtr()->load(coreConfig, Gsage::DataProxy(), config))
      {
        LOG(ERROR) << "Failed to load file " << coreConfig;
        return 1;
      }

      config.put("startupScript", "");
      config.put("startLuaInterface", false);

      if(!facade.initialize(config, RESOURCES_FOLDER, 0))
      {
        LOG(ERROR) << "Failed to initialize game engine";
        return 1;
      }

      Gsage::DataProxy deps = facade.getConfig().get("packager.deps", Gsage::DataProxy::create(Gsage::DataWrapper::JSON_OBJECT));
      if(!facade.installLuaPackages(deps)) {
        LOG(ERROR) << "Failed to install packages";
        return 1;
      }

      return facade.getExitCode();
    }
#ifdef __cplusplus
}
#endif
