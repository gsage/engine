#ifndef _FileLoader_H_
#define _FileLoader_H_
#
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

#include "DataProxy.h"

namespace Gsage {
  class FileLoader
  {
    public:
      enum Encoding {
        Msgpack,
        Json
      };

      FileLoader(Encoding format, const DataProxy& environment);
      virtual ~FileLoader();

      /**
       * Get FileLoader singleton
       */
      static FileLoader getSingleton();

      /**
       * Get FileLoader singleton pointer
       */
      static FileLoader* getSingletonPtr();

      /**
       * Initialize file loader instance
       *
       * @param format: format to use for the file loading
       * @param environment: environment for the application
       */
      static void init(FileLoader::Encoding format, const DataProxy& environment);

      /**
       * Load file with environment and params
       *
       * @param path: path to file
       * @param params: parameters
       * @param dest: DataProxy to load into
       */
      bool load(const std::string& path, const DataProxy& params, DataProxy& dest) const;

      /**
       * Load file with environment and params
       *
       * @param path: path to file
       * @param params: parameters
       */
      std::pair<DataProxy, bool> load(const std::string& path, const DataProxy& params) const;

      /**
       * Load file with environment
       *
       * @param path: path to file
       */
      std::pair<DataProxy, bool> load(const std::string& path) const;

      /**
       * Dump file to disk
       *
       * @param path: path to file
       * @param value: value to dump
       */
      void dump(const std::string& path, const DataProxy& value) const;

    private:
      std::pair<std::string, bool> loadFile(const std::string& path) const;
      bool parse(const std::string& data, DataProxy& dest) const;

      Encoding mFormat;
      DataProxy mEnvironment;

      static FileLoader* mInstance;
  };
}

#endif
