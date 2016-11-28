#ifndef _RenderSystem_H_
#define _RenderSystem_H_

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

#include <string>

namespace Gsage {
  /**
   * OpenGL based render system interface
   */
  class RenderSystem
  {
    public:
      RenderSystem();
      virtual ~RenderSystem();

      /**
       * Get FBO texture ID
       *
       * @param target Render target name
       */
      virtual unsigned int getFBOID(const std::string& target = "") const = 0;

      /**
       * Set render target width
       *
       * @param target Render target name
       */
      virtual void setWidth(unsigned int width, const std::string& target = "") = 0;

      /**
       * Get render target width
       *
       * @param target Render target name
       */
      virtual unsigned int getWidth(const std::string& target = "") const = 0;

      /**
       * Set render target height
       *
       * @param target Render target name
       */
      virtual void setHeight(unsigned int height, const std::string& target = "") = 0;

      /**
       * Get render target height
       *
       * @param target Render target name
       */
      virtual unsigned int getHeight(const std::string& target = "") const = 0;

      /**
       * Set size of render target
       *
       * @param target Render target name
       */
      virtual void setSize(unsigned int width, unsigned int height, const std::string& target = "") = 0;
  };
}

#endif
