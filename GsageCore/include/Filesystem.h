#ifndef _Filesystem_H_
#define _Filesystem_H_

/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2019 Artem Chernyshev and contributors

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

#include "GsageDefinitions.h"
#include "ThreadSafeQueue.h"
#include "EventDispatcher.h"
#include "Poco/ThreadPool.h"
#include "Poco/Runnable.h"
#include "UpdateListener.h"
#include <atomic>

namespace Gsage {
  /**
   * FileEvent used for async I/O callbacks
   */
  class GSAGE_API FileEvent : public Event
  {
    public:
      static const Event::Type COPY_COMPLETE;
      static const Event::Type COPY_FAILED;

      FileEvent() {};
      FileEvent(Event::ConstType type, unsigned int id);

      unsigned int id;
  };

  class CopyWorker;

  /**
   * Filesystem utils powered by POCO
   */
  class Filesystem : public UpdateListener, public EventDispatcher
  {
    public:
      Filesystem();
      virtual ~Filesystem();

      /**
       * Recoursively copy directory in async fashion
       */
      std::shared_ptr<CopyWorker> copytreeAsync(const std::string& src, const std::string& dst);

      /**
       * Remove directory
       *
       * @param path
       * @param recursive If recursive is true and the file is a directory, recursively deletes all files in the directory.
       *
       * @returns true if success
       */
      bool rmdir(const std::string& path, bool recursive = false) const;

      /**
       * Check if path exists
       */
      bool exists(const std::string& path) const;

      /**
       * Get all files in directory
       *
       * @param path
       */
      std::vector<std::string> ls(const std::string& path) const;

      /**
       * Flush all copy complete events
       */
      void update(double time);

      /**
       * Queue file event
       */
      void queueEvent(FileEvent event);

    private:
      ThreadSafeQueue<FileEvent> mEvents;
      std::atomic<unsigned int> mCopyID;
      Poco::ThreadPool mThreadPool;
  };

  class CopyWorker : public Poco::Runnable
  {
    public:
      CopyWorker(Filesystem* fs, unsigned int id, const std::string& src, const std::string& dst) 
        : mSrc(src)
        , mDst(dst)
        , mID(id)
        , mFS(fs)
      {

      }

      virtual void run();

      inline unsigned int getID() const { return mID; }

      inline std::string str() const { return mSrc + " -> " + mDst; }
    private:
      std::string mSrc;
      std::string mDst;
      Filesystem* mFS;
      unsigned int mID;
  };
}

#endif
