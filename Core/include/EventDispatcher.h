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

#ifndef _EventDispatcher_H_
#define _EventDispatcher_H_

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/signals2.hpp>
#include <algorithm>
#include <map>
#include <memory>
#include <vector>

namespace Gsage {
  /**
   * Class requred to enhance boost::sinals so it can support event propagation cancelling
   */
  struct InterruptableCombiner {
    typedef bool result_type;
    template <typename InputIterator> result_type operator()(InputIterator aFirstObserver, InputIterator aLastObserver) const {
      result_type val = true;
      for (; aFirstObserver != aLastObserver && val; ++aFirstObserver)  {
        val = *aFirstObserver;
      }
      return val;
    }
  };

  class EventDispatcher;

  /**
   * Abstract event class
   */
  class Event
  {
    public:
      Event(const std::string& type) : mType(type) {};
      virtual ~Event() {};

      /**
       * Get event type
       */
      const std::string getType() const { return mType; };
    private:
      std::string mType;

  };

  class DispatcherEvent : public Event
  {
    public:
      static const std::string FORCE_UNSUBSCRIBE;

      DispatcherEvent(const std::string& type) : Event(type) {}
      virtual ~DispatcherEvent() {};
  };

  class EventDispatcher
  {
    public:

      EventDispatcher();
      virtual ~EventDispatcher();

      /**
       * Boost signals connection
       */
      typedef boost::signals2::connection EventConnection;
      /**
       * Boost function that represents event callback
       */
      typedef boost::function<bool (EventDispatcher*, const Event&)> EventCallback;
      /**
       * Boost signal
       */
      typedef boost::signals2::signal<bool (EventDispatcher*, const Event&), InterruptableCombiner> EventSignal;
      /**
       * All event bindings
       */
      typedef std::map<std::string, EventSignal*> EventTypes;

      /**
       * Dispatch event to all subscribers of the type, defined in the event.
       *
       * @param event Abstract event
       */
      void fireEvent(const Event& event);
    private:
      template<class C>
      friend class EventSubscriber;
      /**
       * Check if dispatcher has listeners for event type
       *
       * @param type Event type
       */
      bool hasListenersForType(const std::string& type);
      /**
       * Adds event subscriber to the event. Should be called by event subscriber class
       *
       * @param eventType Type of the event
       * @param callback Function binding
       * @param priority Priority of attached event callback
       */
      EventConnection addEventListener(const std::string& eventType, EventCallback callback, const int priority = 0);
      EventTypes mSignals;
  };
}

#endif
