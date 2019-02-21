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

#include "TestDefinitions.h"
#include "Logger.h"

#if GSAGE_PLATFORM == GSAGE_WIN32
#define WIN32_LEAN_AND_MEAN
#include "WIN32/WindowsIncludes.h"
#endif

#ifndef RESOURCES_FOLDER
#define RESOURCES_FOLDER "./resources"
#endif

#include "sol.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#if GSAGE_PLATFORM == GSAGE_WIN32
  void fetchCmdArgs(int* argc, char*** argv) {
      // init results
      *argc = 0;

      // prepare extraction
      char* winCmd = GetCommandLine();
      int index = 0;
      bool newOption = true;
      // use static so converted command line can be
      // accessed from outside this function
      static std::vector<char*> argVector;

      // walk over the command line and convert it to argv
      while(winCmd[index] != 0){
          if (winCmd[index] == ' ') {
              // terminate option string
              winCmd[index] = 0;
              newOption = true;

          } else  {
              if(newOption){
                  argVector.push_back(&winCmd[index]);
                  (*argc)++;
              }
              newOption = false;
          }
          index++;
      }

      // elements inside the vector are guaranteed to be continous
      *argv = &argVector[0];
  }

  INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT count)
#else
    int main(int argc, char *argv[])
#endif
    {
#if GSAGE_PLATFORM == GSAGE_WIN32
      int argc = 0;
      char** argv;
      fetchCmdArgs(&argc, &argv);
#endif

#if GSAGE_PLATFORM == GSAGE_APPLE
      CFBundleRef mainBundle = CFBundleGetMainBundle();
      CFURLRef resourcesURL = CFBundleCopyBundleURL(mainBundle);
      char path[PATH_MAX];
      if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
      {
        return 1;
      }
      CFRelease(resourcesURL);
      std::stringstream p;
      p << path << "/Contents";
      chdir(p.str().c_str());
#endif
      int retVal = 0;
      Gsage::GsageFacade facade;
      std::string coreConfig = "testConfig.json";
      Gsage::DataProxy dp = Gsage::DataProxy::create(Gsage::DataWrapper::JSON_OBJECT);
      dp.put("startupScript", "scripts/entrypoint.lua");
      dp.put("startLuaInterface", true);
      dp.put("scriptsPath", TEST_RESOURCES);
      lua_State* L = lua_open();
      sol::state_view lua(L);
      if(!L) {
        LOG(ERROR) << "Lua state is not initialized";
        return 1;
      }
      lua["TRESOURCES"] = TEST_RESOURCES;
      lua["RESOURCES_FOLDER"] = RESOURCES_FOLDER;
      sol::table t = lua.create_named_table("arg");
#if GSAGE_PLATFORM == GSAGE_APPLE
      lua["PLUGINS_DIR"] = "../PlugIns";
#else
      lua["PLUGINS_DIR"] = "PlugIns";
#endif
      // proxy c++ args to the lua entrypoint
      for(int i = 0; i < argc; i++) {
        t[i] = std::string(argv[i]);
      }

      facade.setLuaState(L, false);

      if(!facade.initialize(coreConfig, RESOURCES_FOLDER, &dp))
      {
        LOG(ERROR) << "Failed to initialize game engine";
        return 1;
      }

      while(facade.update())
      {
      }

      if(retVal != 0)
        return retVal;

      return facade.getExitCode();
    }
#ifdef __cplusplus
}
#endif
