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


#include "GsageFacade.h"
#include "Editor.h"
#include "lua/LuaInterface.h"

#include <stdio.h>
#include <thread>

#include "Logger.h"
#include "DataProxy.h"

#if GSAGE_PLATFORM == GSAGE_WIN32
#define WIN32_LEAN_AND_MEAN
#include "WIN32/WindowsIncludes.h"
#endif

#ifndef RESOURCES_FOLDER
#define RESOURCES_FOLDER "./resources"
#endif

#include "sol.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atomic>

#ifdef __cplusplus
extern "C" {
#endif

#if GSAGE_PLATFORM == GSAGE_WIN32
  INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT count)
#else
    int main(int argc, char *argv[])
#endif
    {
#if GSAGE_PLATFORM == GSAGE_APPLE
      CFBundleRef mainBundle = CFBundleGetMainBundle();
      CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
      char path[PATH_MAX];
      if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX)) // Error: expected unqualified-id before 'if'
      {
        return 1;
      }
      CFRelease(resourcesURL); // error: expected constructor, destructor or type conversion before '(' token
      chdir(path); // error: expected constructor, destructor or type conversion before '(' token
#endif
#if GSAGE_PLATFORM == GSAGE_WIN32
      SetProcessDPIAware();
#endif
      int retVal = 0;
      Gsage::GsageFacade facade;
      Gsage::Editor editor(&facade);
      std::string coreConfig = "editorConfig.json";
      lua_State* L = lua_open();
      if(!L) {
        LOG(ERROR) << "Lua state is not initialized";
        return 1;
      }

      sol::state_view lua(L);
      lua.new_usertype<Gsage::Editor>("Editor",
          "putToGlobalState", &Gsage::Editor::putToGlobalState,
          "getGlobalState", &Gsage::Editor::getGlobalState,
          "saveGlobalState", &Gsage::Editor::saveGlobalState
      );
      lua["editor"] = &editor;

      facade.setLuaState(L, false);

      if(!facade.initialize(
        coreConfig,
        RESOURCES_FOLDER,
        Gsage::FileLoader::Json,
        Gsage::GsageFacade::Managers | Gsage::GsageFacade::Plugins
      ))
      {
        LOG(ERROR) << "Failed to initialize game engine";
        return 10;
      }

      Gsage::WindowPtr window = nullptr;
      auto pair = facade.getConfig().get<Gsage::DataProxy>("splashScreen");
      Gsage::WindowManagerPtr wm = facade.getWindowManager();
      if(pair.second && wm) {
        window = wm->createWindow(
          pair.first.get("windowName", "gsage"),
          pair.first.get("width", 600),
          pair.first.get("height", 400),
          false,
          pair.first.get("windowParams", Gsage::DataProxy())
        );
      }

      std::atomic_bool finalizeLoad(false);
      bool packagesInstalled = false;

      std::thread packager([&](){
        Gsage::DataProxy deps = facade.getConfig().get("packager.deps", Gsage::DataProxy::create(Gsage::DataWrapper::JSON_OBJECT));
        packagesInstalled = facade.installLuaPackages(deps);
        finalizeLoad.store(true);
      });

      while(facade.update())
      {
        if(finalizeLoad.load()) {
          if(packager.joinable())
            packager.join();

          // close splash screen
          if(window) {
            wm->destroyWindow(window);
            window = nullptr;
          }

          if(!packagesInstalled) {
            LOG(ERROR) << "Failed to install lua packages";
            return 20;
          }

          if(!facade.configureSystems()) {
            LOG(ERROR) << "Failed to configure systems";
            return 30;
          }

          if(!editor.initialize(RESOURCES_FOLDER)) {
            LOG(ERROR) << "Failed to initialize editor core";
            return 40;
          }
          finalizeLoad.store(false);
        }
      }

      if(retVal != 0)
        return retVal;

      return facade.getExitCode();
    }
#ifdef __cplusplus
}
#endif
