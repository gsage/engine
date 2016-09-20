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

#include "input/OisInputListener.h"
#include "Logger.h"
#ifdef OIS_APPLE_PLATFORM
#include <Carbon/Carbon.h>
#endif

namespace Gsage {

  AutoRepeatKey::AutoRepeatKey(float a_fRepeatDelay, float a_fInitialDelay):
    m_nKey(KeyboardEvent::KC_UNASSIGNED),
    m_fRepeatDelay(a_fRepeatDelay),
    m_fInitialDelay(a_fInitialDelay)
  {}

  void AutoRepeatKey::begin(const KeyboardEvent& evt) {
    m_nKey = evt.key;
    m_nChar = evt.text;

    m_fElapsed = 0;
    m_fDelay = m_fInitialDelay;
  }

  void AutoRepeatKey::end(const KeyboardEvent& evt) {
    if (m_nKey != evt.key) return;

    m_nKey = KeyboardEvent::KC_UNASSIGNED;
  }

  void AutoRepeatKey::update(float a_fElapsed) {
    if (m_nKey == KeyboardEvent::KC_UNASSIGNED) return;

    m_fElapsed += a_fElapsed;
    if (m_fElapsed < m_fDelay) return;

    m_fElapsed -= m_fDelay;
    m_fDelay = m_fRepeatDelay;

    do {
      repeatKey(m_nKey, m_nChar);

      m_fElapsed -= m_fRepeatDelay;
    } while (m_fElapsed >= m_fRepeatDelay);

    m_fElapsed = 0;
  }

  OisInputListener::OisInputListener(Ogre::RenderWindow* window, EventDispatcher* eventRedirect) :
    mMouse(0),
    mKeyboard(0),
    mInputManager(0),
    mEventRedirect(eventRedirect),
    mWindow(0)
  {
    LOG(INFO) << "*** Initializing OIS ***";
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    window->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
#if defined OIS_WIN32_PLATFORM
    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
#elif defined OIS_LINUX_PLATFORM
    pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
    pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("true")));
    pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
    pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
#endif

#ifdef OIS_APPLE_PLATFORM
    CGDisplayShowCursor(kCGDirectMainDisplay);
    CGAssociateMouseAndMouseCursorPosition(TRUE);
