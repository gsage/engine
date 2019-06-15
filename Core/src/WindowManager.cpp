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

#include "WindowManager.h"
#include "EngineEvent.h"
#include <tinyfiledialogs.h>

namespace Gsage {

  WindowManager::WindowManager(const std::string& type)
    : mType(type)
  {
  }

  WindowManager::~WindowManager()
  {
    mWindows.clear();
  }

  void WindowManager::fireWindowEvent(Event::ConstType type, unsigned long handle, unsigned int width, unsigned int height)
  {
    fireEvent(WindowEvent(type, handle, width, height));
  }

  WindowPtr WindowManager::getWindow(const std::string& name)
  {
    if(mWindows.count(name) == 0) {
      return nullptr;
    }

    return mWindows[name];
  }

  WindowPtr WindowManager::getWindow(const unsigned long long handle)
  {
    for(auto& pair : mWindows) {
      if(pair.second->getWindowHandle() == handle) {
        return pair.second;
      }
    }
    return nullptr;
  }

  void WindowManager::windowCreated(WindowPtr window)
  {
    mWindows[window->getName()] = window;
  }

  bool WindowManager::windowDestroyed(WindowPtr window)
  {
    if(contains(mWindows, window->getName())) {
      fireWindowEvent(WindowEvent::CLOSE, window->getWindowHandle());
      mWindows.erase(window->getName());
      return true;
    }
    return false;
  }

  WindowManager::DialogResult WindowManager::openDialog(
        Window::DialogMode mode,
        const std::string& title,
        const std::string& defaultFilePath,
        const std::vector<std::string> filters)
  {
    return openDialogSync(mode, title, defaultFilePath, filters);
  }

  std::tuple<std::vector<std::string>, Window::DialogStatus, std::string> WindowManager::openDialogSync(
      Window::DialogMode mode,
      const std::string& title,
      const std::string& defaultFilePath,
      const std::vector<std::string> filters
  )
  {
    std::vector<std::string> paths;
    Window::DialogStatus status;
    const char* outPath = NULL;

    const char* f[256];
    for(int i = 0; i < filters.size(); i++) {
      f[i] = filters[i].c_str();
    }

    switch(mode) {
      case Window::DialogMode::FILE_DIALOG_OPEN_MULTIPLE:
      case Window::DialogMode::FILE_DIALOG_OPEN:
        outPath = tinyfd_openFileDialog(&title[0], &defaultFilePath[0], filters.size(), &f[0], NULL, mode == Window::DialogMode::FILE_DIALOG_OPEN_MULTIPLE);
        break;
      case Window::DialogMode::FILE_DIALOG_OPEN_FOLDER:
        outPath = tinyfd_selectFolderDialog(&title[0], &defaultFilePath[0]);
        break;
      case Window::DialogMode::FILE_DIALOG_SAVE:
        outPath = tinyfd_saveFileDialog(&title[0], &defaultFilePath[0], filters.size(), &f[0], NULL);
        break;
    }

    std::stringstream err;

    if(outPath == NULL) {
      return std::make_tuple(paths, Window::DialogStatus::CANCEL, "");
    }

    paths = split(std::string(outPath), '|');
    return std::make_tuple(paths, Window::DialogStatus::OKAY, "");
  }
}
