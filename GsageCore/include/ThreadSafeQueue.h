#ifndef _ThreadSafeQueue_H_
#define _ThreadSafeQueue_H_

/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2018 Artem Chernyshev and contributors

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

#include <mutex>
#include <queue>


namespace Gsage {
  template<class C>
  class ThreadSafeQueue
  {
    public:
      struct QueueItem
      {
        C     data;
        bool  isNull;

        void operator<<(ThreadSafeQueue<C> queue)
        {
          isNull = queue.get(data) > 0;
        }
      };

      ThreadSafeQueue(int limit = 1024)
        : mLimit(limit)
      {
      }

      virtual ~ThreadSafeQueue()
      {
      }

      /**
       * Queue item
       *
       * @param item to queue
       */
      void push(C item)
      {
        mMutex.lock();

        // start to pop items from the queue if reached the limit
        if(mQueue.size() >= mLimit) {
          mQueue.pop();
        }

        mQueue.push(item);
        mMutex.unlock();
      }

      ThreadSafeQueue& operator<<(C item)
      {
        push(item);
        return *this;
      }

      /**
       * Get queue item
       * @param dest to write item to
       */
      size_t get(C& dest)
      {
        mMutex.lock();
        size_t tail = mQueue.size();
        if(tail > 0) {
          dest = mQueue.front();
          mQueue.pop();
        }
        mMutex.unlock();
        return tail;
      }

      size_t size()
      {
        mMutex.lock();
        size_t size = mQueue.size();
        mMutex.unlock();
        return size;
      }
    private:
      std::queue<C> mQueue;
      int mLimit;

      mutable std::mutex mMutex;
  };
}

#endif
