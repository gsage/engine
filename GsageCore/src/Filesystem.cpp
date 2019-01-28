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

#include "Filesystem.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Logger.h"

namespace Gsage {
  const Event::Type FileEvent::COPY_COMPLETE = "FileEvent::COPY_COMPLETE";
  const Event::Type FileEvent::COPY_FAILED = "FileEvent::COPY_FAILED";

  FileEvent::FileEvent(Event::ConstType type, unsigned int pID)
    : Event(type)
    , id(pID)
  {
  }

  Filesystem::Filesystem()
  {
  }

  Filesystem::~Filesystem()
  {
    mThreadPool.joinAll();
  }

  std::shared_ptr<CopyWorker> Filesystem::copytreeAsync(const std::string& src, const std::string& dst)
  {
    unsigned int id = mCopyID.fetch_add(1);
    std::shared_ptr<CopyWorker> worker = std::make_shared<CopyWorker>(this, id, src, dst);
    CopyWorker* w = worker.get();
    mThreadPool.start(*w);
    return worker;
  }

  bool Filesystem::rmdir(const std::string& path, bool recursive) const
  {
    Poco::Path p(path);
    Poco::File f(p);
    if(f.exists()) {
      f.remove(recursive);
      return true;
    }
    return false;
  }

  bool Filesystem::exists(const std::string& path) const
  {
    Poco::Path p(path);
    Poco::File f(p);
    return f.exists();
  }

  std::vector<std::string> Filesystem::ls(const std::string& path) const
  {
    std::vector<std::string> res;
    Poco::File f(path);
    if(!f.exists()) {
      return res;
    }

    if(!f.isDirectory()) {
      res.push_back(path);
      return res;
    }

    f.list(res);
    return res;
  }

  void Filesystem::update(double time)
  {
    for(size_t i = 0; i < mEvents.size(); i++){
      FileEvent e;
      if(mEvents.get(e)) {
        fireEvent(e);
      }
    }
  }

  void Filesystem::queueEvent(FileEvent event)
  {
    mEvents << event;
  }

  void CopyWorker::run()
  {
    Poco::File src(mSrc);
    if(!src.exists()) {
      mFS->queueEvent(FileEvent(FileEvent::COPY_FAILED, mID));
      return;
    }

    try{

      Poco::Path p(mDst);
      p.makeParent();

      Poco::File directory(p);
      if(!directory.exists())
        directory.createDirectories();

      src.copyTo(mDst);
      mFS->queueEvent(FileEvent(FileEvent::COPY_COMPLETE, mID));
    } catch (const std::exception& e) {
      LOG(ERROR) << "Failed to copy " << mSrc << "->" << mSrc << ": " << e.what();
      mFS->queueEvent(FileEvent(FileEvent::COPY_FAILED, mID));
    }
  }
}
