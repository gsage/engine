#ifndef _MaterialLoader_H_
#define _MaterialLoader_H_

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

#include "EventSubscriber.h"
#include "EventDispatcher.h"
#include "GsageDefinitions.h"
#include "DataProxy.h"

namespace Gsage {
  class OgreRenderSystem;
  class GsageFacade;

  /**
   TODO: INDEXER

   {
     "filename": {
        "checksum": "...",
        "changed": "...",
        "materials": []
     }
   }

   for(auto& file : filesystem->ls(folder)) {
     if(filesystem->extension(file) != "material") {
       continue;
     }
   }

   */

  /**
   * Custom OGRE material loader
   */
  class MaterialLoader : public EventDispatcher, public EventSubscriber<MaterialLoader>
  {
    public:
      MaterialLoader(OgreRenderSystem* render, GsageFacade* facade);

      /**
       * Loads material from any of resource folders
       *
       * @param material Material name
       * @param group Material group
       * @param background Loads material in background
       */
      bool load(const std::string& material, const std::string& group, bool background = true);

      /**
       * Indexer scans materials files to detect materials sets defined there
       *
       * @param folder Folder to scan
       */
      void scan(const std::string& folder);

    private:
      void scanFolder(const std::string& folder, std::vector<std::string>& files, const std::string extension);

      void readIndexFile();

      void writeIndexFile();

      bool onEnvUpdated(EventDispatcher* sender, const Event& event);

      void reloadIndex();

      OgreRenderSystem* mRender;
      GsageFacade* mFacade;
      DataProxy mIndex;
      std::string mWorkdir;

      struct FileInfo {
        std::string path;
        signed long modified;
        std::string folder;
      };

      typedef std::map<std::string, FileInfo> MaterialIndex;

      MaterialIndex mMaterialIndex;
  };
}

#endif
