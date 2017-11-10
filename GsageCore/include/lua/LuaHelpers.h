#ifndef _LUA_HELPERS_H_
#define _LUA_HELPERS_H_
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

#include "sol.hpp"
#include "lua/LuaEventProxy.h"
#include "lua/LuaEventConnection.h"

namespace Gsage {
  template<class T>
  void LuaInterface::registerEvent(Event::ConstType name, const std::string& handler, sol::usertype<T> ut)
  {
    if(!std::is_base_of<Event, T>::value)
    {
      LOG(ERROR) << "Event not registered: specified type is not derived from Event";
      return;
    }

    if(!mStateView)
    {
      LOG(ERROR) << "Event not registered: no lua state";
      return;
    }

    sol::state_view& lua = *mStateView;

    lua.set_usertype(name, ut);
    lua["LuaEventProxy"][handler] = &LuaEventProxy::addEventListener<T>;
  }

  typedef sol::var_wrapper<sol::meta::unqualified_t<std::string>> StringVar;

  void expand(std::function<void(const char*)> setter)
  {
    // nothing
  }

  template<typename... Args>
  void expand(std::function<void(const char*)> setter, const char* key, StringVar& value, Args&... args)
  {
    setter(key);
    expand(setter, args...);
  }

  template<typename K, typename V, typename... Args>
  void expand(std::function<void(const char*)> setter, K key, V& value, Args&... args)
  {
    expand(setter, args...);
  }

  template<typename C, typename... Args>
  void LuaInterface::registerEvent(Event::ConstType name, const std::string& handler, Args&&... args)
  {
    if(!std::is_base_of<Event, C>::value)
    {
      LOG(ERROR) << "Event not registered: specified type is not derived from Event";
      return;
    }

    if(!mStateView)
    {
      LOG(ERROR) << "Event not registered: no lua state";
      return;
    }

    sol::state_view& lua = *mStateView;
    lua.new_usertype<C>(name, std::forward<Args>(args)...);
    lua["LuaEventConnection"][handler] = &LuaEventConnection::bind<C>;
    lua["LuaEventProxy"][handler] = &LuaEventProxy::addEventListener<C>;
  }
}
#endif
