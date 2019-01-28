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
#include <nlohmann/json.hpp>
#include <inja/inja.hpp>
#include <sol_forward.hpp>

namespace Gsage {
  /**
   * FS wrapper
   */
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
       * Add resource search folder
       *
       * @param index folder priority
       * @param path folder path
       */
      void addSearchFolder(int index, const std::string& path);

      /**
       * Remove resource search folder
       *
       * @param path folder path
       *
       * @returns true if removed anything
       */
      bool removeSearchFolder(const std::string& path);

      /**
       * Initialize file loader instance
       *
       * @param format: format to use for the file loading
       * @param environment: environment for the application
       */
      static void init(FileLoader::Encoding format, const DataProxy& environment);

      /**
       * Load raw file
       *
       * @param path path to file
       * @param dest std::string
       */
      bool load(const std::string& path, std::string& dest, std::ios_base::openmode mode = std::ios_base::in) const;

      /**
       * Load file and process template
       *
       * @param path path to file
       * @param dest std::string
       * @param context template context
       */
      bool loadTemplate(const std::string& path, std::string& dest, const DataProxy& context);

      /**
       * Add lua filter to file template engine
       *
       * @param function: Callback function
       */
      void addTemplateCallback(const std::string& name, int argsNumber, sol::function function);

      /**
       * Get file stream
       */
      std::ifstream stream(const std::string& path) const;

      /**
       * Write string to file
       * @param path path to file
       * @param str string to save
       * @param rootDir directory to dump the file to
       */
      bool dump(const std::string& path, const std::string& str, const std::string& rootDir = "") const;

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
      std::pair<std::string, bool> loadFile(const std::string& path, std::ios_base::openmode mode = std::ios_base::in) const;
      bool parse(const std::string& data, DataProxy& dest) const;

      typedef std::map<int, std::string> ResourceFolders;

      ResourceFolders mResourceSearchFolders;
      Encoding mFormat;
      DataProxy mEnvironment;

      inja::Environment* mInjaEnv;

      static FileLoader* mInstance;
  };
}

#endif
