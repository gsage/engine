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

namespace Gsage {

  ResourceManager::ResourceManager(const std::string& workdir)
    : mWorkdir(workdir)
  {

  }

  ResourceManager::~ResourceManager()
  {
    // cleanup resources here
  }

  bool ResourceManager::load(const DataProxy& resources)
  {
    Ogre::ResourceGroupManager& orgm = Ogre::ResourceGroupManager::getSingleton();
    std::string type;
    std::string section;
    std::string path;
    for(auto& pair : resources)
    {
      section = pair.first;
      for(auto& config : pair.second)
      {
        std::vector<std::string> list = split(config.second.as<std::string>(), ':');
        if(list.size() < 2)
        {
          LOG(ERROR) << "Malformed resource path: " << pair.second.as<std::string>();
          continue;
        }

        std::vector<std::string> pathList;
        if (!mWorkdir.empty())
        {
          pathList.push_back(mWorkdir);
        }

        for(std::vector<std::string>::iterator it = list.begin() + 1; it != list.end(); it++)
        {
          pathList.push_back(*it);
        }

        path = join(pathList, GSAGE_PATH_SEPARATOR);
        type = list[0];

        orgm.addResourceLocation(path, type, section, true, path.find(".zip") != std::string::npos);
      }

      orgm.initialiseResourceGroup(section);
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

  void ResourceManager::unload(const DataProxy& resources)
  {
    for(auto& pair : resources)
    {
      try {
        unload(pair.first);
      } catch(Ogre::Exception& e) {
        LOG(WARNING) << "Failed to unload resource " << pair.first << ", reason: " << e.what();
      }
    }
  }
}
