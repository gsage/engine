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

#include "EngineEvent.h"
#include "EngineSystem.h"

namespace Gsage {

  const Event::Type EngineEvent::LUA_STATE_CHANGE = "luaStateChange";

  const Event::Type EngineEvent::SHUTDOWN = "shutdown";

  const Event::Type EngineEvent::STOPPING = "stopping";

  const Event::Type EntityEvent::CREATE = "entityCreated";

  const Event::Type EntityEvent::DELETE = "entityDeleted";

  const Event::Type SettingsEvent::UPDATE = "update";

  const Event::Type SystemChangeEvent::SYSTEM_ADDED = "systemAdded";

  const Event::Type SystemChangeEvent::SYSTEM_REMOVED = "systemRemoved";

  const Event::Type SelectEvent::OBJECT_SELECTED = "objectSelected";

  const Event::Type SelectEvent::ROLL_OVER = "rollOver";

  const Event::Type SelectEvent::ROLL_OUT = "rollOut";

  const Event::Type WindowEvent::CREATE = "create";

  const Event::Type WindowEvent::RESIZE = "resize";

  const Event::Type WindowEvent::CLOSE = "close";

  EngineEvent::EngineEvent(Event::ConstType type) : Event(type)
  {
  }

  EngineEvent::~EngineEvent()
  {
  }

  EntityEvent::EntityEvent(Event::ConstType type, const std::string& entityId)
    : Event(type)
    , mEntityId(entityId)
  {
  }

  EntityEvent::~EntityEvent()
  {
  }

  SettingsEvent::SettingsEvent(Event::ConstType type, const DataProxy& settings)
    : Event(type)
    , settings(settings)
  {
  }

  SettingsEvent::~SettingsEvent()
  {
  }

  SystemChangeEvent::SystemChangeEvent(Event::ConstType type, const std::string& systemId, EngineSystem* system)
    : Event(type)
    , mSystemId(systemId)
    , mSystem(system)
  {
  }

  SystemChangeEvent::~SystemChangeEvent()
  {
  }

  WindowEvent::WindowEvent(Event::ConstType type, size_t pHandle, unsigned int pWidth, unsigned int pHeight)
    : Event(type)
    , handle(pHandle)
    , width(pWidth)
    , height(pHeight)
  {
  }

  WindowEvent::~WindowEvent()
  {
  }
}
