#ifndef _WindowManagerFactory_H_
#define _WindowManagerFactory_H_

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

#include "WindowManager.h"

namespace Gsage {
  /**
   * Dynamically creates new window managers
   */
  class WindowManagerFactory
  {
    public:
      WindowManagerFactory();
      virtual ~WindowManagerFactory();

      /**
       * Register new kind of window manager
       */
      template<class T, class ... Types>
      void registerWindowManager(const std::string& id, Types ... args) {
        if(!std::is_base_of<WindowManager, T>::value) {
          LOG(ERROR) << "Failed to register a WindowManager: window manager should inherit WindowManager abstract class";
          return;
        }

        std::string managerType = id;

        mRegisteredWindowManagers[id] = [&, managerType, args...](const DataProxy* params) -> WindowManagerPtr {
          WindowManagerPtr res = WindowManagerPtr((WindowManager*)(new T(managerType, args...)));
          if(params != nullptr && !res->initialize(*params)) {
            return nullptr;
          }

          return res;
        };
      }

      /**
       * Remove registered window manager
       */
      bool removeWindowManager(const std::string& id);

      /**
       * Create window manager of a specified type
       * @param id window manager id
       */
      WindowManagerPtr create(const std::string& id);

      /**
       * Create window manager of a specified type
       * @param id window manager id
       * @param params initialize window manager with parameters right after creation
       */
      WindowManagerPtr create(const std::string& id, const DataProxy& params);
    private:
      typedef std::function<WindowManagerPtr(const DataProxy* params)> CreateWindowManager;
      typedef std::map<std::string, CreateWindowManager> RegisteredWindowManagers;
      RegisteredWindowManagers mRegisteredWindowManagers;
  };
}

#endif
