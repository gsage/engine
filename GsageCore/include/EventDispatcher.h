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

#include <algorithm>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace Gsage {
  class EventDispatcher;
  class EventSignal;
  class Event;

  /**
   * std function that represents event callback
   */
  typedef std::function<bool (EventDispatcher*, const Event&)> EventCallback;

  /**
   * Identifies single connection to the EventSignal
   * - signal pointer
   * - priority int
   * - id int
   */
  class EventConnection
  {
    public:
      EventConnection(EventSignal* signal, int priority, int id);
      virtual ~EventConnection();
      /**
       * Disconnect underlying connection
       */
      void disconnect();
    private:
      EventSignal* mSignal;
      int mPriority;
      int mId;
  };

  /**
   * Lightweight version of boost::signal2
   */
  class EventSignal
  {
    public:
      EventSignal();
      virtual ~EventSignal();
      /**
       * Connect to signal
       * @param priority 0 means the biggest priority
       * @param callback function to call
       */
      EventConnection connect(const int priority, EventCallback callback);

      /**
       * Call signall
       * @param dispatcher pointer to dispatcher which calls this signal
       * @param event to pass
       */
      void operator()(EventDispatcher* dispatcher, const Event& event);

      /**
       *Disconnect one callback identified by priority and id
       */
      void disconnect(int priority, int id);
    private:
      typedef std::map<int, EventCallback> CallbacksList;

      typedef std::map<int, CallbacksList> Connections;

      Connections mConnections;
  };

  /**
   * Abstract event class
   */
  class Event
  {
    public:
      typedef std::string Type;
      typedef const std::string& ConstType;

      Event(ConstType type) : mType(type) {};
      virtual ~Event() {};

      /**
       * Get event type
       */
      ConstType getType() const { return mType; };
    private:
      Type mType;

  };

  class DispatcherEvent : public Event
  {
    public:
      static const Event::Type FORCE_UNSUBSCRIBE;

      DispatcherEvent(ConstType type) : Event(type) {}
      virtual ~DispatcherEvent() {};
  };

  class EventDispatcher
  {
    public:

      EventDispatcher();
      virtual ~EventDispatcher();
      /**
       * All event bindings
       */
      typedef std::map<Event::Type, EventSignal*> EventTypes;

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
      bool hasListenersForType(Event::ConstType type);
      /**
       * Adds event subscriber to the event. Should be called by event subscriber class
       *
       * @param eventType Type of the event
       * @param callback Function binding
       * @param priority Priority of attached event callback
       */
      EventConnection addEventListener(Event::ConstType eventType, EventCallback callback, const int priority = 0);
      EventTypes mSignals;
  };
}

#endif
