#ifndef _GsageOgrePlugin_H_
#define _GsageOgrePlugin_H_

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

#include "IPlugin.h"

namespace Gsage {
  class GsageOgrePlugin : public IPlugin
  {
    public:
      GsageOgrePlugin();
      virtual ~GsageOgrePlugin();

      /**
       * Get ogre plugin name
       */
      virtual const std::string& getName() const;

      /**
       * Registers OgreRenderSystem and RecastNavigationSystem
       */
      virtual bool installImpl();

      /**
       * Unregisters OgreRenderSystem and RecastNavigationSystem
       */
      virtual void uninstallImpl();

      /**
       * Set up lua bindings
       */
      virtual void setupLuaBindings();
  };
}

#endif
