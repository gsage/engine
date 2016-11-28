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

#include "EngineRunner.h"

namespace Gsage {
  EngineRunner::EngineRunner(GsageFacade& engine, const std::string& config, const std::string& resourceFolder)
    : mConfig(config)
    , mResourceFolder(resourceFolder)
    , mThread(0)
    , mRunning(false)
    , mEngine(engine)
  {
  }

  EngineRunner::~EngineRunner()
  {
    if(mThread)
      delete mThread;
  }

  bool EngineRunner::start()
  {
    if(mRunning)
      return false;

    mThread = new std::thread(&EngineRunner::startAndRun, this);
    mRunning = true;
    return true;
  }

  bool EngineRunner::stop()
  {
    if(!mRunning)
      return false;
    mEngine.halt();
    mThread->join();
    mRunning = false;
    return true;
  }

  bool EngineRunner::startAndRun()
  {
    bool res = mEngine.initialize(mConfig, mResourceFolder);
    while(mEngine.update())
    {
    }

    return true;
  }
}
