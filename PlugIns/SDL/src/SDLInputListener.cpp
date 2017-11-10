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

  InputHandler* SDLInputFactory::create(size_t windowHandle, Engine* engine)
  {
    if(!mCore) {
      return 0;
    }

    SDL_StartTextInput();
    SDL_ShowCursor(false);
    SDLInputListener* l = new SDLInputListener(windowHandle, engine);
    mCore->addEventListener(l);
    return l;
  }

  void SDLInputFactory::setSDLCore(SDLCore* core)
  {
    mCore = core;
  }

  SDLInputListener::SDLInputListener(size_t handle, Engine* eventRedirect)
    : InputHandler(handle, eventRedirect)
  {
    mKeyMap[SDLK_ESCAPE] = KeyboardEvent::KC_ESCAPE;
    mKeyMap[SDLK_1] = KeyboardEvent::KC_1;
    mKeyMap[SDLK_2] = KeyboardEvent::KC_2;
    mKeyMap[SDLK_3] = KeyboardEvent::KC_3;
    mKeyMap[SDLK_4] = KeyboardEvent::KC_4;
    mKeyMap[SDLK_5] = KeyboardEvent::KC_5;
    mKeyMap[SDLK_6] = KeyboardEvent::KC_6;
    mKeyMap[SDLK_7] = KeyboardEvent::KC_7;
    mKeyMap[SDLK_8] = KeyboardEvent::KC_8;
    mKeyMap[SDLK_9] = KeyboardEvent::KC_9;
    mKeyMap[SDLK_0] = KeyboardEvent::KC_0;
    mKeyMap[SDLK_MINUS] = KeyboardEvent::KC_MINUS;
    mKeyMap[SDLK_EQUALS] = KeyboardEvent::KC_EQUALS;
    mKeyMap[SDLK_BACKSPACE] = KeyboardEvent::KC_BACK;
    mKeyMap[SDLK_TAB] = KeyboardEvent::KC_TAB;
    mKeyMap[SDLK_q] = KeyboardEvent::KC_Q;
    mKeyMap[SDLK_w] = KeyboardEvent::KC_W;
    mKeyMap[SDLK_e] = KeyboardEvent::KC_E;
    mKeyMap[SDLK_r] = KeyboardEvent::KC_R;
    mKeyMap[SDLK_t] = KeyboardEvent::KC_T;
    mKeyMap[SDLK_y] = KeyboardEvent::KC_Y;
    mKeyMap[SDLK_u] = KeyboardEvent::KC_U;
    mKeyMap[SDLK_i] = KeyboardEvent::KC_I;
    mKeyMap[SDLK_o] = KeyboardEvent::KC_O;
    mKeyMap[SDLK_p] = KeyboardEvent::KC_P;
    mKeyMap[SDLK_KP_LEFTBRACE] = KeyboardEvent::KC_LBRACKET;
    mKeyMap[SDLK_KP_RIGHTBRACE] = KeyboardEvent::KC_RBRACKET;
    mKeyMap[SDLK_RETURN] = KeyboardEvent::KC_RETURN;
    mKeyMap[SDLK_LCTRL] = KeyboardEvent::KC_LCONTROL;
    mKeyMap[SDLK_a] = KeyboardEvent::KC_A;
    mKeyMap[SDLK_s] = KeyboardEvent::KC_S;
    mKeyMap[SDLK_d] = KeyboardEvent::KC_D;
    mKeyMap[SDLK_f] = KeyboardEvent::KC_F;
    mKeyMap[SDLK_g] = KeyboardEvent::KC_G;
    mKeyMap[SDLK_h] = KeyboardEvent::KC_H;
    mKeyMap[SDLK_j] = KeyboardEvent::KC_J;
    mKeyMap[SDLK_k] = KeyboardEvent::KC_K;
    mKeyMap[SDLK_l] = KeyboardEvent::KC_L;
    mKeyMap[SDLK_SEMICOLON] = KeyboardEvent::KC_SEMICOLON;
    mKeyMap[SDLK_QUOTE] = KeyboardEvent::KC_APOSTROPHE;
    mKeyMap[SDLK_BACKQUOTE] = KeyboardEvent::KC_GRAVE;
    mKeyMap[SDLK_LSHIFT] = KeyboardEvent::KC_LSHIFT;
    mKeyMap[SDLK_BACKSLASH] = KeyboardEvent::KC_BACKSLASH;
    mKeyMap[SDLK_z] = KeyboardEvent::KC_Z;
    mKeyMap[SDLK_x] = KeyboardEvent::KC_X;
    mKeyMap[SDLK_c] = KeyboardEvent::KC_C;
    mKeyMap[SDLK_v] = KeyboardEvent::KC_V;
    mKeyMap[SDLK_b] = KeyboardEvent::KC_B;
    mKeyMap[SDLK_n] = KeyboardEvent::KC_N;
    mKeyMap[SDLK_m] = KeyboardEvent::KC_M;
    mKeyMap[SDLK_COMMA] = KeyboardEvent::KC_COMMA;
    mKeyMap[SDLK_PERIOD] = KeyboardEvent::KC_PERIOD;
    mKeyMap[SDLK_SLASH] = KeyboardEvent::KC_SLASH;
    mKeyMap[SDLK_RSHIFT] = KeyboardEvent::KC_RSHIFT;
    mKeyMap[SDLK_KP_MULTIPLY] = KeyboardEvent::KC_MULTIPLY;
    mKeyMap[SDLK_MENU] = KeyboardEvent::KC_LMENU;
    mKeyMap[SDLK_SPACE] = KeyboardEvent::KC_SPACE;
    mKeyMap[SDLK_CAPSLOCK] = KeyboardEvent::KC_CAPITAL;
    mKeyMap[SDLK_F1] = KeyboardEvent::KC_F1;
    mKeyMap[SDLK_F2] = KeyboardEvent::KC_F2;
    mKeyMap[SDLK_F3] = KeyboardEvent::KC_F3;
    mKeyMap[SDLK_F4] = KeyboardEvent::KC_F4;
    mKeyMap[SDLK_F5] = KeyboardEvent::KC_F5;
    mKeyMap[SDLK_F6] = KeyboardEvent::KC_F6;
    mKeyMap[SDLK_F7] = KeyboardEvent::KC_F7;
    mKeyMap[SDLK_F8] = KeyboardEvent::KC_F8;
    mKeyMap[SDLK_F9] = KeyboardEvent::KC_F9;
    mKeyMap[SDLK_F10] = KeyboardEvent::KC_F10;
    mKeyMap[SDLK_NUMLOCKCLEAR] = KeyboardEvent::KC_NUMLOCK;
    mKeyMap[SDLK_SCROLLLOCK] = KeyboardEvent::KC_SCROLL;
    mKeyMap[SDLK_KP_7] = KeyboardEvent::KC_NUMPAD7;
    mKeyMap[SDLK_KP_8] = KeyboardEvent::KC_NUMPAD8;
    mKeyMap[SDLK_KP_9] = KeyboardEvent::KC_NUMPAD9;
    mKeyMap[SDLK_KP_MINUS] = KeyboardEvent::KC_SUBTRACT;
    mKeyMap[SDLK_KP_4] = KeyboardEvent::KC_NUMPAD4;
    mKeyMap[SDLK_KP_5] = KeyboardEvent::KC_NUMPAD5;
    mKeyMap[SDLK_KP_6] = KeyboardEvent::KC_NUMPAD6;
    mKeyMap[SDLK_KP_PLUS] = KeyboardEvent::KC_ADD;
    mKeyMap[SDLK_KP_1] = KeyboardEvent::KC_NUMPAD1;
    mKeyMap[SDLK_KP_2] = KeyboardEvent::KC_NUMPAD2;
    mKeyMap[SDLK_KP_3] = KeyboardEvent::KC_NUMPAD3;
    mKeyMap[SDLK_KP_0] = KeyboardEvent::KC_NUMPAD0;
    mKeyMap[SDLK_KP_DECIMAL] = KeyboardEvent::KC_DECIMAL;
    mKeyMap[SDLK_F11] = KeyboardEvent::KC_F11;
    mKeyMap[SDLK_F12] = KeyboardEvent::KC_F12;
    mKeyMap[SDLK_F13] = KeyboardEvent::KC_F13;
    mKeyMap[SDLK_F14] = KeyboardEvent::KC_F14;
    mKeyMap[SDLK_F15] = KeyboardEvent::KC_F15;
    mKeyMap[SDLK_KP_EQUALS] = KeyboardEvent::KC_NUMPADEQUALS;
    mKeyMap[SDLK_AUDIOPREV] = KeyboardEvent::KC_PREVTRACK;
    mKeyMap[SDLK_COLON] = KeyboardEvent::KC_COLON;
    mKeyMap[SDLK_UNDERSCORE] = KeyboardEvent::KC_UNDERLINE;
    mKeyMap[SDLK_STOP] = KeyboardEvent::KC_STOP;
    mKeyMap[SDLK_AUDIONEXT] = KeyboardEvent::KC_NEXTTRACK;
    mKeyMap[SDLK_KP_ENTER] = KeyboardEvent::KC_NUMPADENTER;
    mKeyMap[SDLK_RCTRL] = KeyboardEvent::KC_RCONTROL;
    mKeyMap[SDLK_AUDIOMUTE] = KeyboardEvent::KC_MUTE;
    mKeyMap[SDLK_CALCULATOR] = KeyboardEvent::KC_CALCULATOR;
    mKeyMap[SDLK_AUDIOPLAY] = KeyboardEvent::KC_PLAYPAUSE;
    mKeyMap[SDLK_AUDIOSTOP] = KeyboardEvent::KC_MEDIASTOP;
    mKeyMap[SDLK_VOLUMEDOWN] = KeyboardEvent::KC_VOLUMEDOWN;
    mKeyMap[SDLK_VOLUMEUP] = KeyboardEvent::KC_VOLUMEUP;
    mKeyMap[SDLK_WWW] = KeyboardEvent::KC_WEBHOME;
    mKeyMap[SDLK_KP_COMMA] = KeyboardEvent::KC_NUMPADCOMMA;
    mKeyMap[SDLK_KP_DIVIDE] = KeyboardEvent::KC_DIVIDE;
    mKeyMap[SDLK_SYSREQ] = KeyboardEvent::KC_SYSRQ;
    mKeyMap[SDLK_PAUSE] = KeyboardEvent::KC_PAUSE;
    mKeyMap[SDLK_HOME] = KeyboardEvent::KC_HOME;
    mKeyMap[SDLK_UP] = KeyboardEvent::KC_UP;
    mKeyMap[SDLK_PAGEUP] = KeyboardEvent::KC_PGUP;
    mKeyMap[SDLK_LEFT] = KeyboardEvent::KC_LEFT;
    mKeyMap[SDLK_RIGHT] = KeyboardEvent::KC_RIGHT;
    mKeyMap[SDLK_END] = KeyboardEvent::KC_END;
    mKeyMap[SDLK_DOWN] = KeyboardEvent::KC_DOWN;
    mKeyMap[SDLK_PAGEDOWN] = KeyboardEvent::KC_PGDOWN;
    mKeyMap[SDLK_INSERT] = KeyboardEvent::KC_INSERT;
    mKeyMap[SDLK_DELETE] = KeyboardEvent::KC_DELETE;
    mKeyMap[SDLK_LGUI] = KeyboardEvent::KC_LWIN;
    mKeyMap[SDLK_RGUI] = KeyboardEvent::KC_RWIN;
    mKeyMap[SDLK_POWER] = KeyboardEvent::KC_POWER;
    mKeyMap[SDLK_SLEEP] = KeyboardEvent::KC_SLEEP;
    mKeyMap[SDLK_AC_SEARCH] = KeyboardEvent::KC_WEBSEARCH;
    mKeyMap[SDLK_AC_BOOKMARKS] = KeyboardEvent::KC_WEBFAVORITES;
    mKeyMap[SDLK_AC_REFRESH] = KeyboardEvent::KC_WEBREFRESH;
    mKeyMap[SDLK_AC_STOP] = KeyboardEvent::KC_WEBSTOP;
    mKeyMap[SDLK_AC_FORWARD] = KeyboardEvent::KC_WEBFORWARD;
    mKeyMap[SDLK_AC_BACK] = KeyboardEvent::KC_WEBBACK;
    mKeyMap[SDLK_COMPUTER] = KeyboardEvent::KC_MYCOMPUTER;
    mKeyMap[SDLK_MAIL] = KeyboardEvent::KC_MAIL;
    mKeyMap[SDLK_MEDIASELECT] = KeyboardEvent::KC_MEDIASELECT;
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
        mEngine->fireEvent(TextInputEvent(TextInputEvent::INPUT, event->text.text));
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
    if(mKeyMap.count(event->key.keysym.sym) == 0) {
      return;
    }

    KeyboardEvent::Key key = mKeyMap[event->key.keysym.sym];
    KeyboardEvent e(event->type == SDL_KEYDOWN ? KeyboardEvent::KEY_DOWN : KeyboardEvent::KEY_UP, key, 0, SDL_GetModState());
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

