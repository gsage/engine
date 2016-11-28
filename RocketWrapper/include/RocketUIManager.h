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

#ifndef _RocketUIManager_H_
#define _RocketUIManager_H_

#include <Rocket/Core/Core.h>
#include <Rocket/Controls.h>
#include <Rocket/Core/RenderInterface.h>
#include <Rocket/Core/SystemInterface.h>

#include <OISKeyboard.h>

#include "UIManager.h"
#include "EventSubscriber.h"
#include "KeyboardEvent.h"

struct lua_State;

namespace Gsage {
  class LuaInterface;
  class RenderEvent;
  class Engine;

  class RocketUIManager : public UIManager, public EventSubscriber<RocketUIManager>
  {
    public:
      RocketUIManager();
      virtual ~RocketUIManager();
      /**
       * Initialize ui manager
       *
       * @param called by gsage facade on setup
       * @param L init with lua state
       */
      virtual void initialize(Engine* engine, lua_State* L = 0);
      /**
       * Gets Rocket lua state
       */
      lua_State* getLuaState();

      /**
       * Update load state
       * @param L lua_State
       */
      void setLuaState(lua_State* L);
    private:
      /**
       * Update ui callback
       *
       * @param event RenderEvent
       */
      bool render(EventDispatcher* sender, const Event& event);
      /**
       * Configure projection matrix
       *
       * @param width Window width
       * @param height Window height
       */
      void configureRenderSystem(RenderEvent& event);
      /**
       * Set up system/render interfaces
       *
       * @param width Initial window width
       * @param height Initial window height
       */
      void setUp(unsigned int width, unsigned int height);
      /**
       * Handle mouse event from engine
       *
       * @param event Event
       */
      bool handleMouseEvent(EventDispatcher* sender, const Event& event);
      /**
       * Handle keyboard event from engine
       */
      bool handleKeyboardEvent(EventDispatcher* sender, const Event& event);
      /**
       * Get key modifier state
       */
      int getKeyModifierState();
      /**
       * Build OIS <-> Rocket key map
       */
      void buildKeyMap();
      /**
       * Check if mouse event can be captured by any rocket element
       */
      bool doCapture();

      Rocket::Core::Context* mContext;

      Rocket::Core::RenderInterface* mRenderInterface;
      Rocket::Core::SystemInterface* mSystemInterface;

      // TODO move to some other class
      typedef std::map< KeyboardEvent::Key, Rocket::Core::Input::KeyIdentifier > KeyIdentifierMap;
      KeyIdentifierMap mKeyMap;
      unsigned int mModifiersState;

  };
}
#endif