#endif

    mInputManager = OIS::InputManager::createInputSystem( pl );
    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));
    windowResized(window);
    Ogre::WindowEventUtilities::addWindowEventListener(window, this);

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    mKeyMap[OIS::KC_UNASSIGNED] = KeyboardEvent::KC_UNASSIGNED;
    mKeyMap[OIS::KC_ESCAPE] = KeyboardEvent::KC_ESCAPE;
    mKeyMap[OIS::KC_1] = KeyboardEvent::KC_1;
    mKeyMap[OIS::KC_2] = KeyboardEvent::KC_2;
    mKeyMap[OIS::KC_3] = KeyboardEvent::KC_3;
    mKeyMap[OIS::KC_4] = KeyboardEvent::KC_4;
    mKeyMap[OIS::KC_5] = KeyboardEvent::KC_5;
    mKeyMap[OIS::KC_6] = KeyboardEvent::KC_6;
    mKeyMap[OIS::KC_7] = KeyboardEvent::KC_7;
    mKeyMap[OIS::KC_8] = KeyboardEvent::KC_8;
    mKeyMap[OIS::KC_9] = KeyboardEvent::KC_9;
    mKeyMap[OIS::KC_0] = KeyboardEvent::KC_0;
    mKeyMap[OIS::KC_MINUS] = KeyboardEvent::KC_MINUS;
    mKeyMap[OIS::KC_EQUALS] = KeyboardEvent::KC_EQUALS;
    mKeyMap[OIS::KC_BACK] = KeyboardEvent::KC_BACK;
    mKeyMap[OIS::KC_TAB] = KeyboardEvent::KC_TAB;
    mKeyMap[OIS::KC_Q] = KeyboardEvent::KC_Q;
    mKeyMap[OIS::KC_W] = KeyboardEvent::KC_W;
    mKeyMap[OIS::KC_E] = KeyboardEvent::KC_E;
    mKeyMap[OIS::KC_R] = KeyboardEvent::KC_R;
    mKeyMap[OIS::KC_T] = KeyboardEvent::KC_T;
    mKeyMap[OIS::KC_Y] = KeyboardEvent::KC_Y;
    mKeyMap[OIS::KC_U] = KeyboardEvent::KC_U;
    mKeyMap[OIS::KC_I] = KeyboardEvent::KC_I;
    mKeyMap[OIS::KC_O] = KeyboardEvent::KC_O;
    mKeyMap[OIS::KC_P] = KeyboardEvent::KC_P;
    mKeyMap[OIS::KC_LBRACKET] = KeyboardEvent::KC_LBRACKET;
    mKeyMap[OIS::KC_RBRACKET] = KeyboardEvent::KC_RBRACKET;
    mKeyMap[OIS::KC_RETURN] = KeyboardEvent::KC_RETURN;
    mKeyMap[OIS::KC_LCONTROL] = KeyboardEvent::KC_LCONTROL;
    mKeyMap[OIS::KC_A] = KeyboardEvent::KC_A;
    mKeyMap[OIS::KC_S] = KeyboardEvent::KC_S;
    mKeyMap[OIS::KC_D] = KeyboardEvent::KC_D;
    mKeyMap[OIS::KC_F] = KeyboardEvent::KC_F;
    mKeyMap[OIS::KC_G] = KeyboardEvent::KC_G;
    mKeyMap[OIS::KC_H] = KeyboardEvent::KC_H;
    mKeyMap[OIS::KC_J] = KeyboardEvent::KC_J;
    mKeyMap[OIS::KC_K] = KeyboardEvent::KC_K;
    mKeyMap[OIS::KC_L] = KeyboardEvent::KC_L;
    mKeyMap[OIS::KC_SEMICOLON] = KeyboardEvent::KC_SEMICOLON;
    mKeyMap[OIS::KC_APOSTROPHE] = KeyboardEvent::KC_APOSTROPHE;
    mKeyMap[OIS::KC_GRAVE] = KeyboardEvent::KC_GRAVE;
    mKeyMap[OIS::KC_LSHIFT] = KeyboardEvent::KC_LSHIFT;
    mKeyMap[OIS::KC_BACKSLASH] = KeyboardEvent::KC_BACKSLASH;
    mKeyMap[OIS::KC_Z] = KeyboardEvent::KC_Z;
    mKeyMap[OIS::KC_X] = KeyboardEvent::KC_X;
    mKeyMap[OIS::KC_C] = KeyboardEvent::KC_C;
    mKeyMap[OIS::KC_V] = KeyboardEvent::KC_V;
    mKeyMap[OIS::KC_B] = KeyboardEvent::KC_B;
    mKeyMap[OIS::KC_N] = KeyboardEvent::KC_N;
    mKeyMap[OIS::KC_M] = KeyboardEvent::KC_M;
    mKeyMap[OIS::KC_COMMA] = KeyboardEvent::KC_COMMA;
    mKeyMap[OIS::KC_PERIOD] = KeyboardEvent::KC_PERIOD;
    mKeyMap[OIS::KC_SLASH] = KeyboardEvent::KC_SLASH;
    mKeyMap[OIS::KC_RSHIFT] = KeyboardEvent::KC_RSHIFT;
    mKeyMap[OIS::KC_MULTIPLY] = KeyboardEvent::KC_MULTIPLY;
    mKeyMap[OIS::KC_LMENU] = KeyboardEvent::KC_LMENU;
    mKeyMap[OIS::KC_SPACE] = KeyboardEvent::KC_SPACE;
    mKeyMap[OIS::KC_CAPITAL] = KeyboardEvent::KC_CAPITAL;
    mKeyMap[OIS::KC_F1] = KeyboardEvent::KC_F1;
    mKeyMap[OIS::KC_F2] = KeyboardEvent::KC_F2;
    mKeyMap[OIS::KC_F3] = KeyboardEvent::KC_F3;
    mKeyMap[OIS::KC_F4] = KeyboardEvent::KC_F4;
    mKeyMap[OIS::KC_F5] = KeyboardEvent::KC_F5;
    mKeyMap[OIS::KC_F6] = KeyboardEvent::KC_F6;
    mKeyMap[OIS::KC_F7] = KeyboardEvent::KC_F7;
    mKeyMap[OIS::KC_F8] = KeyboardEvent::KC_F8;
    mKeyMap[OIS::KC_F9] = KeyboardEvent::KC_F9;
    mKeyMap[OIS::KC_F10] = KeyboardEvent::KC_F10;
    mKeyMap[OIS::KC_NUMLOCK] = KeyboardEvent::KC_NUMLOCK;
    mKeyMap[OIS::KC_SCROLL] = KeyboardEvent::KC_SCROLL;
    mKeyMap[OIS::KC_NUMPAD7] = KeyboardEvent::KC_NUMPAD7;
    mKeyMap[OIS::KC_NUMPAD8] = KeyboardEvent::KC_NUMPAD8;
    mKeyMap[OIS::KC_NUMPAD9] = KeyboardEvent::KC_NUMPAD9;
    mKeyMap[OIS::KC_SUBTRACT] = KeyboardEvent::KC_SUBTRACT;
    mKeyMap[OIS::KC_NUMPAD4] = KeyboardEvent::KC_NUMPAD4;
    mKeyMap[OIS::KC_NUMPAD5] = KeyboardEvent::KC_NUMPAD5;
    mKeyMap[OIS::KC_NUMPAD6] = KeyboardEvent::KC_NUMPAD6;
    mKeyMap[OIS::KC_ADD] = KeyboardEvent::KC_ADD;
    mKeyMap[OIS::KC_NUMPAD1] = KeyboardEvent::KC_NUMPAD1;
    mKeyMap[OIS::KC_NUMPAD2] = KeyboardEvent::KC_NUMPAD2;
    mKeyMap[OIS::KC_NUMPAD3] = KeyboardEvent::KC_NUMPAD3;
    mKeyMap[OIS::KC_NUMPAD0] = KeyboardEvent::KC_NUMPAD0;
    mKeyMap[OIS::KC_DECIMAL] = KeyboardEvent::KC_DECIMAL;
    mKeyMap[OIS::KC_OEM_102] = KeyboardEvent::KC_OEM_102;
    mKeyMap[OIS::KC_F11] = KeyboardEvent::KC_F11;
    mKeyMap[OIS::KC_F12] = KeyboardEvent::KC_F12;
    mKeyMap[OIS::KC_F13] = KeyboardEvent::KC_F13;
    mKeyMap[OIS::KC_F14] = KeyboardEvent::KC_F14;
    mKeyMap[OIS::KC_F15] = KeyboardEvent::KC_F15;
    mKeyMap[OIS::KC_KANA] = KeyboardEvent::KC_KANA;
    mKeyMap[OIS::KC_ABNT_C1] = KeyboardEvent::KC_ABNT_C1;
    mKeyMap[OIS::KC_CONVERT] = KeyboardEvent::KC_CONVERT;
    mKeyMap[OIS::KC_NOCONVERT] = KeyboardEvent::KC_NOCONVERT;
    mKeyMap[OIS::KC_YEN] = KeyboardEvent::KC_YEN;
    mKeyMap[OIS::KC_ABNT_C2] = KeyboardEvent::KC_ABNT_C2;
    mKeyMap[OIS::KC_NUMPADEQUALS] = KeyboardEvent::KC_NUMPADEQUALS;
    mKeyMap[OIS::KC_PREVTRACK] = KeyboardEvent::KC_PREVTRACK;
    mKeyMap[OIS::KC_AT] = KeyboardEvent::KC_AT;
    mKeyMap[OIS::KC_COLON] = KeyboardEvent::KC_COLON;
    mKeyMap[OIS::KC_UNDERLINE] = KeyboardEvent::KC_UNDERLINE;
    mKeyMap[OIS::KC_KANJI] = KeyboardEvent::KC_KANJI;
    mKeyMap[OIS::KC_STOP] = KeyboardEvent::KC_STOP;
    mKeyMap[OIS::KC_AX] = KeyboardEvent::KC_AX;
    mKeyMap[OIS::KC_UNLABELED] = KeyboardEvent::KC_UNLABELED;
    mKeyMap[OIS::KC_NEXTTRACK] = KeyboardEvent::KC_NEXTTRACK;
    mKeyMap[OIS::KC_NUMPADENTER] = KeyboardEvent::KC_NUMPADENTER;
    mKeyMap[OIS::KC_RCONTROL] = KeyboardEvent::KC_RCONTROL;
    mKeyMap[OIS::KC_MUTE] = KeyboardEvent::KC_MUTE;
    mKeyMap[OIS::KC_CALCULATOR] = KeyboardEvent::KC_CALCULATOR;
    mKeyMap[OIS::KC_PLAYPAUSE] = KeyboardEvent::KC_PLAYPAUSE;
    mKeyMap[OIS::KC_MEDIASTOP] = KeyboardEvent::KC_MEDIASTOP;
    mKeyMap[OIS::KC_VOLUMEDOWN] = KeyboardEvent::KC_VOLUMEDOWN;
    mKeyMap[OIS::KC_VOLUMEUP] = KeyboardEvent::KC_VOLUMEUP;
    mKeyMap[OIS::KC_WEBHOME] = KeyboardEvent::KC_WEBHOME;
    mKeyMap[OIS::KC_NUMPADCOMMA] = KeyboardEvent::KC_NUMPADCOMMA;
    mKeyMap[OIS::KC_DIVIDE] = KeyboardEvent::KC_DIVIDE;
    mKeyMap[OIS::KC_SYSRQ] = KeyboardEvent::KC_SYSRQ;
    mKeyMap[OIS::KC_RMENU] = KeyboardEvent::KC_RMENU;
    mKeyMap[OIS::KC_PAUSE] = KeyboardEvent::KC_PAUSE;
    mKeyMap[OIS::KC_HOME] = KeyboardEvent::KC_HOME;
    mKeyMap[OIS::KC_UP] = KeyboardEvent::KC_UP;
    mKeyMap[OIS::KC_PGUP] = KeyboardEvent::KC_PGUP;
    mKeyMap[OIS::KC_LEFT] = KeyboardEvent::KC_LEFT;
    mKeyMap[OIS::KC_RIGHT] = KeyboardEvent::KC_RIGHT;
    mKeyMap[OIS::KC_END] = KeyboardEvent::KC_END;
    mKeyMap[OIS::KC_DOWN] = KeyboardEvent::KC_DOWN;
    mKeyMap[OIS::KC_PGDOWN] = KeyboardEvent::KC_PGDOWN;
    mKeyMap[OIS::KC_INSERT] = KeyboardEvent::KC_INSERT;
    mKeyMap[OIS::KC_DELETE] = KeyboardEvent::KC_DELETE;
    mKeyMap[OIS::KC_LWIN] = KeyboardEvent::KC_LWIN;
    mKeyMap[OIS::KC_RWIN] = KeyboardEvent::KC_RWIN;
    mKeyMap[OIS::KC_APPS] = KeyboardEvent::KC_APPS;
    mKeyMap[OIS::KC_POWER] = KeyboardEvent::KC_POWER;
    mKeyMap[OIS::KC_SLEEP] = KeyboardEvent::KC_SLEEP;
    mKeyMap[OIS::KC_WAKE] = KeyboardEvent::KC_WAKE;
    mKeyMap[OIS::KC_WEBSEARCH] = KeyboardEvent::KC_WEBSEARCH;
    mKeyMap[OIS::KC_WEBFAVORITES] = KeyboardEvent::KC_WEBFAVORITES;
    mKeyMap[OIS::KC_WEBREFRESH] = KeyboardEvent::KC_WEBREFRESH;
    mKeyMap[OIS::KC_WEBSTOP] = KeyboardEvent::KC_WEBSTOP;
    mKeyMap[OIS::KC_WEBFORWARD] = KeyboardEvent::KC_WEBFORWARD;
    mKeyMap[OIS::KC_WEBBACK] = KeyboardEvent::KC_WEBBACK;
    mKeyMap[OIS::KC_MYCOMPUTER] = KeyboardEvent::KC_MYCOMPUTER;
    mKeyMap[OIS::KC_MAIL] = KeyboardEvent::KC_MAIL;
    mKeyMap[OIS::KC_MEDIASELECT] = KeyboardEvent::KC_MEDIASELECT;

    mModifierMap[OIS::Keyboard::Ctrl] = KeyboardEvent::Ctrl;
    mModifierMap[OIS::Keyboard::Shift] = KeyboardEvent::Shift;
    mModifierMap[OIS::Keyboard::Alt] = KeyboardEvent::Alt;
  }

  OisInputListener::~OisInputListener()
  {
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
  }

  void OisInputListener::update(const float& time)
  {
    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();

    AutoRepeatKey::update(time);
  }

  void OisInputListener::windowResized(Ogre::RenderWindow* window)
  {
    unsigned int width, height, depth;
    int left, top;
    window->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
  }

  void OisInputListener::windowClosed(Ogre::RenderWindow* window)
  {
    if(window == mWindow)
    {
      if(mInputManager)
      {
        LOG(INFO) << "*** Stopping OIS ***";

        mInputManager->destroyInputObject( mMouse );
        mInputManager->destroyInputObject( mKeyboard );

        OIS::InputManager::destroyInputSystem(mInputManager);
        mInputManager = 0;
      }
    }
  }

  bool OisInputListener::mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
  {
    fireMouseEvent(MouseEvent::MOUSE_DOWN, arg, id);
    return true;
  }

  bool OisInputListener::mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
  {
    fireMouseEvent(MouseEvent::MOUSE_UP, arg, id);
    return true;
  }

  bool OisInputListener::mouseMoved(const OIS::MouseEvent& arg)
  {
    fireMouseEvent(MouseEvent::MOUSE_MOVE, arg);
    return true;
  }

  void OisInputListener::fireMouseEvent(const std::string& type, const OIS::MouseEvent& event, const OIS::MouseButtonID& oisButtonID)
  {
    MouseEvent e(type, event.state.width, event.state.height, mapButtonType(oisButtonID));
    e.setAbsolutePosition(event.state.X.abs, event.state.Y.abs, event.state.Z.abs);
    e.setRelativePosition(event.state.X.rel, event.state.Y.rel, event.state.Z.rel);
    if(mEventRedirect)
      mEventRedirect->fireEvent(e);
    else
      fireEvent(e);
  }

  void OisInputListener::fireMouseEvent(const std::string& type, const OIS::MouseEvent& event)
  {
    MouseEvent e(type, event.state.width, event.state.height);
    e.setAbsolutePosition(event.state.X.abs, event.state.Y.abs, event.state.Z.abs);
    e.setRelativePosition(event.state.X.rel, event.state.Y.rel, event.state.Z.rel);
    if(mEventRedirect)
      mEventRedirect->fireEvent(e);
    else
      fireEvent(e);
  }

  MouseEvent::ButtonType OisInputListener::mapButtonType(const OIS::MouseButtonID& oisButtonID)
  {
    MouseEvent::ButtonType res;
    switch(oisButtonID)
    {
      case OIS::MB_Right:
        res = MouseEvent::Right;
        break;
      case OIS::MB_Left:
        res = MouseEvent::Left;
        break;
      case OIS::MB_Middle:
        res = MouseEvent::Middle;
        break;
      case OIS::MB_Button3:
        res = MouseEvent::Button3;
        break;
      case OIS::MB_Button4:
        res = MouseEvent::Button4;
        break;
      case OIS::MB_Button5:
        res = MouseEvent::Button5;
        break;
      case OIS::MB_Button6:
        res = MouseEvent::Button6;
        break;
      case OIS::MB_Button7:
        res = MouseEvent::Button7;
        break;
    }
    return res;
  }

  bool OisInputListener::keyPressed(const OIS::KeyEvent& e)
  {
    KeyboardEvent::Key key = mKeyMap[e.key];
    KeyboardEvent event(KeyboardEvent::KEY_DOWN, key, e.text, getModifiersState());
    mEventRedirect->fireEvent(event);
    begin(event);
    return true;
  }

  bool OisInputListener::keyReleased(const OIS::KeyEvent& e)
  {
    KeyboardEvent::Key key = mKeyMap[e.key];
    KeyboardEvent event(KeyboardEvent::KEY_UP, key, e.text, getModifiersState());
    mEventRedirect->fireEvent(event);
    end(event);
    return true;
  }

  void OisInputListener::repeatKey(KeyboardEvent::Key key, unsigned int character)
  {
    mEventRedirect->fireEvent(KeyboardEvent(KeyboardEvent::KEY_DOWN, key, character, getModifiersState()));
  }

  unsigned int OisInputListener::getModifiersState()
  {
    OIS::Keyboard::Modifier modifiers[] = {
      OIS::Keyboard::Ctrl,
      OIS::Keyboard::Alt,
      OIS::Keyboard::Shift
    };

    unsigned int res = 0;
    for(int i = 0; i < 3; i++)
    {
      if(mKeyboard->isModifierDown(modifiers[i]))
          res |= mModifierMap[modifiers[i]];
    }

    return res;
  }
}
