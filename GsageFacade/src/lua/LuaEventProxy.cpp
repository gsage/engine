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

  }

  bool LuaEventProxy::addEventListener(EventDispatcher* dispatcher, const std::string& eventType, const luabind::object& callback)
  {
    if(luabind::type(callback) != LUA_TFUNCTION)
      return false;

    boost::optional<Callbacks&> callbacksOptional = getCallbacks(dispatcher, eventType);
    Callbacks& callbacks = callbacksOptional ? callbacksOptional.get() : subscribe(dispatcher, eventType);
    callbacks.push_back(callback);
    return true;
  }

  bool LuaEventProxy::removeEventListener(EventDispatcher* dispatcher, const std::string& eventType, const luabind::object& callback)
  {
    auto callbacks = getCallbacks(dispatcher, eventType);
    if(!callbacks)
      return false;

    Callbacks::iterator iter = std::find(callbacks.get().begin(), callbacks.get().end(), callback);
    if(iter == callbacks.get().end())
      return false;

    callbacks.get().erase(iter);
    if(callbacks.get().size() == 0)
    {
      EventSubscriber<LuaEventProxy>::removeEventListener(dispatcher, eventType, &LuaEventProxy::handleEvent);
      mCallbackBindings.erase(CallbackBinding(dispatcher, eventType));
      LOG(INFO) << "Removed event listener for event " << eventType << " as there is no more lua callbacks";
    }

    return true;
  }

  bool LuaEventProxy::handleEvent(EventDispatcher* sender, const Event& event)
  {
    auto callbacks = getCallbacks(sender, event.getType());
    if(callbacks)
    {
      for(luabind::object& callback : callbacks.get())
      {
        if(callback.is_valid())
        {
          std::string error;
          try
          {
            callback(boost::ref(event));
          }
          catch(std::runtime_error e)
          {
            error = e.what();
          }
          catch(luabind::error e)
          {
            error = e.what();
          }

          if(!error.empty())
          {
            removeEventListener(sender, event.getType(), callback);
            LOG(ERROR) << "Failed to call " << event.getType() << " listener: " << error  << ", removed from subscribers";
          }

        }
      }
    }

    return false;
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

  boost::optional<LuaEventProxy::Callbacks&> LuaEventProxy::getCallbacks(EventDispatcher* dispatcher, const std::string& eventType)
  {
    CallbackBinding binding(dispatcher, eventType);
    if(mCallbackBindings.count(binding) == 0)
      return boost::optional<Callbacks&>(boost::none);

    return boost::optional<Callbacks&>(mCallbackBindings[binding]);
  }

  LuaEventProxy::Callbacks& LuaEventProxy::subscribe(EventDispatcher* dispatcher, const std::string& eventType)
  {
    Callbacks& res = mCallbackBindings[CallbackBinding(dispatcher, eventType)];
    EventSubscriber<LuaEventProxy>::addEventListener(dispatcher, eventType, &LuaEventProxy::handleEvent);
    return res;
  }
}
