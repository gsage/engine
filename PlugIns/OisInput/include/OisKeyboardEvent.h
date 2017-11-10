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

#ifndef _OisKeyboardEvent_H_
#define _OisKeyboardEvent_H_

#include <OISKeyboard.h>
#include "EventDispatcher.h"

namespace Gsage {

  class OisKeyboardEvent : public Event
  {
    public:
      static const Event::Type KEY_DOWN;
      static const Event::Type KEY_UP;

      OisKeyboardEvent(Event::ConstType type, const OIS::KeyCode& key, const unsigned int text, const unsigned int modifierState);
      virtual ~OisKeyboardEvent();

      OIS::KeyCode key;
      unsigned int text;
      /**
       * Check modifier map
       *
       * @param modifier Modifier code
       */
      bool isModifierDown(const OIS::Keyboard::Modifier& modifier);
      /**
       * Get modifier state flags
       */
      unsigned int getModifiersState() const;
    private:
      unsigned int modifierState;
  };
}

#endif
