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

#include "ResourceManager.h"
#include "Logger.h"
#include "GsageFacade.h"
#include "FileLoader.h"
#include "MaterialLoader.h"
#include "ScopedLocale.h"

#if OGRE_VERSION >= 0x020100
#include "ogre/v2/HlmsUnlit.h"
#endif

namespace Gsage {

  std::tuple<std::string, std::string> ResourceManager::processPath(const std::string& line, const std::string& workdir)
  {
    std::vector<std::string> list = split(line, ';');
    std::string path;
    std::string type = list[0];
    int offset = 1;
    if(list.size() < 2)
    {
      type = "FileSystem";
      path = list[0];
      offset = 0;
    } else if(list.size() > 2) {
      LOG(ERROR) << "Malformed resource path: " << line;
      OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS,
                  std::string("Malformed resource path: ") + line,
                  "processPath");
    }

    if(!mFacade->filesystem()->isAbsolute(path)) {
      std::vector<std::string> pathList;
      if (!workdir.empty())
      {
        pathList.push_back(workdir);
      }

      for(std::vector<std::string>::iterator it = list.begin() + offset; it != list.end(); it++)
      {
        pathList.push_back(*it);
      }

      path = join(pathList, GSAGE_PATH_SEPARATOR);
    }
    
    path = FileLoader::getSingletonPtr()->searchFile(path);
    if(path.empty()) {
      // failure
      return std::make_tuple("", "");
    }

    return std::make_tuple(path, type);
  }

  ResourceManager::ResourceManager(GsageFacade* facade, MaterialLoader* materialLoader)
    : mHlmsLoaded(false)
    , mFacade(facade)
    , mMaterialLoader(materialLoader)
  {
  }

  ResourceManager::~ResourceManager()
  {
    // cleanup resources here
  }

  bool ResourceManager::load(const DataProxy& input, bool initialize)
  {
    Ogre::ResourceGroupManager& orgm = Ogre::ResourceGroupManager::getSingleton();
    std::string type;
    std::string section;
    std::string path;
    std::string workdir;
    DataProxy resources;
    if(input.getStoredType() == DataWrapper::Array) {
      resources.put("Extra", input);
    } else {
      resources = input;
    }

    resources.read("workdir", workdir);

#if OGRE_VERSION >= 0x020100
    if(!mHlmsLoaded) {
      Ogre::Root& root = Ogre::Root::getSingleton();
      DataProxy hlms;
      resources.read("Hlms", hlms);
      Ogre::RenderSystem *renderSystem = root.getRenderSystem();
      mShaderSyntax = "GLSL";
      if( renderSystem->getName() == "Direct3D11 Rendering Subsystem" )
        mShaderSyntax = "HLSL";
      else if( renderSystem->getName() == "Metal Rendering Subsystem" )
        mShaderSyntax = "Metal";
      mCommonFolder = hlms.get("common", "FileSystem:materials/Common");

      LOG(INFO) << "[Hlms] Loading configuration";
      registerHlms<Ogre::HlmsPbs>(hlms.get("pbs", "FileSystem:materials/Pbs"));
      registerHlms<Gsage::HlmsUnlit>(hlms.get("unlit", "FileSystem:materials/Unlit"));
      mHlmsLoaded = true;
    }
#endif

    for(auto& pair : resources)
    {
      section = pair.first;
      if(section == "workdir" || section == "Hlms") {
        continue;
      }

      for(auto& config : pair.second)
      {
        std::tie(path, type) = processPath(config.second.as<std::string>(), workdir);
        if(type.empty()) {
          LOG(ERROR) << "Failed to find resource " << path;
          continue;
        }
        LOG(INFO) << "Adding resource location " << path;
        orgm.addResourceLocation(path, type, section, true, type == "Zip");
        if(type != "Zip") {
          mMaterialLoader->scan(path);
        }
      }

      if(initialize) {
#if OGRE_VERSION_MAJOR == 2
        orgm.initialiseResourceGroup(section, true);
#else
        {
          ScopedCLocale l(true);
          orgm.initialiseResourceGroup(section);
        }
#endif
      }
    }
    return true;
  }

  void ResourceManager::unload(const std::string& group)
  {
    Ogre::ResourceGroupManager::getSingleton().unloadUnreferencedResourcesInGroup(group);
  }

  void ResourceManager::unload()
  {
    Ogre::ResourceGroupManager& orgm = Ogre::ResourceGroupManager::getSingleton();

    for(auto& groupName : orgm.getResourceGroups())
    {
      orgm.unloadUnreferencedResourcesInGroup(groupName);
    }
  }

  void ResourceManager::unload(const DataProxy& input)
  {
    DataProxy resources;
    if(input.getStoredType() == DataWrapper::Array) {
      resources.put("Extra", input);
    } else {
      resources = input;
    }

    for(auto& pair : resources)
    {
      if(pair.first == "workdir") {
        continue;
      }

      try {
        unload(pair.first);
      } catch(Ogre::Exception& e) {
        LOG(WARNING) << "Failed to unload resource " << pair.first << ", reason: " << e.what();
      }
    }
  }
}
