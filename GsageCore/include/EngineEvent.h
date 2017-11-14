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

#ifndef _EngineEvent_H_
#define _EngineEvent_H_

#include "EventDispatcher.h"
#include "DataProxy.h"

namespace Gsage {
  class EngineSystem;
  /**
   * Engine related event
   */
  class EngineEvent : public Event
  {
    public:
      /**
       * New lua state was created, can be used to reinitialize all bindings
       */
      static const Event::Type LUA_STATE_CHANGE;
      /**
       * Engine issued stopping
       */
      static const Event::Type STOPPING;
      /**
       * Engine was shut down
       */
      static const Event::Type SHUTDOWN;
      EngineEvent(Event::ConstType type);
      virtual ~EngineEvent();
  };

  /**
   * Entity changes event
   */
  class EntityEvent : public Event
  {
    public:
      /**
       * Entity added event
       */
      static const Event::Type CREATE;

      /**
       * Entity removed event
       */
      static const Event::Type DELETE;

      EntityEvent(Event::ConstType type, const std::string& entityId);

      virtual ~EntityEvent();

      std::string mEntityId;
  };

  /**
   * Triggered when settings are changed
   */
  class SettingsEvent : public Event
  {
    public:
      /**
       * Settings updated
       */
      static const Event::Type UPDATE;

      SettingsEvent(Event::ConstType type, const DataProxy& settings);
      virtual ~SettingsEvent();
      DataProxy settings;
  };

  /**
   * Event which is triggered when system added or removed in the engine
   */
  class SystemChangeEvent : public Event
  {
    public:
      /**
       * System was added
       */
      static const Event::Type SYSTEM_ADDED;
      /**
       * System was removed
       */
      static const Event::Type SYSTEM_REMOVED;

      SystemChangeEvent(Event::ConstType type, const std::string& systemId, EngineSystem* system = 0);
      virtual ~SystemChangeEvent();

      const std::string mSystemId;
      EngineSystem* mSystem;
  };

  class WindowEvent : public Event
  {
    public:
      /**
       * Window was created
       */
      static const Event::Type CREATE;
      /**
       * Window resized
       */
      static const Event::Type RESIZE;
      /**
       * Window closed
       */
      static const Event::Type CLOSE;

      WindowEvent(Event::ConstType type, size_t handle, unsigned int width = 0, unsigned int height = 0);
      virtual ~WindowEvent();

      unsigned int width;
      unsigned int height;
      size_t handle;
  };

  /**
   * Some entity was selected
   */
  class SelectEvent : public Event
  {
    public:
      /**
       * Object selected
       */
      static const Event::Type OBJECT_SELECTED;
      /**
       * Object is rolled over
       */
      static const Event::Type ROLL_OVER;
      /**
       * Object is rolled out
       */
      static const Event::Type ROLL_OUT;
      SelectEvent(Event::ConstType type, const unsigned int& flags, const std::string& entityId)
        : mFlags(flags)
        , mEntityId(entityId)
        , Event(type)
      {
      }

      virtual ~SelectEvent() {};

      /**
       * Check entity flags
       * @param flags Flags mask
       */
      bool hasFlags(const unsigned int& flags) const
      {
        return (mFlags & flags) == flags;
      }

      /**
       * Get selected entity id
       */
      const std::string& getEntityId() const { return mEntityId; }
    private:
      unsigned int mFlags;
      std::string mEntityId;
  };
}

#endif
