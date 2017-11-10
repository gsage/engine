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

#ifndef _INPUT_MANAGER_H_
#define _INPUT_MANAGER_H_

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include "MouseEvent.h"
#include "Engine.h"
#include "EventDispatcher.h"
#include "GsageFacade.h"
#include "KeyboardEvent.h"

#include "input/InputFactory.h"

#define REPEAT_KEY_INTERVAL 0.04f
#define REPEAT_KEY_START_DELAY 0.3f

namespace Gsage {

  class AutoRepeatKey {
    private:

      KeyboardEvent::Key m_nKey;
      unsigned int m_nChar;

      float m_fElapsed;
      float m_fDelay;

      float m_fRepeatDelay;
      float m_fInitialDelay;

    protected:

      virtual void repeatKey(KeyboardEvent::Key a_nKey, unsigned int a_nChar) = 0;

    public:

      AutoRepeatKey(float a_fRepeatDelay = REPEAT_KEY_INTERVAL, float a_fInitialDelay = REPEAT_KEY_START_DELAY);

      void begin(const KeyboardEvent& evt);

      void end(const KeyboardEvent& evt);

      void update(float a_fElapsed); // Elapsed time in seconds
  };

  class OisInputFactory : public AbstractInputFactory
  {
    public:
      OisInputFactory() {};
      virtual ~OisInputFactory(){};

      /**
       * Create ois input handler
       */
      InputHandler* create(size_t windowHandle, Engine* engine);
  };

  class OisInputListener : public InputHandler, public OIS::MouseListener, public OIS::KeyListener, public EventDispatcher, AutoRepeatKey
  {
    public:
      OisInputListener(size_t handle, Engine* eventRedirect = 0);
      ~OisInputListener();

      void update(const float& time);
      // Gsage::InputHandler
      virtual void handleResize(unsigned int width, unsigned int height);
      virtual void handleClose();
      // OIS::MouseListener
      virtual bool mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id);
      virtual bool mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id);
      virtual bool mouseMoved(const OIS::MouseEvent& arg);
      // OIS::KeyListener
      virtual bool keyPressed(const OIS::KeyEvent& e);
      virtual bool keyReleased(const OIS::KeyEvent& e);

      OIS::Mouse* mouse() { return mMouse; }
      OIS::Keyboard* keyboard() { return mKeyboard; }

    protected:
      virtual void repeatKey(KeyboardEvent::Key key, unsigned int character);
    private:
      OIS::Mouse* mMouse;
      OIS::Keyboard* mKeyboard;
      OIS::InputManager* mInputManager;
      Engine* mEventRedirect;
      size_t mHandle;


      void fireMouseEvent(Event::ConstType type, const OIS::MouseEvent& event);
      void fireMouseEvent(Event::ConstType type, const OIS::MouseEvent& event, const MouseEvent::ButtonType& button);

      MouseEvent::ButtonType mapButtonType(const OIS::MouseButtonID& oisButtonID);

      unsigned int getModifiersState();
      typedef std::map<OIS::KeyCode, KeyboardEvent::Key> KeyIdentifierMap;
      typedef std::map<OIS::Keyboard::Modifier, KeyboardEvent::Modifier> ModifierMap;
      KeyIdentifierMap mKeyMap;
      ModifierMap mModifierMap;
  };
}

#endif
