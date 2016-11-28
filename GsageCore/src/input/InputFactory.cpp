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

#include "input/InputFactory.h"
#include "Engine.h"
#include "EngineEvent.h"

namespace Gsage {
  InputHandler::InputHandler(size_t windowHandle, Engine* engine)
    : mHandle(windowHandle)
    , mEngine(engine)
  {
    addEventListener(engine, WindowEvent::RESIZE, &InputHandler::handleWindowEvent);
  }

  bool InputHandler::handleWindowEvent(EventDispatcher* sender, const Event& e)
  {
    const WindowEvent& event = static_cast<const WindowEvent&>(e);
    if(event.handle != mHandle)
    {
      return true;
    }

    handleResize(event.width, event.height);
    return true;
  }

  InputManager::InputManager(Engine* engine)
    : mEngine(engine)
  {
    addEventListener(engine, WindowEvent::CREATE, &InputManager::handleWindowEvent);
    addEventListener(engine, WindowEvent::CLOSE, &InputManager::handleWindowEvent, 10);
  }

  InputManager::~InputManager()
  {
    mInputHandlers.clear();
    for(auto pair : mFactories) {
      delete pair.second;
    }

    mFactories.clear();
  }

  void InputManager::useFactory(const std::string& id)
  {
    mCurrentFactoryId = id;
  }

  void InputManager::update(const float& time)
  {
    for(auto pair : mInputHandlers) {
      pair.second->update(time);
    }
  }

  bool InputManager::handleWindowEvent(EventDispatcher* sender, const Event& e)
  {
    const WindowEvent& event = static_cast<const WindowEvent&>(e);
    if(event.getType() == WindowEvent::CREATE && !mCurrentFactoryId.empty()) {
      if(mFactories.count(mCurrentFactoryId) == 0) {
        LOG(ERROR) << "Failed to init input of type '" << mCurrentFactoryId << "': no such factory";
        return true;
      }

      if(mInputHandlers.count(event.handle) != 0) {
        return true;
      }

      // Create new handler
      mInputHandlers[event.handle] = InputHandlerPtr(mFactories[mCurrentFactoryId]->create(event.handle, mEngine));
      mInputHandlers[event.handle]->handleResize(event.width, event.height);
    } else if(event.getType() == WindowEvent::CLOSE) {
      if(mInputHandlers.count(event.handle) == 0) {
        return true;
      }

      // Pass handle event to handler and then erase this handler
      mInputHandlers[event.handle]->handleClose();
      mInputHandlers.erase(event.handle);
    }
    return true;
  }

}
