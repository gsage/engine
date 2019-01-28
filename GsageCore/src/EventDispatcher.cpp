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

#include "EventDispatcher.h"
#include "Logger.h"

namespace Gsage {
  const Event::Type DispatcherEvent::FORCE_UNSUBSCRIBE = "forceUnsubscribe";

  EventDispatcher::EventDispatcher()
  {
  }

  EventDispatcher::~EventDispatcher()
  {
    removeAllListeners();
  }

  EventConnection EventDispatcher::addEventListener(Event::ConstType eventType, EventCallback callback, const int priority)
  {
    if(!hasListenersForType(eventType)) {
      mMutationMutex.lock();
      mSignals[eventType] = new EventSignal();
      mMutationMutex.unlock();
    }

    mMutationMutex.lock();
    EventConnection c = mSignals[eventType]->connect(priority, callback);
    mMutationMutex.unlock();
    return c;
  }

  bool EventDispatcher::hasListenersForType(Event::ConstType type)
  {
    mMutationMutex.lock();
    bool res = mSignals.count(type) > 0;
    mMutationMutex.unlock();
    return res;
  }

  int EventDispatcher::fireEvent(const Event& event)
  {
    if(!hasListenersForType(event.getType())) {
      return 0;
    }

    EventSignal* s = nullptr;
    mMutationMutex.lock();
    s = mSignals[event.getType()];
    mMutationMutex.unlock();
    return (*s)(this, event);
  }

  void EventDispatcher::removeAllListeners()
  {
    mMutationMutex.lock();
    for(std::pair<Event::Type, EventSignal*> element : mSignals)
    {
      if (element.first == DispatcherEvent::FORCE_UNSUBSCRIBE)
      {
        (*element.second)(this, DispatcherEvent(DispatcherEvent::FORCE_UNSUBSCRIBE));
        delete element.second;
      }
    }
    mSignals.clear();
    mMutationMutex.unlock();
  }

  EventSignal::EventSignal()
  {

  }

  EventSignal::~EventSignal()
  {
  }

  EventConnection EventSignal::connect(const int priority, EventCallback callback)
  {
    if(mConnections.count(priority) == 0)
    {
      mConnections[priority] = CallbacksList();
    }

    int id = mConnections[priority].empty() ? 0 : mConnections[priority].size();
    mConnections[priority].insert(std::make_pair(id, callback));
    return EventConnection(this, priority, id);
  }

  int EventSignal::operator()(EventDispatcher* dispatcher, const Event& event)
  {
    int handleCount = 0;
    for(auto pair : mConnections)
    {
      for(auto p : pair.second)
      {
        handleCount++;
        if(!p.second(dispatcher, event))
        {
          return handleCount;
        }
      }
    }

    return handleCount;
  }

  void EventSignal::disconnect(int priority, int id)
  {
    if(mConnections.count(priority) == 0)
      return;

    mConnections[priority].erase(id);
  }

  EventConnection::EventConnection(EventSignal* signal, int priority, int id)
    : mSignal(signal)
    , mPriority(priority)
    , mId(id)
  {
  }

  EventConnection::~EventConnection()
  {
  }

  void EventConnection::disconnect()
  {
    mSignal->disconnect(mPriority, mId);
  }
}
