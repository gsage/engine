#ifndef _LuaEventConnection_H_
#define _LuaEventConnection_H_

/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Gsage Authors

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

#include "sol.hpp"
#include "EventSubscriber.h"

namespace Gsage {
  /**
   * LuaEventConnection can be used to connect lua functions to C++ events
   */
  class LuaEventConnection : public EventSubscriber<LuaEventConnection>
  {
    public:
      LuaEventConnection(sol::protected_function handler);
      virtual ~LuaEventConnection();

      /**
       * bind lua function
       * @param target EventDispatcher to listen
       * @param type event type
       * @return binding id
       */
      long bind(EventDispatcher* target, Event::ConstType type);

      /**
       * bind lua function
       * @param target EventDispatcher to listen
       * @param type event type
       * @return binding id
       */
      template<typename C>
      long bind(EventDispatcher* target, Event::ConstType type)
      {
        if(!hasEventListener(target, type)) {
          addEventListener(target, type, &LuaEventConnection::handle, -100);
        }
        EventSubscriber::EventSubscription binding(target, type);
        mBindings[mNextID] = binding;

        if(mRouter.count(binding) == 0) {
          mRouter[binding] = Listeners();
        }

        mRouter[binding][mNextID] = [this](EventDispatcher* target, const Event& event, long id) -> bool {
          auto res = mHandler(id, std::ref(static_cast<const C&>(event)), target);
          std::string error;
          if(!res.valid()) {
            sol::error err = res;
            error = err.what();
          }

          if(!error.empty())
          {
            LOG(WARNING) << "Failed to call " << event.getType() << " lua listener: " << error;
          }

          sol::optional<bool> invalidate = res;
          if(invalidate) {
            return invalidate.value();
          }

          return false;
        };
        return mNextID++;
      }

      /**
       * Unbind lua function
       *
       * @param id binding id
       * @returns false if failed to unbind
       */
      bool unbind(long id);

      /**
       * Handle event and proxy it to lua
       *
       * @param sender EventDispatcher that sends events
       * @param e event
       * @return true
       */
      bool handle(EventDispatcher* sender, const Event& e);
      bool onForceUnsubscribe(EventDispatcher* sender, const Event& event);
    private:
      typedef std::map<long, EventSubscriber::EventSubscription> Bindings;
      Bindings mBindings;

      typedef std::function<bool(EventDispatcher*, const Event&, long)> Handler;

      typedef std::map<long, Handler> Listeners;
      typedef std::map<EventSubscriber::EventSubscription, Listeners> Router;
      Router mRouter;

      sol::protected_function mHandler;
      long mNextID;
  };
}

#endif
