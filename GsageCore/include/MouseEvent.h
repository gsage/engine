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

#ifndef _MouseEvent_H_
#define _MouseEvent_H_

#include "EventDispatcher.h"

namespace Gsage
{
  /**
   * Mouse event that is not specific to any input library
   */
  class MouseEvent : public Event
  {
    public:
      static const Event::Type MOUSE_DOWN;
      static const Event::Type MOUSE_UP;
      static const Event::Type MOUSE_MOVE;

      enum ButtonType { Left = 0, Right, Middle, Button3, Button4, Button5, Button6, Button7, None };

      MouseEvent(Event::ConstType type,
                 const float& width,
                 const float& height,
                 const ButtonType& button = None,
                 const std::string& dispatcher = "main");
      virtual ~MouseEvent();

      ButtonType button;

      void setRelativePosition(const float& x, const float& y, const float& z);
      void setAbsolutePosition(const float& x, const float& y, const float& z);
      float mouseX;
      float mouseY;
      float mouseZ;

      float relativeX;
      float relativeY;
      float relativeZ;

      float width;
      float height;

      std::string dispatcher;
  };
}

#endif
