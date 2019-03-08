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

#include "lua/LuaEventProxy.h"
#include "Logger.h"

namespace Gsage  {

  LuaEventProxy::LuaEventProxy()
  {

  }

  LuaEventProxy::~LuaEventProxy()
  {
    mCallbackBindings.clear();
  }

  bool LuaEventProxy::addEventListener(EventDispatcher* dispatcher, Event::ConstType eventType, const sol::object& callback)
  {
    if(callback.get_type() != sol::type::function)
      return false;

    Callbacks* cb = getCallbacks(dispatcher, eventType);
    Callbacks& callbacks = cb != 0 ? *cb : subscribe(dispatcher, eventType);
    callbacks.push_back(GenericCallbackPtr(new GenericCallback(callback.as<sol::function>())));
    return true;
  }

  bool LuaEventProxy::removeEventListener(EventDispatcher* dispatcher, Event::ConstType eventType, const sol::object& callback)
  {
    if(callback.get_type() != sol::type::function)
      return false;

    auto callbacks = getCallbacks(dispatcher, eventType);
    if(!callbacks)
      return false;

    sol::function listener = callback.as<sol::function>();
    Callbacks::iterator iter = std::find_if(callbacks->begin(), callbacks->end(), [listener](GenericCallbackPtr e){
      return e->func() == listener;
    });
    if(iter == callbacks->end())
      return false;

    int previousSize = callbacks->size();
    callbacks->erase(iter);
    if(callbacks->size() == 0)
    {
      EventSubscriber<LuaEventProxy>::removeEventListener(dispatcher, eventType, &LuaEventProxy::handleEvent);
      mCallbackBindings.erase(CallbackBinding(dispatcher, eventType));
      LOG(TRACE) << "Removed event listener for event " << eventType << " as there is no more lua callbacks";
    } else {
      LOG(TRACE) << "Removed event callback " << eventType;
      assert(getCallbacks(dispatcher, eventType)->size() != previousSize);
    }

    return true;
  }

  bool LuaEventProxy::handleEvent(EventDispatcher* sender, const Event& event)
  {
    Callbacks* callbacks = getCallbacks(sender, event.getType());
    if(callbacks != 0)
    {
      for(auto callback : (*callbacks))
      {
        if(!callback->valid())
        {
          removeEventListener(sender, event.getType(), callback->func());
          LOG(ERROR) << "Failed to call " << event.getType() << " listener invalid, removed from subscribers";
          continue;
        }
        std::string error;
        sol::function_result res;
        try {
          res = (*callback)(event);
        } catch (...) {
          LOG(WARNING) << "Failed to call " << event.getType() << " lua listener: unknown error";
          continue;
        }

        if(!res.valid()) {
          sol::error err = res;
          error = err.what();
        }

        if(res.get_type() == sol::type::boolean) {
          bool cont = res;
          if(!cont) {
            return false;
          }
        }

        if(!error.empty())
        {
          LOG(WARNING) << "Failed to call " << event.getType() << " lua listener: " << error;
        }
      }
    }

    return true;
  }

  bool LuaEventProxy::onForceUnsubscribe(EventDispatcher* sender, const Event& event)
  {
    EventSubscriber<LuaEventProxy>::onForceUnsubscribe(sender, event);
    CallbackBindings::iterator iter = mCallbackBindings.begin();
    while(iter != mCallbackBindings.end())
    {
      if((*iter).first.first == sender)
      {
        LOG(TRACE) << "Unbinding lua callbacks, event type: " << (*iter).first.second;
        CallbackBindings::iterator tmp = iter++;
        mCallbackBindings.erase(tmp);
      }
      else
      {
        iter++;
      }
    }
    return true;
  }

  LuaEventProxy::Callbacks* LuaEventProxy::getCallbacks(EventDispatcher* dispatcher, Event::ConstType eventType)
  {
    CallbackBinding binding(dispatcher, eventType);
    if(!contains(mCallbackBindings, binding))
      return 0;

    return &mCallbackBindings[binding];
  }

  LuaEventProxy::Callbacks& LuaEventProxy::subscribe(EventDispatcher* dispatcher, Event::ConstType eventType)
  {
    Callbacks& res = mCallbackBindings[CallbackBinding(dispatcher, eventType)];
    EventSubscriber<LuaEventProxy>::addEventListener(dispatcher, eventType, &LuaEventProxy::handleEvent);
    return res;
  }
}
