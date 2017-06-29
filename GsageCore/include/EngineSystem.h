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


#ifndef _EngineSystem_H_
#define _EngineSystem_H_

#include <vector>

#include "Component.h"
#include "GsageDefinitions.h"
#include "DataProxy.h"

namespace Gsage
{
  class Engine;
  class EntityComponent;

  /**
   * Abstract system class.
   */
  class EngineSystem
  {
    public:
      EngineSystem();
      virtual ~EngineSystem();
      /**
       * Configure the system. This method can be called several times
       *
       * @param config DataProxy system configuration.
       */
      virtual bool configure(const DataProxy& config);
      /**
       * Get configuration of the system.
       */
      virtual const DataProxy& getConfig();
      /**
       * Initialization of the system should be done here
       * This method is called only once by the Engine itself.
       *
       * @param settings Initial settings of the system
       */
      virtual bool initialize(const DataProxy& settings);

      /**
       * Updates system
       * @param time Delta time
       */
      virtual void update(const double& time) = 0;
      /**
       * Create component from the data object
       * @param data Component data description
       */
      virtual EntityComponent* createComponent(const DataProxy& data, Entity* owner) = 0;
      /**
       * Remove component from engine system
       * @param component Pointer to the component for removal
       */
      virtual bool removeComponent(EntityComponent* component) = 0;
      /**
       * Virtual method, that should remove all components, related to the system
       */
      virtual void unloadComponents() = 0;
      /**
       * Sets engine instance (called, when system is added to engine)
       *
       * @param engine Engine pointer
       */
      virtual void setEngineInstance(Engine* engine);
      /**
       * Reset engine instance (called, when system is removed from engine)
       */
      void resetEngineInstance();
      /**
       * Flag that means that the system was initialized
       */
      bool isReady() { return mReady; }
      /**
       * Flag that means that the system should be updated
       */
      bool isEnabled() { return mEnabled; }
      /**
       * Enable/disable the system
       * @param value
       */
      void setEnabled(bool value);

      /**
       * Get detailed type of the system.
       * For example to detect if the render system is ogre.
       */
      const DataProxy& getSystemInfo() const;
    protected:

      /**
       * Update configuration
       */
      virtual void configUpdated();

      Engine* mEngine;
      bool mReady;
      DataProxy mConfig;
      bool mConfigDirty;

      bool mEnabled;
      DataProxy mSystemInfo;
  };
}

#endif
