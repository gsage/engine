#ifndef _InputPlugin_H_
#define _InputPlugin_H_

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
#include "input/InputFactory.h"

namespace Gsage {

  /**
   * Fake input handler
   */
  class FakeInputHandler : public InputHandler
  {
    public:
    FakeInputHandler(size_t h, Engine* e) : InputHandler(h,e) {};
    virtual ~FakeInputHandler() {};
    virtual void handleResize(unsigned int width, unsigned int height) {}
    virtual void handleClose() {};
    virtual void update(double time) {}
  };

  /**
   * Minimal implementation for an input factory
   */
  class FakeInputFactory : public AbstractInputFactory
  {
    virtual ~FakeInputFactory() {};
    /**
     * Create fake input handler
     */
    InputHandler* create(size_t windowHandle, Engine* engine)
    {
      return new FakeInputHandler(windowHandle, engine);
    }
  };
  /**
   * Test plugin
   */
  class InputPlugin : public IPlugin
  {
    public:
      InputPlugin();
      virtual ~InputPlugin();
      /**
       * Get ois plugin name
       */
      const std::string& getName() const;

      /**
       * Unregisters input system
       */
      bool installImpl();

      /**
       * Registers input system
       */
      void uninstallImpl();
  };
}

#endif
