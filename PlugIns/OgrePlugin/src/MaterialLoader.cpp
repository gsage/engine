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

#include "MaterialLoader.h"
#include "systems/OgreRenderSystem.h"
#include "GsageFacade.h"
#include "Filesystem.h"
#include "Engine.h"
#include "EngineEvent.h"
#include "ScopedLocale.h"

#if OGRE_VERSION >= 0x020100
#include <OgreHlmsManager.h>
#include <OgreHlmsDatablock.h>
#endif

#include <iostream>
#include <regex>

namespace Gsage {

  MaterialLoader::MaterialLoader(OgreRenderSystem* render, GsageFacade* facade)
    : mRender(render)
    , mFacade(facade)
  {
    reloadIndex();

    EventSubscriber<MaterialLoader>::addEventListener(mFacade->getEngine(), EngineEvent::ENV_UPDATED, &MaterialLoader::onEnvUpdated);
  }

  bool MaterialLoader::load(const std::string& material, const std::string& group, bool background)
  {
    if(!contains(mMaterialIndex, material)) {
      return false;
    }

    FileInfo info = mMaterialIndex[material];

    if(!mFacade->filesystem()->exists(info.path)) {
      return false;
    }

    bool modified = mFacade->filesystem()->getLastModified(info.path) != info.modified;

#if OGRE_VERSION >= 0x020100
    Ogre::HlmsManager* hlmsManager = Ogre::Root::getSingleton().getHlmsManager();
    Ogre::HlmsDatablock* datablock = hlmsManager->getDatablockNoDefault(material);
    if(!modified && datablock) {
      return true;
    }

    if(datablock) {
      Ogre::Hlms* hlms = datablock->getCreator();
      auto linkedRenderables = datablock->getLinkedRenderables();
      auto itor = linkedRenderables.begin();
      auto end  = linkedRenderables.end();
      while( itor != end )
      {
        (*itor)->setDatablock(hlms->getDefaultDatablock());
        ++itor;
      }
      hlms->destroyDatablock(datablock->getName());
      datablock = NULL;
    }
#endif
    {
      ScopedCLocale l(true);
      std::ifstream ifs(info.path, std::ios::in);
      Ogre::DataStreamPtr dataStream(new Ogre::FileStreamDataStream(info.path, &ifs, false));
      Ogre::MaterialManager::getSingletonPtr()->parseScript(dataStream, group);
      ifs.close();
    }
    return true;
  }

  void MaterialLoader::scan(const std::string& file)
  {
    Filesystem* fs = mFacade->filesystem();
    std::string folder = file;
    if(!fs->isDirectory(file)) {
      folder = fs->directory(file);
    }
    LOG(INFO) << "Scanning folder " << folder;

    std::vector<std::string> files;
    scanFolder(folder, files, "material");

    bool dirty = false;
    for(auto& file : files) {
      signed long modified = fs->getLastModified(file);
      DataProxy cached;
      mIndex.read(file, cached, false);
      if(modified != cached.get<signed long>("modified", -1)) {
        // scanning material file
        std::string content;
        if(!FileLoader::getSingletonPtr()->load(file, content)) {
          LOG(ERROR) << "Failed to scan file " << file;
          continue;
        }
        std::regex r(R"((material|hlms)\s+([\w\/\.]+)(\s+(unlit|pbs))?)", std::regex_constants::icase);
        std::smatch sm;
        while(std::regex_search(content, sm, r))
        {
          if(sm.size() < 2) {
            continue;
          }

          std::string material = sm[2].str();
          LOG(TRACE) << "Detected material " << material;
          mMaterialIndex[material] = FileInfo{
            file,
            modified,
            folder
          };
          dirty = true;
          content = sm.suffix();
        }
      }
    }
    if(dirty) {
      writeIndexFile();
    }
  }

  void MaterialLoader::scanFolder(const std::string& folder, std::vector<std::string>& files, const std::string extension)
  {
    Filesystem* fs = mFacade->filesystem();
    std::vector<std::string> parts;
    parts.push_back(folder);

    for(auto& file : fs->ls(folder)) {
      parts.push_back(file);
      std::string fullPath = fs->join(parts);
      parts.pop_back();

      if(fs->isDirectory(fullPath)) {
        scanFolder(fullPath, files, extension);
        continue;
      }

      if(fs->extension(file) != extension) {
        continue;
      }

      files.push_back(fullPath);
    }
  }

  void MaterialLoader::readIndexFile()
  {
    Filesystem* fs = mFacade->filesystem();
    std::vector<std::string> parts;
    parts.push_back(mWorkdir);
    parts.push_back(".materials.index");

    std::string indexPath = fs->join(parts);
    if(fs->exists(indexPath)) {
      bool success = false;
      std::tie(mIndex, success) = Gsage::load(indexPath, DataWrapper::JSON_OBJECT);
      if(!success)
        return;

      for(auto& pair : mIndex) {
        for(auto& mat : pair.second.get("materials", DataProxy())) {
          mMaterialIndex[mat.second.as<std::string>()] = FileInfo{
            pair.first,
            pair.second.get<signed long>("modified", 0),
            pair.second.get("folder", "")
          };
        }
      }
    }
  }

  void MaterialLoader::writeIndexFile()
  {
    mIndex = DataProxy();
    Filesystem* fs = mFacade->filesystem();
    std::vector<std::string> parts;
    parts.push_back(mWorkdir);
    parts.push_back(".materials.index");

    std::string indexPath = fs->join(parts);

    for(auto& pair : mMaterialIndex) {
      DataProxy materials;
      DataProxy object;
      mIndex.read(pair.second.path, object, false);

      materials = object.getOrCreateChild("materials");
      materials.push(pair.first);
      object.put("materials", materials);
      object.put("modified", pair.second.modified);
      object.put("folder", pair.second.folder);

      mIndex.put(pair.second.path, object, false);
    }

    if(!dump(mIndex, indexPath, DataWrapper::JSON_OBJECT)) {
      LOG(ERROR) << "Failed to write material index file " << indexPath;
    }
  }

  bool MaterialLoader::onEnvUpdated(EventDispatcher* sender, const Event& event)
  {
    Engine* e = static_cast<Engine*>(sender);
    if(e->env().get("workdir", "") != mWorkdir) {
      reloadIndex();
    }

    return true;
  }

  void MaterialLoader::reloadIndex()
  {
    mWorkdir = mFacade->getEngine()->env().get("workdir", "");
    mMaterialIndex.clear();
    readIndexFile();
  }
}
