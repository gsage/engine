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

#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <mutex>

#include "DataProxy.h"
#if OGRE_VERSION >= 0x020100
#include <OgreHlmsUnlit.h>
#include <OgreHlmsPbs.h>
#include <OgreHlmsManager.h>
#include <OgreArchiveManager.h>
#endif

namespace Gsage {
  class GsageFacade;
  class MaterialLoader;
  class ResourceManager
  {
    public:
      ResourceManager(GsageFacade* facade, MaterialLoader* materialLoader);
      virtual ~ResourceManager(void);

      /**
       * Loads all resource groups
       *
       * @param resources DataProxy with all required resources
       * @param initialize Intializes resource group
       */
      bool load(const DataProxy& resources, bool initialize = true);

      /**
       * Unload resources
       *
       * @param group Resource group to unload
       */
      void unload(const std::string& group);

      /**
       * Unload all resources
       */
      void unload();

      /**
       * Unload resources
       * @param resources Resource groups DataProxy
       */
      void unload(const DataProxy& resources);

#if OGRE_VERSION >= 0x020100
      /**
       * Register and load Hlms data
       *
       * @param resourcePath Hlms data folder
       */
      template<class T>
      void registerHlms(const std::string& resourcePath)
      {
        if(!std::is_base_of<Ogre::Hlms, T>::value) {
          OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS,
                      "Failed to register Hlms",
                      "Provided type should inherit Ogre::Hlms"
                      "ResourceManager::registerHlms" );
        }
        std::string type;
        std::string path;

        Ogre::Root& root = Ogre::Root::getSingleton();

        std::tie(path, type) = processPath(mCommonFolder);
        if(type.empty()) {
          LOG(ERROR) << "Failed to find resource " << path;
          return;
        }
        std::vector<std::string> folders;
        //Fill the library folder paths with the relevant folders
        folders.push_back(path + "/" + mShaderSyntax);
        folders.push_back(path + "/Any");

        auto archiveManager = Ogre::ArchiveManager::getSingletonPtr();
        auto loadArchive = [&] (const std::string& p, const std::string& t) -> Ogre::Archive* {
          LOG(INFO) << "[Hlms] Loading archive " << p << " type: " << t;
          return archiveManager->load(p, t, p.find(".zip") != std::string::npos);
        };

        Ogre::ArchiveVec library;
        for(auto folder : folders) {
          library.push_back(loadArchive(folder, type));
        }
        std::tie(path, type) = processPath(resourcePath);
        library.push_back(loadArchive(path + "/Any", type));
        Ogre::Archive *archive = loadArchive(path + "/" + mShaderSyntax, type);
        T *hlms = OGRE_NEW T(archive, &library);
        root.getHlmsManager()->registerHlms(hlms, true);
      }
#endif

      std::tuple<std::string, std::string> processPath(const std::string& line, const std::string& workdir = "");
    private:
      bool mHlmsLoaded;

      GsageFacade* mFacade;
      MaterialLoader* mMaterialLoader;

#if OGRE_VERSION >= 0x020100
      std::string mShaderSyntax;
      std::string mCommonFolder;
#endif
  };
}

