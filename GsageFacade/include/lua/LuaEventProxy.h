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

#ifndef _LuaEventProxy_H_
#define _LuaEventProxy_H_

#include <luabind/luabind.hpp>
#include "EventSubscriber.h"

namespace Gsage {
  /**
   * Class that allows to bind lua function to any event
   */
  class LuaEventProxy : public EventSubscriber<LuaEventProxy>
  {
    public:

      typedef std::pair<EventDispatcher*, const std::string> CallbackBinding;
      typedef std::vector<luabind::object> Callbacks;
      typedef std::map<CallbackBinding, Callbacks> CallbackBindings;

      LuaEventProxy();
      virtual ~LuaEventProxy();
      /**
       * Adds event listener
       *
       * @param dispatcher Object that dispatches the event
       * @param eventType Event id
       * @param callback Lua object that is called on event dispatch
       * @returns true if the callback was added successfully
       */
      bool addEventListener(EventDispatcher* dispatcher, const std::string& eventType, const luabind::object& callback);

      /**
       * Removes event listener
       *
       * @param dispatcher Object that dispatches the event
       * @param eventType Event id
       * @param callback Lua object that is called on event dispatch
       * @returns true if the callback was removed successfully
       */
      bool removeEventListener(EventDispatcher* dispatcher, const std::string& eventType, const luabind::object& callback);

      /**
       * Get callbacks for binding
       * @param dispatcher Object that dispatches the event
       * @param eventType Event id
       */
      boost::optional<Callbacks&> getCallbacks(EventDispatcher* dispatcher, const std::string& eventType);

      /**
       * Overriding standard callback which is called on dispatcher deletion
       * @param sender EventDispatcher
       * @param event DispatcherEvent
       */
      bool onForceUnsubscribe(EventDispatcher* sender, const Event& event);
    private:
      /**
       * Listens to all events and proxies to lua objects
       *
       * @param event Any event
       */
      bool handleEvent(EventDispatcher* sender, const Event& event);
      /**
       * Add c++ event listener to the event
       *
       * @param dispatcher Event dispatcher
       * @param eventType Event id
       * @returns list of callbacks
       */
      Callbacks& subscribe(EventDispatcher* dispatcher, const std::string& eventType);

      CallbackBindings mCallbackBindings;
  };
}

#endif
