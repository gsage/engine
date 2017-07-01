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

#ifndef _KeyboardEvent_H_
#define _KeyboardEvent_H_

#include "EventDispatcher.h"

namespace Gsage {
  class KeyboardEvent : public Event
  {
    public:
      static const std::string KEY_DOWN;
      static const std::string KEY_UP;

      enum Modifier {
        Ctrl = 0x01,
        Shift = 0x02,
        Alt = 0x04
      };

      enum Key {
        KC_UNASSIGNED,
        KC_ESCAPE,
        KC_1,
        KC_2,
        KC_3,
        KC_4,
        KC_5,
        KC_6,
        KC_7,
        KC_8,
        KC_9,
        KC_0,
        KC_MINUS,
        KC_EQUALS,
        KC_BACK,
        KC_TAB,
        KC_Q,
        KC_W,
        KC_E,
        KC_R,
        KC_T,
        KC_Y,
        KC_U,
        KC_I,
        KC_O,
        KC_P,
        KC_LBRACKET,
        KC_RBRACKET,
        KC_RETURN,
        KC_LCONTROL,
        KC_A,
        KC_S,
        KC_D,
        KC_F,
        KC_G,
        KC_H,
        KC_J,
        KC_K,
        KC_L,
        KC_SEMICOLON,
        KC_APOSTROPHE,
        KC_GRAVE,
        KC_LSHIFT,
        KC_BACKSLASH,
        KC_Z,
        KC_X,
        KC_C,
        KC_V,
        KC_B,
        KC_N,
        KC_M,
        KC_COMMA,
        KC_PERIOD,
        KC_SLASH,
        KC_RSHIFT,
        KC_MULTIPLY,
        KC_LMENU,
        KC_SPACE,
        KC_CAPITAL,
        KC_F1,
        KC_F2,
        KC_F3,
        KC_F4,
        KC_F5,
        KC_F6,
        KC_F7,
        KC_F8,
        KC_F9,
        KC_F10,
        KC_NUMLOCK,
        KC_SCROLL,
        KC_NUMPAD7,
        KC_NUMPAD8,
        KC_NUMPAD9,
        KC_SUBTRACT,
        KC_NUMPAD4,
        KC_NUMPAD5,
        KC_NUMPAD6,
        KC_ADD,
        KC_NUMPAD1,
        KC_NUMPAD2,
        KC_NUMPAD3,
        KC_NUMPAD0,
        KC_DECIMAL,
        KC_OEM_102,
        KC_F11,
        KC_F12,
        KC_F13,
        KC_F14,
        KC_F15,
        KC_KANA,
        KC_ABNT_C1,
        KC_CONVERT,
        KC_NOCONVERT,
        KC_YEN,
        KC_ABNT_C2,
        KC_NUMPADEQUALS,
        KC_PREVTRACK,
        KC_AT,
        KC_COLON,
        KC_UNDERLINE,
        KC_KANJI,
        KC_STOP,
        KC_AX,
        KC_UNLABELED,
        KC_NEXTTRACK,
        KC_NUMPADENTER,
        KC_RCONTROL,
        KC_MUTE,
        KC_CALCULATOR,
        KC_PLAYPAUSE,
        KC_MEDIASTOP,
        KC_VOLUMEDOWN,
        KC_VOLUMEUP,
        KC_WEBHOME,
        KC_NUMPADCOMMA,
        KC_DIVIDE,
        KC_SYSRQ,
        KC_RMENU,
        KC_PAUSE,
        KC_HOME,
        KC_UP,
        KC_PGUP,
        KC_LEFT,
        KC_RIGHT,
        KC_END,
        KC_DOWN,
        KC_PGDOWN,
        KC_INSERT,
        KC_DELETE,
        KC_LWIN,
        KC_RWIN,
        KC_APPS,
        KC_POWER,
        KC_SLEEP,
        KC_WAKE,
        KC_WEBSEARCH,
        KC_WEBFAVORITES,
        KC_WEBREFRESH,
        KC_WEBSTOP,
        KC_WEBFORWARD,
        KC_WEBBACK,
        KC_MYCOMPUTER,
        KC_MAIL,
        KC_MEDIASELECT,
      };

      KeyboardEvent(const std::string& type, const Key& key, const unsigned int text, const unsigned int modifierState);
      virtual ~KeyboardEvent();

      Key key;
      unsigned int text;
      /**
       * Check modifier map
       *
       * @param modifier Modifier code
       */
      bool isModifierDown(const Modifier& modifier) const;
      /**
       * Get modifier state flags
       */
      unsigned int getModifiersState() const;
    private:
      unsigned int modifierState;
  };
}

#endif
