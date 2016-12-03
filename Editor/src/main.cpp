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

#include <stdio.h>
#include <thread>

#include "Logger.h"

#if GSAGE_PLATFORM == GSAGE_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#if GSAGE_PLATFORM == GSAGE_APPLE
#include "OSX/MainWrapperOSX.h"
#endif

#ifndef RESOURCES_FOLDER
#define RESOURCES_FOLDER "./resources"
#endif

#define MALLOC_CHECK_ 2

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
      int retVal = 0;
      Gsage::GsageFacade facade;
      std::string coreConfig = "gameConfig.json";

      if(!facade.initialize(coreConfig, RESOURCES_FOLDER))
      {
        LOG(ERROR) << "Failed to initialize game engine";
        return 1;
      }

#if GSAGE_PLATFORM == GSAGE_APPLE
      NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
      id mAppDelegate = [[AppDelegate alloc] initWithClass:&facade];
      [[NSApplication sharedApplication] setDelegate:mAppDelegate];
      retVal = NSApplicationMain(argc, (const char **) argv);
      [pool release];
#else
      while(facade.update())
      {
      }
#endif

      return retVal;
    }
#ifdef __cplusplus
}
#endif
