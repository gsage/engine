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

#include "MouseEvent.h"

using namespace Gsage;

const Event::Type MouseEvent::MOUSE_DOWN = "mouseDown";
const Event::Type MouseEvent::MOUSE_UP = "mouseUp";
const Event::Type MouseEvent::MOUSE_MOVE = "mouseMove";

MouseEvent::MouseEvent(Event::ConstType type, const float& w, const float& h, const ButtonType& b, const std::string& dispatcher) 
  : Event(type)
  , width(w)
  , height(h)
  , button(b)
  , dispatcher(dispatcher)
{
}

MouseEvent::~MouseEvent()
{
}

void MouseEvent::setRelativePosition(const float& x, const float& y, const float& z)
{
  relativeX = x;
  relativeY = y;
  relativeZ = z;
}

void MouseEvent::setAbsolutePosition(const float& x, const float& y, const float& z)
{
  mouseX = x;
  mouseY = y;
  mouseZ = z;
}
