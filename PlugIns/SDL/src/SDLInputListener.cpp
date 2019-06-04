/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Artem Chernyshev

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

#include "SDLInputListener.h"
#include "SDLCore.h"
#include "Engine.h"

namespace Gsage {

  InputHandlerPtr SDLInputFactory::create(size_t windowHandle, Engine* engine)
  {
    if(mListener) {
      return mListener;
    }

    if(!mCore) {
      return 0;
    }

    SDL_StartTextInput();
    SDL_ShowCursor(engine->settings().get("sdl.showCursor", false));
    SDLInputListener* l = new SDLInputListener(windowHandle, engine);
    mCore->addEventListener(l);
    mListener = InputHandlerPtr(l);
    return mListener;
  }

  void SDLInputFactory::setSDLCore(SDLCore* core)
  {
    mCore = core;
  }

  SDLInputListener::SDLInputListener(size_t handle, Engine* eventRedirect)
    : InputHandler(handle, eventRedirect)
  {
    mKeyMap[SDL_SCANCODE_ESCAPE] = KeyboardEvent::KC_ESCAPE;
    mKeyMap[SDL_SCANCODE_1] = KeyboardEvent::KC_1;
    mKeyMap[SDL_SCANCODE_2] = KeyboardEvent::KC_2;
    mKeyMap[SDL_SCANCODE_3] = KeyboardEvent::KC_3;
    mKeyMap[SDL_SCANCODE_4] = KeyboardEvent::KC_4;
    mKeyMap[SDL_SCANCODE_5] = KeyboardEvent::KC_5;
    mKeyMap[SDL_SCANCODE_6] = KeyboardEvent::KC_6;
    mKeyMap[SDL_SCANCODE_7] = KeyboardEvent::KC_7;
    mKeyMap[SDL_SCANCODE_8] = KeyboardEvent::KC_8;
    mKeyMap[SDL_SCANCODE_9] = KeyboardEvent::KC_9;
    mKeyMap[SDL_SCANCODE_0] = KeyboardEvent::KC_0;
    mKeyMap[SDL_SCANCODE_MINUS] = KeyboardEvent::KC_MINUS;
    mKeyMap[SDL_SCANCODE_EQUALS] = KeyboardEvent::KC_EQUALS;
    mKeyMap[SDL_SCANCODE_BACKSPACE] = KeyboardEvent::KC_BACK;
    mKeyMap[SDL_SCANCODE_TAB] = KeyboardEvent::KC_TAB;
    mKeyMap[SDL_SCANCODE_Q] = KeyboardEvent::KC_Q;
    mKeyMap[SDL_SCANCODE_W] = KeyboardEvent::KC_W;
    mKeyMap[SDL_SCANCODE_E] = KeyboardEvent::KC_E;
    mKeyMap[SDL_SCANCODE_R] = KeyboardEvent::KC_R;
    mKeyMap[SDL_SCANCODE_T] = KeyboardEvent::KC_T;
    mKeyMap[SDL_SCANCODE_Y] = KeyboardEvent::KC_Y;
    mKeyMap[SDL_SCANCODE_U] = KeyboardEvent::KC_U;
    mKeyMap[SDL_SCANCODE_I] = KeyboardEvent::KC_I;
    mKeyMap[SDL_SCANCODE_O] = KeyboardEvent::KC_O;
    mKeyMap[SDL_SCANCODE_P] = KeyboardEvent::KC_P;
    mKeyMap[SDL_SCANCODE_LEFTBRACKET] = KeyboardEvent::KC_LBRACKET;
    mKeyMap[SDL_SCANCODE_RIGHTBRACKET] = KeyboardEvent::KC_RBRACKET;
    mKeyMap[SDL_SCANCODE_KP_LEFTBRACE] = KeyboardEvent::KC_LBRACKET;
    mKeyMap[SDL_SCANCODE_KP_RIGHTBRACE] = KeyboardEvent::KC_RBRACKET;
    mKeyMap[SDL_SCANCODE_RETURN] = KeyboardEvent::KC_RETURN;
    mKeyMap[SDL_SCANCODE_LCTRL] = KeyboardEvent::KC_LCONTROL;
    mKeyMap[SDL_SCANCODE_A] = KeyboardEvent::KC_A;
    mKeyMap[SDL_SCANCODE_S] = KeyboardEvent::KC_S;
    mKeyMap[SDL_SCANCODE_D] = KeyboardEvent::KC_D;
    mKeyMap[SDL_SCANCODE_F] = KeyboardEvent::KC_F;
    mKeyMap[SDL_SCANCODE_G] = KeyboardEvent::KC_G;
    mKeyMap[SDL_SCANCODE_H] = KeyboardEvent::KC_H;
    mKeyMap[SDL_SCANCODE_J] = KeyboardEvent::KC_J;
    mKeyMap[SDL_SCANCODE_K] = KeyboardEvent::KC_K;
    mKeyMap[SDL_SCANCODE_L] = KeyboardEvent::KC_L;
    mKeyMap[SDL_SCANCODE_SEMICOLON] = KeyboardEvent::KC_SEMICOLON;
    mKeyMap[SDL_SCANCODE_APOSTROPHE] = KeyboardEvent::KC_APOSTROPHE;
    mKeyMap[SDL_SCANCODE_GRAVE] = KeyboardEvent::KC_GRAVE;
    mKeyMap[SDL_SCANCODE_LSHIFT] = KeyboardEvent::KC_LSHIFT;
    mKeyMap[SDL_SCANCODE_BACKSLASH] = KeyboardEvent::KC_BACKSLASH;
    mKeyMap[SDL_SCANCODE_Z] = KeyboardEvent::KC_Z;
    mKeyMap[SDL_SCANCODE_X] = KeyboardEvent::KC_X;
    mKeyMap[SDL_SCANCODE_C] = KeyboardEvent::KC_C;
    mKeyMap[SDL_SCANCODE_V] = KeyboardEvent::KC_V;
    mKeyMap[SDL_SCANCODE_B] = KeyboardEvent::KC_B;
    mKeyMap[SDL_SCANCODE_N] = KeyboardEvent::KC_N;
    mKeyMap[SDL_SCANCODE_M] = KeyboardEvent::KC_M;
    mKeyMap[SDL_SCANCODE_COMMA] = KeyboardEvent::KC_COMMA;
    mKeyMap[SDL_SCANCODE_PERIOD] = KeyboardEvent::KC_PERIOD;
    mKeyMap[SDL_SCANCODE_SLASH] = KeyboardEvent::KC_SLASH;
    mKeyMap[SDL_SCANCODE_RSHIFT] = KeyboardEvent::KC_RSHIFT;
    mKeyMap[SDL_SCANCODE_KP_MULTIPLY] = KeyboardEvent::KC_MULTIPLY;
    mKeyMap[SDL_SCANCODE_MENU] = KeyboardEvent::KC_LMENU;
    mKeyMap[SDL_SCANCODE_SPACE] = KeyboardEvent::KC_SPACE;
    mKeyMap[SDL_SCANCODE_CAPSLOCK] = KeyboardEvent::KC_CAPITAL;
    mKeyMap[SDL_SCANCODE_F1] = KeyboardEvent::KC_F1;
    mKeyMap[SDL_SCANCODE_F2] = KeyboardEvent::KC_F2;
    mKeyMap[SDL_SCANCODE_F3] = KeyboardEvent::KC_F3;
    mKeyMap[SDL_SCANCODE_F4] = KeyboardEvent::KC_F4;
    mKeyMap[SDL_SCANCODE_F5] = KeyboardEvent::KC_F5;
    mKeyMap[SDL_SCANCODE_F6] = KeyboardEvent::KC_F6;
    mKeyMap[SDL_SCANCODE_F7] = KeyboardEvent::KC_F7;
    mKeyMap[SDL_SCANCODE_F8] = KeyboardEvent::KC_F8;
    mKeyMap[SDL_SCANCODE_F9] = KeyboardEvent::KC_F9;
    mKeyMap[SDL_SCANCODE_F10] = KeyboardEvent::KC_F10;
    mKeyMap[SDL_SCANCODE_NUMLOCKCLEAR] = KeyboardEvent::KC_NUMLOCK;
    mKeyMap[SDL_SCANCODE_SCROLLLOCK] = KeyboardEvent::KC_SCROLL;
    mKeyMap[SDL_SCANCODE_KP_7] = KeyboardEvent::KC_NUMPAD7;
    mKeyMap[SDL_SCANCODE_KP_8] = KeyboardEvent::KC_NUMPAD8;
    mKeyMap[SDL_SCANCODE_KP_9] = KeyboardEvent::KC_NUMPAD9;
    mKeyMap[SDL_SCANCODE_KP_MINUS] = KeyboardEvent::KC_SUBTRACT;
    mKeyMap[SDL_SCANCODE_KP_4] = KeyboardEvent::KC_NUMPAD4;
    mKeyMap[SDL_SCANCODE_KP_5] = KeyboardEvent::KC_NUMPAD5;
    mKeyMap[SDL_SCANCODE_KP_6] = KeyboardEvent::KC_NUMPAD6;
    mKeyMap[SDL_SCANCODE_KP_PLUS] = KeyboardEvent::KC_ADD;
    mKeyMap[SDL_SCANCODE_KP_1] = KeyboardEvent::KC_NUMPAD1;
    mKeyMap[SDL_SCANCODE_KP_2] = KeyboardEvent::KC_NUMPAD2;
    mKeyMap[SDL_SCANCODE_KP_3] = KeyboardEvent::KC_NUMPAD3;
    mKeyMap[SDL_SCANCODE_KP_0] = KeyboardEvent::KC_NUMPAD0;
    mKeyMap[SDL_SCANCODE_KP_DECIMAL] = KeyboardEvent::KC_DECIMAL;
    mKeyMap[SDL_SCANCODE_F11] = KeyboardEvent::KC_F11;
    mKeyMap[SDL_SCANCODE_F12] = KeyboardEvent::KC_F12;
    mKeyMap[SDL_SCANCODE_F13] = KeyboardEvent::KC_F13;
    mKeyMap[SDL_SCANCODE_F14] = KeyboardEvent::KC_F14;
    mKeyMap[SDL_SCANCODE_F15] = KeyboardEvent::KC_F15;
    mKeyMap[SDL_SCANCODE_KP_EQUALS] = KeyboardEvent::KC_NUMPADEQUALS;
    mKeyMap[SDL_SCANCODE_AUDIOPREV] = KeyboardEvent::KC_PREVTRACK;
    mKeyMap[SDL_SCANCODE_STOP] = KeyboardEvent::KC_STOP;
    mKeyMap[SDL_SCANCODE_AUDIONEXT] = KeyboardEvent::KC_NEXTTRACK;
    mKeyMap[SDL_SCANCODE_KP_ENTER] = KeyboardEvent::KC_NUMPADENTER;
    mKeyMap[SDL_SCANCODE_RCTRL] = KeyboardEvent::KC_RCONTROL;
    mKeyMap[SDL_SCANCODE_MUTE] = KeyboardEvent::KC_MUTE;
    mKeyMap[SDL_SCANCODE_CALCULATOR] = KeyboardEvent::KC_CALCULATOR;
    mKeyMap[SDL_SCANCODE_AUDIOPLAY] = KeyboardEvent::KC_PLAYPAUSE;
    mKeyMap[SDL_SCANCODE_AUDIOSTOP] = KeyboardEvent::KC_MEDIASTOP;
    mKeyMap[SDL_SCANCODE_VOLUMEDOWN] = KeyboardEvent::KC_VOLUMEDOWN;
    mKeyMap[SDL_SCANCODE_VOLUMEUP] = KeyboardEvent::KC_VOLUMEUP;
    mKeyMap[SDL_SCANCODE_WWW] = KeyboardEvent::KC_WEBHOME;
    mKeyMap[SDL_SCANCODE_KP_COMMA] = KeyboardEvent::KC_NUMPADCOMMA;
    mKeyMap[SDL_SCANCODE_KP_DIVIDE] = KeyboardEvent::KC_DIVIDE;
    mKeyMap[SDL_SCANCODE_SYSREQ] = KeyboardEvent::KC_SYSRQ;
    mKeyMap[SDL_SCANCODE_PAUSE] = KeyboardEvent::KC_PAUSE;
    mKeyMap[SDL_SCANCODE_HOME] = KeyboardEvent::KC_HOME;
    mKeyMap[SDL_SCANCODE_UP] = KeyboardEvent::KC_UP;
    mKeyMap[SDL_SCANCODE_PAGEUP] = KeyboardEvent::KC_PGUP;
    mKeyMap[SDL_SCANCODE_LEFT] = KeyboardEvent::KC_LEFT;
    mKeyMap[SDL_SCANCODE_RIGHT] = KeyboardEvent::KC_RIGHT;
    mKeyMap[SDL_SCANCODE_END] = KeyboardEvent::KC_END;
    mKeyMap[SDL_SCANCODE_DOWN] = KeyboardEvent::KC_DOWN;
    mKeyMap[SDL_SCANCODE_PAGEDOWN] = KeyboardEvent::KC_PGDOWN;
    mKeyMap[SDL_SCANCODE_INSERT] = KeyboardEvent::KC_INSERT;
    mKeyMap[SDL_SCANCODE_DELETE] = KeyboardEvent::KC_DELETE;
    mKeyMap[SDL_SCANCODE_LGUI] = KeyboardEvent::KC_LWIN;
    mKeyMap[SDL_SCANCODE_RGUI] = KeyboardEvent::KC_RWIN;
    mKeyMap[SDL_SCANCODE_LALT] = KeyboardEvent::KC_LALT;
    mKeyMap[SDL_SCANCODE_RALT] = KeyboardEvent::KC_RALT;
    mKeyMap[SDL_SCANCODE_POWER] = KeyboardEvent::KC_POWER;
    mKeyMap[SDL_SCANCODE_SLEEP] = KeyboardEvent::KC_SLEEP;
    mKeyMap[SDL_SCANCODE_AC_SEARCH] = KeyboardEvent::KC_WEBSEARCH;
    mKeyMap[SDL_SCANCODE_AC_BOOKMARKS] = KeyboardEvent::KC_WEBFAVORITES;
    mKeyMap[SDL_SCANCODE_AC_REFRESH] = KeyboardEvent::KC_WEBREFRESH;
    mKeyMap[SDL_SCANCODE_AC_STOP] = KeyboardEvent::KC_WEBSTOP;
    mKeyMap[SDL_SCANCODE_AC_FORWARD] = KeyboardEvent::KC_WEBFORWARD;
    mKeyMap[SDL_SCANCODE_AC_BACK] = KeyboardEvent::KC_WEBBACK;
    mKeyMap[SDL_SCANCODE_COMPUTER] = KeyboardEvent::KC_MYCOMPUTER;
    mKeyMap[SDL_SCANCODE_MAIL] = KeyboardEvent::KC_MAIL;
    mKeyMap[SDL_SCANCODE_MEDIASELECT] = KeyboardEvent::KC_MEDIASELECT;
  }

