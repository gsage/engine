/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Gsage Authors

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

#include "Editor.h"
#include "sol.hpp"
#include "Logger.h"
#include "FileLoader.h"

#include <cstdlib>

#define DEFAULT_STATE_FILE "defaultWorkspace.json"
#define GLOBAL_STATE_FILE "workspace.json"

namespace Gsage {

  Editor::Editor(GsageFacade* facade)
    : mFacade(facade)
  {
    const char* home = std::getenv(
#if GSAGE_PLATFORM == GSAGE_WIN32
      "USERPROFILE"
#else
      "HOME"
#endif
    );
    std::stringstream ss;
    ss << home << GSAGE_PATH_SEPARATOR << ".gsage";
    mConfigHome = ss.str();
  }

  Editor::~Editor()
  {
    saveGlobalState();
  }

  bool Editor::initialize(const std::string& resourceFolder)
  {
    mResourcePath = resourceFolder;
    if(!mFacade->filesystem()->mkdir(mConfigHome)) {
      LOG(ERROR) << "Failed to create config home dir " << mConfigHome;
      return false;
    }

    if(!FileLoader::getSingletonPtr()->load(mConfigHome + GSAGE_PATH_SEPARATOR + GLOBAL_STATE_FILE, DataProxy(), mGlobalEditorState)) {
      if(!FileLoader::getSingletonPtr()->load(DEFAULT_STATE_FILE, DataProxy(), mGlobalEditorState)) {
        LOG(WARNING) << "No saved global state";
      }
    }

    return true;
  }

  const DataProxy& Editor::getGlobalState() const
  {
    return mGlobalEditorState;
  }

  void Editor::putToGlobalState(const std::string& name, DataProxy data)
  {
    mGlobalEditorState.put(name, data);
  }

  bool Editor::saveGlobalState()
  {
    FileLoader::getSingletonPtr()->dump(mConfigHome + GSAGE_PATH_SEPARATOR + GLOBAL_STATE_FILE, mGlobalEditorState);
    return true;
  }

}
