#ifndef _ResourceMonitor_H_
#define _ResourceMonitor_H_

/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Gsage Authors

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

#include "UpdateListener.h"

namespace Gsage {
  /**
   * Utility class used to monitor CPU and MEM usage
   */
  class ResourceMonitor : public UpdateListener
  {
    public:
      /**
       * Struct containing gathered stats
       */
      struct Stats
      {
        long long physicalMem;
        long long virtualMem;
        float lastCPU;
        float lastSysCPU;
        float lastUserCPU;
      };

      ResourceMonitor(float updateInterval);
      virtual ~ResourceMonitor();

      /**
       * Update stats
       * @param time elapsed time
       */
      void update(double time);

      /**
       * Get gathered stats
       * @return ResourceManager::Stats
       */
      const Stats& getStats() const;

      /**
       * Get current CPU load
       * @return cpu load
       */
      float getCPULoad();
    private:
      Stats mStats;

      float mUpdateInterval;
      float mElapsed;
      unsigned long long mPreviousTotalTicks;
      unsigned long long mPreviousIdleTicks;
  };
}

#endif
