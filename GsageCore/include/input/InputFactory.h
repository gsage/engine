#ifndef _InputFactory_H_
#define _InputFactory_H_

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

#include "EventSubscriber.h"
#include "UpdateListener.h"

namespace Gsage {
  class Engine;
  /**
   * Input interface
   */
  class InputHandler : public EventSubscriber<InputHandler>, public UpdateListener
  {
    public:
      InputHandler(size_t windowHandle, Engine* engine);
      virtual ~InputHandler() {};
      /**
       * Handle window resize
       *
       * @param width Window width
       * @param height Window height
       */
      virtual void handleResize(unsigned int width, unsigned int height) = 0;

      /**
       * Handle window close
       */
      virtual void handleClose() = 0;
    protected:
      bool handleWindowEvent(EventDispatcher* sender, const Event& event);

      size_t mHandle;
      Engine* mEngine;
  };

  /**
   * Input handler wrapped into shared pointer
   */
  typedef std::shared_ptr<InputHandler> InputHandlerPtr;

  /**
   * Base class for any input factory
   */
  class AbstractInputFactory
  {
    public:
      AbstractInputFactory() {};
      virtual ~AbstractInputFactory() {};
      /**
       * Creates input handler
       *
       * @param windowHandle Window handle
       * @param engine Gsage::Engine pointer
       *
       * @returns pointer to handler. It should be wrapped into smart pointer
       */
      virtual InputHandler* create(size_t windowHandle, Engine* engine) = 0;
  };

  /**
   * Input manager creates input handler for each new window.
   * Type of input can be configured by passing any concrete factory to the constructor.
   * For example: OisInputFactory
   */
  class InputManager : public EventSubscriber<InputManager>, public UpdateListener
  {
    public:
      /**
       * @param engine Pointer to the engine
       */
      InputManager(Engine* engine);

      virtual ~InputManager();

      /**
       * Tells InputFactory to use any particular factory
       * @param id String id of factory
       */
      void useFactory(const std::string& id);

      /**
       * Register new type of input factory
       *
       * @param id String id of factory
       */
      template<class T>
      void registerFactory(const std::string& id)
      {
        if(!std::is_base_of<AbstractInputFactory, T>::value)
        {
          LOG(ERROR) << "Factory for id '" << id << "' was not registered. It must be derived from AbstractInputFactory";
          return;
        }
        mFactories[id] = new T();
      }

      /**
       * Remove factory by id. It will also remove all input listeners
       *
       * @param id String id of factory
       */
      void removeFactory(const std::string& id);

      /**
       * Update all handlers
       *
       * @param time Elapsed time
       */
      void update(const float& time);
    private:
      bool handleWindowEvent(EventDispatcher* sender, const Event& e);
      Engine* mEngine;

      typedef std::map<size_t, InputHandlerPtr> InputHandlers;
      InputHandlers mInputHandlers;

      typedef std::map<std::string, AbstractInputFactory*> InputFactories;
      InputFactories mFactories;

      std::string mCurrentFactoryId;
  };
}

#endif
