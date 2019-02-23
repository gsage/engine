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

  const Event::Type EngineEvent::LUA_STATE_CHANGE = "EngineEvent::LUA_STATE_CHANGE";

  const Event::Type EngineEvent::SHUTDOWN = "EngineEvent::SHUTDOWN";

  const Event::Type EngineEvent::STOPPING = "EngineEvent::STOPPING";

  const Event::Type EngineEvent::UPDATE = "EngineEvent::UPDATE";

  const Event::Type EngineEvent::ENV_UPDATED = "EngineEvent::ENV_UPDATED";

  const Event::Type DropFileEvent::DROP_FILE = "DropFileEvent::DROP_FILE";

  const Event::Type DropFileEvent::DROP_BEGIN = "DropFileEvent::DROP_BEGIN";

  const Event::Type EntityEvent::REMOVE = "EntityEvent::REMOVE";

  const Event::Type EntityEvent::CREATE = "EntityEvent::CREATE";

  const Event::Type SettingsEvent::UPDATE = "SettingsEvent::UPDATE";

  const Event::Type SystemChangeEvent::SYSTEM_ADDED = "SystemChangeEvent::SYSTEM_ADDED";

  const Event::Type SystemChangeEvent::SYSTEM_REMOVED = "SystemChangeEvent::SYSTEM_REMOVED";

  const Event::Type SystemChangeEvent::SYSTEM_STARTED = "SystemChangeEvent::SYSTEM_STARTED";

  const Event::Type SystemChangeEvent::SYSTEM_STOPPING = "SystemChangeEvent::SYSTEM_STOPPING";

  const Event::Type SelectEvent::OBJECT_SELECTED = "SystemChangeEvent::OBJECT_SELECTED";

  const Event::Type SelectEvent::ROLL_OVER = "SelectEvent::ROLL_OVER";

  const Event::Type SelectEvent::ROLL_OUT = "SelectEvent::ROLL_OUT";

  const Event::Type WindowEvent::CREATE = "WindowEvent::CREATE";

  const Event::Type WindowEvent::RESIZE = "WindowEvent::RESIZE";

  const Event::Type WindowEvent::CLOSE = "WindowEvent::CLOSE";

  const Event::Type WindowEvent::MOVE = "WindowEvent::MOVE";

  EngineEvent::EngineEvent(Event::ConstType type) : Event(type)
  {
  }

  EngineEvent::~EngineEvent()
  {
  }

  DropFileEvent::DropFileEvent(Event::ConstType type, const std::string& pfile) 
    : Event(type)
    , file(pfile)
  {
  }

  DropFileEvent::~DropFileEvent()
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

  WindowEvent::WindowEvent(Event::ConstType type, size_t pHandle, unsigned int pWidth, unsigned int pHeight, int pX, int pY)
    : Event(type)
    , handle(pHandle)
    , width(pWidth)
    , height(pHeight)
    , x(pX)
    , y(pY)
  {
  }

  WindowEvent::~WindowEvent()
  {
  }
}
