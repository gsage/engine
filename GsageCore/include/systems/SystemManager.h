#ifndef _SystemManager_H_
#define _SystemManager_H_

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

#include "systems/SystemFactory.h"

namespace Gsage {
  class Engine;
  /**
   * System manager is used to create systems at the runtime
   */
  class SystemManager
  {
    public:
      SystemManager(Engine* engine);
      virtual ~SystemManager();

      /**
       * Register system. Will create the factory for the system.
       *
       * @param id Factory id
       */
      template<class S>
      bool registerSystem(const std::string& id)
      {
        if(!std::is_base_of<EngineSystem, S>::value) {
          LOG(ERROR) << "Failed to register factory \"" << id << "\": specified type is not derived from EngineSystem";
          return false;
        }

        return registerFactory<ConcreteSystemFactory<S>>(id);
      }

      /**
       * Register system. Will create the factory for the system.
       */
      template<class S>
      bool registerSystem()
      {
        return registerFactory<ConcreteSystemFactory<S>>(S::ID);
      }

      /**
       * Unregister system factory using system ID
       */
      template<class S>
      bool removeSystem()
      {
        return removeSystem(S::ID);
      }

      /**
       * Unregister system factory
       *
       * @param id Factory ID
       */
      bool removeSystem(const std::string& id)
      {
        if(mFactories.count(id) == 0)
          return false;

        delete mFactories[id];
        mFactories.erase(id);
        return true;
      }

      /*
       * Register factory
       *
       * @param id Factory id
       */
      template<class F>
      bool registerFactory(const std::string& id)
      {
        if(!std::is_base_of<SystemFactory, F>::value) {
          LOG(ERROR) << "Failed to register factory \"" << id << "\": specified type is not derived from the SystemFactory";
          return false;
        }
        return registerFactory(id, new F());
      }

      /*
       * Register factory using ID from the system class
       */
      template<class F>
      bool registerFactory()
      {
        return registerFactory<F>(F::systemType::ID);
      }

      /*
       * Register factory
       *
       * @param id Factory id
       * @param factory Factory instance
       */
      bool registerFactory(const std::string& id, SystemFactory* factory)
      {
        if(mFactories.count(id) > 0) {
          LOG(WARNING) << "Tried to register \"" << id << "\" factory twice";
          return false;
        }

        mFactories[id] = factory;
        LOG(INFO) << "Registered system factory \"" << id << "\"";
        return true;
      }

      /**
       * SystemManager will find appropriate factory for id and will register the system in the engine
       *
       * @param id Factory id
       * @returns pointer to created EngineSystem, 0 if failed
       */
      EngineSystem* create(const std::string& id);

    private:
      typedef std::map<std::string, SystemFactory*> Factories;
      Factories mFactories;
      Engine* mEngine;
  };
}

#endif
