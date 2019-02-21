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
#include <atomic>
#include <channel>

#include "Component.h"
#include "GsageDefinitions.h"
#include "DataProxy.h"

namespace Gsage
{
  class Engine;
  class EntityComponent;
  class GsageFacade;
  class EngineSystem;

  /**
   * Async task object
   */
  class Task
  {
    public:
      typedef std::function<void()> Function;

      Task(Function func);
      virtual ~Task();

      /**
       * Cancel task
       */
      void cancel();

      /**
       * Run task
       */
      void run();
    private:
      Function mFunction;
      std::atomic<bool> mCancelled;
  };

  typedef std::shared_ptr<Task> TaskPtr;

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
       * Shutdown is called before the system is removed from the engine
       */
      virtual void shutdown();

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
       * Sets gsage facade instance (called, when system is create by gsage facade)
       *
       * @param facade GsageFacade pointer
       */
      virtual void setFacadeInstance(GsageFacade* facade);
      /**
       * Reset engine instance (called, when system is removed from engine)
       */
      void resetEngineInstance();
      /**
       * Flag that means that the system was initialized
       */
      bool isReady() { return mReady.load(); }
      /**
       * Set system ready
       * @param value Ready flag
       */
      void setReady(bool value) {
        mReadyWasSet.store(true);
        mReady.store(value);
      }

      /**
       * Check if the system was just set up by checking mReadyWasSet flag
       * Will return true only once
       */
      bool becameReady()
      {
        if(mReadyWasSet.load()) {
          mReadyWasSet.store(false);
          return true;
        }

        return false;
      }
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

      /**
       * Get system name as it is registered in the Engine
       * @returns system's name string
       */
      const std::string& getName() const;

      /**
       * Called by the engine instance, sets system name
       * @param name system's name as it's registered in the Engine
       */
      void setName(const std::string& name);

      /**
       * Get system kind created by the SystemFactory
       * @returns system kind string
       */
      const std::string& getKind() const;

      /**
       * Called by the SystemFactory instance, sets system kind
       * @param name system kind string
       */
      void setKind(const std::string& kind);

      /**
       * Count of threads to use for the system
       */
      inline int getThreadsNumber() { return mThreadsNumber; };

      /**
       * Check if multithreading is enabled
       */
      inline bool dedicatedThread() { return mDedicatedThread; };
      /**
       * This function tells Engine if the particular system allows running in separate thread
       *
       * @returns false by default
       */
      virtual bool allowMultithreading() { return false; }

      /**
       * Runs async task using system thread pool
       *
       * @param task function to run
       */
      virtual TaskPtr asyncTask(Task::Function func);
    protected:
      /**
       * Update configuration
       */
      virtual void configUpdated();

      Engine* mEngine;
      DataProxy mConfig;
      GsageFacade* mFacade;
      DataProxy mSystemInfo;

      std::string mName;
      std::string mKind;

      int mThreadsNumber;

      bool mEnabled;
      bool mConfigDirty;
      bool mDedicatedThread;

      std::atomic_bool mReadyWasSet;
      std::atomic_bool mReady;
      std::atomic_bool mShutdown;

      cpp::channel<TaskPtr> mTasksQueue;
      SignalChannel mShutdownChannel;

      std::vector<std::thread> mBackgroundWorkers;
  };
}

#endif
