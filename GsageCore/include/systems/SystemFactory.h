#ifndef _SystemFactory_H_
#define _SystemFactory_H_

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

#include <exception>
#include <string>
#include "Logger.h"
#include "Engine.h"
#include "GsageDefinitions.h"

#if GSAGE_PLATFORM == GSAGE_LINUX
#define _NOEXCEPT _GLIBCXX_USE_NOEXCEPT
#endif

namespace Gsage {
  class Engine;
  class EngineSystem;

  class SystemFactoryException : public std::exception
  {
    public:
      SystemFactoryException(const char* error)
        : mErrMessage(error)
      {}

      virtual const char* what() const _NOEXCEPT{
        return mErrMessage;
      }
    private:
      const char * mErrMessage;
  };

  /**
   * De-templated base class for the system factory
   */
  class SystemFactory
  {
    public:
      virtual ~SystemFactory() {}
      /**
       * Creates the system in the engine
       *
       * @param engine Engine instance
       * @returns pointer to created EngineSystem, 0 if failed
       */
      virtual EngineSystem* create(Engine* engine) = 0;
  };

  /**
   * Creates new system factory
   */
  template<class S>
  class ConcreteSystemFactory : public SystemFactory
  {
    public:
      typedef S systemType;
      ConcreteSystemFactory()
      {
        if(!std::is_base_of<EngineSystem, S>::value) {
          LOG(ERROR) << "Failed to create factory: specified type is not derived from the EngineSystem";
          throw new SystemFactoryException("Template type is not derived from EngineSystem");
        }
      }
      /**
       * Creates the system of type S in the engine
       *
       * @param engine Engine instance
       * @returns pointer to created EngineSystem, 0 if failed
       */
      EngineSystem* create(Engine* engine)
      {
        return engine->addSystem<S>();
      }
  };
}

#endif
