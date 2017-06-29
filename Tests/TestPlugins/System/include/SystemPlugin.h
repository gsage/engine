#ifndef _SystemPlugin_H_
#define _SystemPlugin_H_

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

#include "IPlugin.h"
#include "EngineSystem.h"

namespace Gsage {
  class TestComponent : public EntityComponent
  {
    public:
      static const std::string SYSTEM;
  };

  /**
   * Fake test system
   */
  class TestSystem : public EngineSystem
  {
    public:
      static const std::string ID;
      typedef TestComponent type;
      virtual ~TestSystem()
      {
      }

      bool initialize(const DataProxy& config)
      {
        EngineSystem::initialize(config);
        mSetting = config.get<bool>("testSetting", false);
        return true;
      }

      void update(const double& time)
      {
      }

      EntityComponent* createComponent(const DataProxy& data, Entity* owner)
      {
        return 0;
      }
      bool removeComponent(EntityComponent* component)
      {
        return 0;
      }

      void unloadComponents()
      {
      }
    private:
      bool mSetting;
  };

  /**
   * Test plugin
   */
  class SystemPlugin : public IPlugin
  {
    public:
      SystemPlugin();
      virtual ~SystemPlugin();

      /**
       * Get ois plugin name
       */
      const std::string& getName() const;

      /**
       * Unregisters input system
       */
      bool installImpl();

      /**
       * Registers input system
       */
      void uninstallImpl();

      /**
       * Set up lua bindings as well
       */
      void setupLuaBindings();

  };
}

#endif
