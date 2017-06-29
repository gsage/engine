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

#include <OgreRoot.h>
#include <OgreConfigFile.h>

#include "DataProxy.h"

namespace Gsage {
  class ResourceManager
  {
    public:
      ResourceManager(const std::string& workdir);
      virtual ~ResourceManager(void);
      /**
       * Loads all resource groups
       *
       * @param resources DataProxy with all required resources
       */
      bool load(const DataProxy& resources);

      /**
       * Unload resources
       *
       * @param group Resource group to unload
       */
      void unload(const std::string& group);

      /**
       * Unload all resources
       */
      void unload();

      /**
       * Unload resources
       * @param resources Resource groups DataProxy
       */
      void unload(const DataProxy& resources);
    private:
      std::string mWorkdir;
  };
}