  SDLInputListener::~SDLInputListener()
  {
  }

  void SDLInputListener::handleEvent(SDL_Event* event)
  {
    switch(event->type) {
      case SDL_MOUSEWHEEL:
      case SDL_MOUSEMOTION:
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEBUTTONDOWN:
        handleMouseEvent(event);
        break;
      case SDL_KEYUP:
      case SDL_KEYDOWN:
        handleKeyboardEvent(event);
        break;
      case SDL_TEXTINPUT:
        TextInputEvent e(TextInputEvent::INPUT, event->text.text);
#if GSAGE_PLATFORM == GSAGE_LINUX
        SDL_Keycode sc = event->key.keysym.scancode;
        if(mKeyMap.count(sc) != 0) {
          e.key = mKeyMap[sc];
        } else {
          e.key = KeyboardEvent::KC_UNASSIGNED;
        }
#endif
        mEngine->fireEvent(e);
        break;
    }
  }

  void SDLInputListener::handleMouseEvent(SDL_Event* event)
  {
    int x, y;
    SDL_GetMouseState(&x, &y);

    switch(event->type) {
      case SDL_MOUSEWHEEL:
        fireMouseEvent(MouseEvent::MOUSE_MOVE, x, y, mPreviousZ + event->wheel.y * 20);
        break;
      case SDL_MOUSEMOTION:
        fireMouseEvent(MouseEvent::MOUSE_MOVE, x, y, mPreviousZ);
        break;
      case SDL_MOUSEBUTTONDOWN:
        fireMouseEvent(MouseEvent::MOUSE_DOWN, x, y, mPreviousZ, mapButtonType(event->button.button));
        break;
      case SDL_MOUSEBUTTONUP:
        fireMouseEvent(MouseEvent::MOUSE_UP, x, y, mPreviousZ, mapButtonType(event->button.button));
        break;
    }
  }

