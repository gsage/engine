/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Artem Chernyshev

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

#include "ResourceMonitor.h"
#include "GsageDefinitions.h"

#if GSAGE_PLATFORM == GSAGE_LINUX
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <stdio.h>
#include <string.h>
#elif GSAGE_PLATFORM == GSAGE_APPLE
#include <mach/mach.h>
#endif

namespace Gsage {

  ResourceMonitor::ResourceMonitor(float updateInterval)
    : mUpdateInterval(updateInterval)
    , mElapsed(.0f)
    , mPreviousIdleTicks(0)
    , mPreviousTotalTicks(0)
  {
    memset(&mStats, 0, sizeof(mStats));
  }

  ResourceMonitor::~ResourceMonitor()
  {
  }

  void ResourceMonitor::update(double time)
  {
    mElapsed += time;
    if(mElapsed < mUpdateInterval) {
      return;
    }
#if GSAGE_PLATFORM == GSAGE_APPLE
    mStats.lastCPU = getCPULoad();
#endif

    mElapsed = 0;
  }

  const ResourceMonitor::Stats& ResourceMonitor::getStats() const
  {
    return mStats;
  }

  float ResourceMonitor::getCPULoad()
  {
#if GSAGE_PLATFORM == GSAGE_APPLE
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count) == KERN_SUCCESS)
    {
      unsigned long long totalTicks = 0;
      for(int i = 0; i < CPU_STATE_MAX; i++) {
        totalTicks += cpuinfo.cpu_ticks[i];
      }
      unsigned long long idleTicks = cpuinfo.cpu_ticks[CPU_STATE_IDLE];
      unsigned long long totalTicksSinceLastTime = totalTicks - mPreviousTotalTicks;
      unsigned long long idleTicksSinceLastTime  = idleTicks - mPreviousIdleTicks;
      float ret = 1.0f-((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime)/totalTicksSinceLastTime : 0);
      mPreviousTotalTicks = totalTicks;
      mPreviousIdleTicks  = idleTicks;
      return ret;
    } else {
      return -1.0f;
    }
#else
    return -1.0f;
#endif
  }

}
