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

#include "GameDataManager.h"
#include <boost/regex.hpp>
#include "Engine.h"
#include "EngineSystem.h"
#include "Entity.h"
#include "Component.h"
#include "components/RenderComponent.h"

#include "Logger.h"

namespace Gsage {

  const std::string GameDataManager::CONFIG_SECTION = "dataManager";

  GameDataManager::GameDataManager(Engine* engine, const DataNode& config)
    : mEngine(engine)
    , mCurrentSaveFile(0)
  {
    mFileExtension    = config.get(CONFIG_SECTION + ".extension", "json");
    mCharactersFolder = config.get(CONFIG_SECTION + ".charactersFolder", ".");
    mLevelsFolder     = config.get(CONFIG_SECTION + ".levelsFolder", ".");
    mSavesFolder      = config.get(CONFIG_SECTION + ".savesFolder", ".");
  }

  GameDataManager::~GameDataManager()
  {
  }

  bool GameDataManager::initGame(const std::string& templateFile)
  {
    resetSaveFile();
    DataNode& root = getSaveFile();

    if(!parseFile(mSavesFolder + "/" + templateFile + "." + mFileExtension, root))
      return false;

    const std::string& area = root.get<std::string>("area");
    if(!loadArea(area))
    {
      LOG(ERROR) << "Failed to load area: " << area;
      return false;
    }

    loadCharacters(area);
    return true;
  }

  bool GameDataManager::loadSave(const std::string& saveFile)
  {
    resetSaveFile();
    DataNode& root = getSaveFile();

    if(!parseFile(saveFile + "." + mFileExtension, root))
      return initGame(saveFile);

    const std::string& area = root.get<std::string>("area");
    if(!loadArea(area))
    {
      LOG(ERROR) << "Failed to load area: " << area;
      return false;
    }

    auto placement = root.get_child_optional("placement." + area);
    if(placement)
    {
      for(auto& pair : placement.get())
      {
        Entity* e = addCharacter(pair.first);
        if(e)
        {
          RenderComponent* render = mEngine->getComponent<RenderComponent>(e);
          if(render)
            render->setPosition(pair.second.get<Ogre::Vector3>("position"));
        }
      }
    }

    return true;
  }

  bool GameDataManager::dumpSave(const std::string& saveFile)
  {
    const ObjectPool<Entity>::PointerVector& entities = mEngine->getEntities();
    DataNode saveData;
    DataNode entitiesNode;
    DataNode placementNode;

    std::string currentArea = getSaveFile().get("area", "none");
    saveData.put("area", currentArea);

    for(auto entity : entities)
    {
      if(!entity->hasFlag("dynamic"))
        continue;

      DataNode entityNode;
      for(auto pair : entity->mComponents)
      {
        Component* c = mEngine->getComponent<Component>(entity, pair.first);
        if(c)
        {
          entityNode.put_child(pair.first, c->getNode());
        }
      }
      entityNode.put("id", entity->getId());
      entitiesNode.put_child(entity->getId(), entityNode);

      RenderComponent* render = mEngine->getComponent<RenderComponent>(entity);
      if(render)
      {
        placementNode.put(entity->getId() + ".position", render->getPosition());
      }
    }

    DataNode settingsNode;
    Engine::EngineSystems& systems = mEngine->getSystems();
    for(auto pair : systems)
    {
      DataNode config = pair.second->getConfig();
      if(config.empty())
        continue;

      settingsNode.put_child(pair.first, config);
    }

    saveData.put_child("settings", settingsNode);
    saveData.put_child("characters", entitiesNode);
    saveData.put_child("placement." + currentArea, placementNode);
    return dumpFile(saveFile + "." + mFileExtension, saveData);
  }

  Entity* GameDataManager::createEntity(const std::string& json)
  {
    DataNode dest;
    std::stringstream stream(json);
    try
    {
      boost::property_tree::read_json(stream, dest);
    }
    catch(boost::property_tree::json_parser_error& ex)
    {
      LOG(ERROR) << "Failed to read json: " << json << ", reason: " << ex.message();
      return 0;
    }

    return mEngine->createEntity(dest);
  }

  Entity* GameDataManager::createEntity(const std::string& name, const TemplateParameters& params)
  {
    std::string json = readFile(name);
    if(json.empty())
    {
      LOG(WARNING) << "Template " << name << " is empty";
      return 0;
    }

    for(auto pair : params)
    {
      std::string pattern = "%" + pair.first + "%";

      LOG(INFO) << pattern << " replace with " << pair.second;
      json = boost::regex_replace(json, boost::regex(pattern), pair.second);
    }

    return createEntity(json);
  }

  bool GameDataManager::loadArea(const std::string& area)
  {
    DataNode areaInfo;
    if(!parseFile(mLevelsFolder + "/" + area + "." + mFileExtension, areaInfo))
      return false;

    auto entities = areaInfo.get_child_optional("entities");
    if(!entities)
    {
      LOG(ERROR) << "No entities in area: " << area;
      return false;
    }

    auto systemsConfigs = getSaveFile().get_child_optional("settings");
    if(!systemsConfigs)
      systemsConfigs = areaInfo.get_child_optional("settings");

    if(systemsConfigs)
    {
      mEngine->configureSystems(systemsConfigs.get());
    }

    for(auto& element : entities.get())
    {
      mEngine->createEntity(element.second);
    }
    return true;
  }

  Entity* GameDataManager::addCharacter(const std::string& name)
  {
    DataNode entityNode;
    DataNode& saveFile = getSaveFile();
    auto entityNodeOpt = saveFile.get_child_optional("characters." + name);

    if(entityNodeOpt)
    {
      entityNode = entityNodeOpt.get();
    }
    else if(parseFile(mCharactersFolder + "/" + name + "." + mFileExtension, entityNode))
    {
      LOG(INFO) << "Creating new character";
    }
    else
    {
      LOG(ERROR) << "Failed to create character: " << name << " not found in db";
      return 0;
    }

    Entity* e = mEngine->createEntity(entityNode);
    e->setFlag("dynamic");
    return e;
  }

  bool GameDataManager::loadCharacters(const std::string& area)
  {
    DataNode charactersIndex;
    if(!parseFile(mLevelsFolder + "/placement." + mFileExtension, charactersIndex))
      return false;

    auto locationCharacters = charactersIndex.get_child_optional(area);
    // no charactes on specified area
    if(!locationCharacters)
      return true;

    std::string entityId;
    for(auto& node : locationCharacters.get())
    {
      entityId = node.second.get_value<std::string>();
      addCharacter(entityId);
    }

    return true;
  }

  const std::string GameDataManager::readFile(const std::string& path)
  {
    std::ifstream stream(path);
    std::string res;
    stream.seekg(0, std::ios::end);
    res.reserve(stream.tellg());
    stream.seekg(0, std::ios::beg);
    try
    {
      res.assign((std::istreambuf_iterator<char>(stream)),
                  std::istreambuf_iterator<char>());
    }
    catch(...)
    {
      LOG(ERROR) << "Failed to read file: " << path;
    }

    stream.close();

    return res;
  }

  DataNode& GameDataManager::getSaveFile()
  {
    if(mCurrentSaveFile == 0)
      mCurrentSaveFile = new DataNode();

    return *mCurrentSaveFile;
  }

  void GameDataManager::resetSaveFile()
  {
    delete mCurrentSaveFile;
    mCurrentSaveFile = 0;
  }
}