  void SDLInputListener::handleKeyboardEvent(SDL_Event* event)
  {
    SDL_Keycode vc = event->key.keysym.sym;
    SDL_Keycode sc = event->key.keysym.scancode;
    if(mKeyMap.count(sc) == 0) {
      LOG(WARNING) << "Unhandled " <<  SDL_GetKeyName(vc) << " " << (event->type == SDL_KEYDOWN ? "down" : "up");
      return;
    }

    unsigned int text = 0;
    if(vc > 0x20 && vc < 0x7A) {
      text = vc;
    }

    KeyboardEvent::Key key = mKeyMap[sc];
    KeyboardEvent e(event->key.state == SDL_PRESSED ? KeyboardEvent::KEY_DOWN : KeyboardEvent::KEY_UP, key, text, SDL_GetModState());
    mEngine->fireEvent(e);
  }

  const MouseEvent::ButtonType SDLInputListener::mapButtonType(Uint8 sdlButtonID)
  {
    MouseEvent::ButtonType res;
    switch(sdlButtonID) {
      case SDL_BUTTON_LEFT:
        res = MouseEvent::Left;
        break;
      case SDL_BUTTON_MIDDLE:
        res = MouseEvent::Middle;
        break;
      case SDL_BUTTON_RIGHT:
        res = MouseEvent::Right;
        break;
      case SDL_BUTTON_X1:
        res = MouseEvent::Button3;
        break;
      case SDL_BUTTON_X2:
        res = MouseEvent::Button4;
        break;
    }
    return res;
  }
}
