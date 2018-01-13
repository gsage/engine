#ifndef _SDLPlugin_H_
#define _SDLPlugin_H_

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

#include "IPlugin.h"
#include "SDLCore.h"

namespace Gsage {
  class SDLPlugin : public IPlugin
  {
    public:
      SDLPlugin();
      virtual ~SDLPlugin();

      /**
       * Get rocket UI plugin name
       *
       * @returns "SDL" string
       */
      virtual const std::string& getName() const;
      /**
       * Install rocker ui manager
       *
       * @returns true if succesful
       */
      virtual bool installImpl();

      /**
       * Uninstall rocket ui manager
       */
      virtual void uninstallImpl();

      /**
       * Set up lua bindings for imgui
       */
      virtual void setupLuaBindings();
    private:
      SDLCore mSDLCore;
  };
}

#endif
