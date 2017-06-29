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

#include "EventSubscriber.h"
#include "sol.hpp"

namespace Gsage {
  /**
   * Class that allows to bind lua function to any event
   */
  class LuaEventProxy : public EventSubscriber<LuaEventProxy>
  {
    public:
      /**
       * sol::protected_function wrapper
       */
      class GenericCallback
      {
        public:
          GenericCallback(sol::protected_function func) : mFunc(func) {};
          virtual ~GenericCallback() {};
          /**
           * Checks if wrapped function is valid
           * @returns sol::protected_function valid result
           */
          bool valid() const{
            return mFunc.valid();
          }

          /**
           * Calls underlying lua callback
           *
           * @param event Event to process
           * @returnds sol::protected_function_result
           */
          virtual sol::protected_function_result operator()(const Event& event)
          {
            return mFunc(std::ref(event));
          }

          operator const sol::protected_function&() const {
            return mFunc;
          }

          friend bool operator==(GenericCallback* cb, const sol::protected_function& func) {
            return cb->mFunc == func;
          }

          sol::protected_function& func()
          {
            return mFunc;
          }

        protected:
          sol::protected_function mFunc;
      };

      /**
       * Templated version of callback. Casts event to any specific type before calling the function.
       */
      template<class T>
      class Callback : public GenericCallback
      {
        public:
          Callback(sol::protected_function func) : GenericCallback(func) {}
          virtual ~Callback() {}
          /**
           * Calls underlying lua callback
           *
           * @param event Event to process (will be cast to the particular templated event).
           * @returnds sol::protected_function_result
           */
          virtual sol::protected_function_result operator()(const Event& event)
          {
            return mFunc(std::ref(static_cast<const T&>(event)));
          }
      };

      typedef std::pair<EventDispatcher*, const std::string> CallbackBinding;
      typedef std::vector<GenericCallback*> Callbacks;
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
      bool addEventListener(EventDispatcher* dispatcher, const std::string& eventType, const sol::object& callback);

      /**
       * @copydoc LuaEventProxy::addEventListener
       */
      template<class T>
      bool addEventListener(EventDispatcher* dispatcher, const std::string& eventType, const sol::object& callback)
      {
        if(callback.get_type() != sol::type::function)
          return false;

        Callbacks* cb = getCallbacks(dispatcher, eventType);
        Callbacks& callbacks = cb != 0 ? *cb : subscribe(dispatcher, eventType);
        callbacks.push_back(new Callback<T>(callback.as<sol::protected_function>()));
        return true;
      }

      /**
       * Removes event listener
       *
       * @param dispatcher Object that dispatches the event
       * @param eventType Event id
       * @param callback Lua object that is called on event dispatch
       * @returns true if the callback was removed successfully
       */
      bool removeEventListener(EventDispatcher* dispatcher, const std::string& eventType, const sol::object& callback);

      /**
       * Get callbacks for binding
       * @param dispatcher Object that dispatches the event
       * @param eventType Event id
       */
      Callbacks* getCallbacks(EventDispatcher* dispatcher, const std::string& eventType);

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
