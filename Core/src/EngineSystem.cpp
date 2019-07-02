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

#include "EngineSystem.h"
#include "EngineEvent.h"
#include "Engine.h"

namespace Gsage
{

  Task::Task(Task::Function func)
    : mFunction(func)
    , mCancelled(false)
  {

  }

  Task::~Task()
  {
  }

  void Task::cancel()
  {
    mCancelled.store(true);
  }

  void Task::run()
  {
    if(!mCancelled.load()) {
      mFunction();
    }
  }

  EngineSystem::EngineSystem()
    : mReady(false)
    , mConfigDirty(false)
    , mEnabled(true)
    , mThreadsNumber(1)
    , mDedicatedThread(false)
    , mReadyWasSet(false)
    , mShutdown(false)
    , mRestart(false)
  {
  }

  EngineSystem::~EngineSystem()
  {
  }

  bool EngineSystem::configure(const DataProxy& config)
  {
    mergeInto(mConfig, config);
    mEnabled = mConfig.get("enabled", true);
    mConfigDirty = true;
    return true;
  }

  const DataProxy& EngineSystem::getConfig()
  {
    return mConfig;
  }

  bool EngineSystem::initialize(const DataProxy& settings)
  {
    mShutdown.store(false);
    mConfig = settings;
    mThreadsNumber = mConfig.get("threadsNumber", 1);
    mDedicatedThread = mConfig.get("dedicatedThread", false);
    if(mDedicatedThread && !allowMultithreading()) {
      LOG(ERROR) << "System " << mName << " does not support multithreaded mode";
      return false;
    }
    setReady(true);

    size_t backgroundWorkersCount = mConfig.get("backgroundWorkersCount", 0);
    if(backgroundWorkersCount > 0) {
      size_t count = backgroundWorkersCount - mBackgroundWorkers.size();
      for(int i = 0; i < count; i++) {
        mBackgroundWorkers.push_back(std::thread([&, i](){
          LOG(INFO) << "Starting background worker " << i;
          bool run = true;
          while(run) {
            ChannelSignal signal(ChannelSignal::NONE);
            cpp::select select;
            select.recv(mTasksQueue, [](TaskPtr task){
              task->run();
            });
            select.recv_only(mShutdownChannel, signal);
            select.wait(std::chrono::milliseconds(200));

            run = !mShutdown.load() && signal != ChannelSignal::SHUTDOWN;
          }
          LOG(INFO) << "Stopped background worker " << i;
        }));
      }
    }

    mEngine->fireEvent(SystemChangeEvent(SystemChangeEvent::SYSTEM_STARTED, getName(), this));
    LOG(INFO) << "System " << getName() << " was started";
    return true;
  }

  void EngineSystem::shutdown()
  {
    mEngine->fireEvent(SystemChangeEvent(SystemChangeEvent::SYSTEM_STOPPING, getName(), this));
    mShutdown.store(true);
    for(int i = 0; i < mBackgroundWorkers.size(); i++) {
      mShutdownChannel.send(ChannelSignal::SHUTDOWN);
    }

    for(auto& t : mBackgroundWorkers) {
      if(t.joinable()) {
        t.join();
      }
    }
    mBackgroundWorkers.clear();
    setReady(false);
  }

  void EngineSystem::restart()
  {
    if(!mReady)
      return;

    // if the system if running, it should be restarted only after update finished
    if(mEnabled) {
      mRestart = true;
    } else {
      shutdown();
      initialize(mConfig);
    }
  }

  void EngineSystem::setEngineInstance(Engine* value)
  {
    mEngine = value;
  }

  void EngineSystem::setFacadeInstance(GsageFacade* value)
  {
    mFacade = value;
  }

  void EngineSystem::resetEngineInstance()
  {
    mEngine = NULL;
  }

  void EngineSystem::configUpdated()
  {
    mConfigDirty = false;
  }

  void EngineSystem::setEnabled(bool value)
  {
    mEnabled = value;
    mConfig.put("enabled", value);
  }

  const DataProxy& EngineSystem::getSystemInfo() const
  {
    return mSystemInfo;
  }

  const std::string& EngineSystem::getName() const
  {
    return mName;
  }

  void EngineSystem::setName(const std::string& name)
  {
    mName = name;
  }

  const std::string& EngineSystem::getKind() const
  {
    return mKind;
  }

  void EngineSystem::setKind(const std::string& name)
  {
    mKind = name;
  }

  TaskPtr EngineSystem::asyncTask(Task::Function func)
  {
    if(mBackgroundWorkers.size() == 0) {
      LOG(WARNING) << "No background workers are started in system " << mName << ". Task will be run in foreground";
      func();
      return nullptr;
    }

    TaskPtr t = std::make_shared<Task>(func);
    mTasksQueue.send(t);
    return t;
  }
}
