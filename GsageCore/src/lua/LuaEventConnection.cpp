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

#include "sol.hpp"
#include "lua/LuaEventConnection.h"

namespace Gsage {

  LuaEventConnection::LuaEventConnection(sol::protected_function handler)
    : mHandler(handler)
    , mNextID(0)
  {
  }

  LuaEventConnection::~LuaEventConnection()
  {
  }

  long LuaEventConnection::bind(EventDispatcher* target, Event::ConstType type)
  {
    return bind<Event>(target, type);
  }

  bool LuaEventConnection::unbind(long id)
  {
    if(mBindings.count(id) == 0) {
      return false;
    }

    EventSubscriber::EventSubscription& b = mBindings[id];
    // stop listening for the event
    mRouter[b].erase(id);
    bool res = true;
    if(mRouter[b].empty()) {
      res = removeEventListener(b.first, b.second, &LuaEventConnection::handle);
    }
    mBindings.erase(id);
    return res;
  }

  bool LuaEventConnection::handle(EventDispatcher* target, const Event& event)
  {
    if(!mHandler.valid()) {
      LOG(ERROR) << "lua event connection has invalid lua handler";
      return true;
    }

    EventSubscriber::EventSubscription b(target, event.getType());
    if(mRouter.count(b) == 0) {
      removeEventListener(target, event.getType(), &LuaEventConnection::handle);
      return true;
    }

    Listeners& l = mRouter[b];

    for(auto pair : l) {
      bool doUnbind = pair.second(target, event, pair.first);
      if(doUnbind) {
        unbind(pair.first);
      }
    }
    return true;
  }

  bool LuaEventConnection::onForceUnsubscribe(EventDispatcher* sender, const Event& event)
  {
    EventSubscriber<LuaEventConnection>::onForceUnsubscribe(sender, event);
    EventSubscriber::EventSubscription b(sender, event.getType());
    if(mRouter.count(b) == 0) {
      return true;
    }

    for(auto pair : mRouter[b]) {
      mBindings.erase(pair.first);
    }

    mRouter.erase(b);
    return true;
  }
}
