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

#include "UIManager.h"
#include "EventSubscriber.h"
#include "KeyboardEvent.h"

struct lua_State;

namespace Gsage {
  class LuaInterface;
  class RenderEvent;
  class Engine;

  class RenderSystemWrapper {
    public:
      typedef std::map<std::string, Rocket::Core::Context*> Contexts;

      RenderSystemWrapper(Engine* engine) : mInitialized(false), mEngine(engine) {}

      virtual ~RenderSystemWrapper();

      /**
       * Get context
       *
       * @param name Context name
       *
       * @returns rocket context
       */
      virtual Rocket::Core::Context* getContext(const std::string& name);

      /**
       * Get all contexts
       */
      virtual Contexts getContexts();

      /**
       * Set lua state
       */
      virtual void setLuaState(lua_State* L);

      /**
       * Destroy render system wrapper
       */
      virtual void destroy();
    protected:
      /**
       * Create context
       */
      virtual Rocket::Core::Context* createContext(const std::string& name, unsigned int width, unsigned int height);

      /**
       * Set up interfaces
       */
      virtual void setUpInterfaces(unsigned int width, unsigned int height) = 0;

      Contexts mContexts;
      Engine* mEngine;
      lua_State* mLuaState;
      bool mInitialized;
  };

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

      /**
       * Configures rendering
       */
      void setUp();

      /**
       * SystemChangeEvent::SYSTEM_ADDED and SystemChangeEvent::SYSTEM_REMOVED handler
       */
      bool handleSystemChange(EventDispatcher* sender, const Event& event);
    private:
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
       * Handle keyboard event from engine
       */
      bool handleInputEvent(EventDispatcher* sender, const Event& event);
      /**
       * Get key modifier state
       */
      int getKeyModifierState();
      /**
       * Build Engine <-> Rocket key map
       */
      void buildKeyMap();
      /**
       * Check if mouse event can be captured by any rocket element
       */
      bool doCapture(Rocket::Core::Context* ctx);

      RenderSystemWrapper* mRenderSystemWrapper;

      // TODO move to some other class
      typedef std::map< KeyboardEvent::Key, Rocket::Core::Input::KeyIdentifier > KeyIdentifierMap;
      KeyIdentifierMap mKeyMap;
      unsigned int mModifiersState;

      bool mIsSetUp;

  };
}
#endif
