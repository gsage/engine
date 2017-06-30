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

  const std::string EngineEvent::LUA_STATE_CHANGE = "luaStateChange";

  const std::string EngineEvent::HALT = "halt";

  const std::string SettingsEvent::UPDATE = "update";

  const std::string SystemChangeEvent::SYSTEM_ADDED = "systemAdded";

  const std::string SystemChangeEvent::SYSTEM_REMOVED = "systemRemoved";

  const std::string SelectEvent::OBJECT_SELECTED = "objectSelected";

  const std::string SelectEvent::ROLL_OVER = "rollOver";

  const std::string SelectEvent::ROLL_OUT = "rollOut";

  const std::string WindowEvent::CREATE = "create";

  const std::string WindowEvent::RESIZE = "resize";

  const std::string WindowEvent::CLOSE = "close";

  EngineEvent::EngineEvent(const std::string& type) : Event(type)
  {
  }

  EngineEvent::~EngineEvent()
  {
  }

  SettingsEvent::SettingsEvent(const std::string& type, const DataProxy& settings)
    : Event(type)
    , settings(settings)
  {
  }

  SettingsEvent::~SettingsEvent()
  {
  }

  SystemChangeEvent::SystemChangeEvent(const std::string& type, const std::string& systemId, EngineSystem* system)
    : Event(type)
    , mSystemId(systemId)
    , mSystem(system)
  {
  }

  SystemChangeEvent::~SystemChangeEvent()
  {
  }

  WindowEvent::WindowEvent(const std::string& type, size_t pHandle, unsigned int pWidth, unsigned int pHeight)
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
