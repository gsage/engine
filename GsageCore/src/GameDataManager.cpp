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
#include "Engine.h"
#include "EngineSystem.h"
#include "Entity.h"
#include "Component.h"
#include "FileLoader.h"
#include "components/RenderComponent.h"
#include "GeometryPrimitives.h"

#include "Logger.h"

namespace Gsage {

  const std::string GameDataManager::CONFIG_SECTION = "dataManager";

  GameDataManager::GameDataManager(Engine* engine, const DataProxy& config)
    : mEngine(engine)
    , mCurrentSaveFile(0)
  {
    std::string workdir = engine->env().get("workdir", ".");
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
    DataProxy& root = getSaveFile();

    if(!FileLoader::getSingletonPtr()->load(mSavesFolder + GSAGE_PATH_SEPARATOR + templateFile + "." + mFileExtension, DataProxy(), root))
      return false;

    const std::string& area = root.get("area", "none");
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
    DataProxy& root = getSaveFile();

    if(!FileLoader::getSingletonPtr()->load(saveFile + "." + mFileExtension, DataProxy(), root))
      return initGame(saveFile);

    const std::string& area = root.get<std::string>("area", "none");
    if(!loadArea(area))
    {
      LOG(ERROR) << "Failed to load area: " << area;
      return false;
    }

    auto placement = root.get<DataProxy>("placement." + area);
    if(placement.second)
    {
      for(auto& pair : placement.first)
      {
        // TODO figure out more flexible way to do it
        DataProxy params;
        params.put("render.root.position", pair.second.get("position", "0,0,0"));
        Entity* e = addCharacter(pair.first, &params);
      }
    }

    return true;
  }

  bool GameDataManager::dumpSave(const std::string& saveFile)
  {
    const ObjectPool<Entity>::PointerVector& entities = mEngine->getEntities();
    DataProxy saveData;
    DataProxy entitiesNode;
    DataProxy placementNode;

    std::string currentArea = getSaveFile().get("area", "none");
    saveData.put("area", currentArea);

    for(auto entity : entities)
    {
      if(!entity->hasFlag("dynamic"))
        continue;

      entitiesNode.put(entity->getId(), getEntityData(entity->getId()));

      RenderComponent* render = mEngine->getComponent<RenderComponent>(entity);

      if(render)
      {
        Gsage::Vector3 position = render->getPosition();
        std::stringstream ss;
        ss << position.X << "," << position.Y << "," << position.Z;
        placementNode.put(entity->getId() + ".position", ss.str());
      }
    }

    DataProxy settingsNode;
    Engine::EngineSystems& systems = mEngine->getSystems();
    for(auto pair : systems)
    {
      DataProxy config = pair.second->getConfig();
      if(config.empty())
        continue;

      settingsNode.put(pair.first, config);
    }

    saveData.put("settings", settingsNode);
    saveData.put("characters", entitiesNode);
    saveData.put("placement." + currentArea, placementNode);
    FileLoader::getSingletonPtr()->dump(saveFile + "." + mFileExtension, saveData);
    return true;
  }

  bool GameDataManager::removeEntity(const std::string& id)
  {
    return mEngine->removeEntity(id);
  }

  DataProxy GameDataManager::getEntityData(const std::string& id)
  {
    DataProxy entityNode;
    Entity* entity = mEngine->getEntity(id);
    if(!entity) {
      return entityNode;
    }

    for(auto pair : entity->mComponents)
    {
      EntityComponent* c = mEngine->getComponent(entity, pair.first);
      if(c)
      {
        entityNode.put(pair.first, c->getNode());
      }
    }
    entityNode.put("id", entity->getId());
    entityNode.put("class", entity->getClass());
    entityNode.put("props", entity->getProps());
    return entityNode;
  }

  Entity* GameDataManager::createEntity(const std::string& json)
  {
    DataProxy dest = DataProxy::create(DataWrapper::JSON_OBJECT);
    if(!loads(dest, json, DataWrapper::JSON_OBJECT)) {
      LOG(ERROR) << "Failed to read json: " << json;
      return 0;
    }
    return mEngine->createEntity(dest);
  }

  Entity* GameDataManager::createEntity(const std::string& name, const DataProxy& params)
  {
    DataProxy dest;
    if(!FileLoader::getSingletonPtr()->load(name, params, dest))
    {
      return 0;
    }
    return mEngine->createEntity(dest);
  }

  Entity* GameDataManager::createEntity(DataProxy data)
  {
    return mEngine->createEntity(data);
  }

  bool GameDataManager::loadArea(const std::string& area)
  {
    DataProxy areaInfo;
    std::string path = mLevelsFolder + "/" + area + "." + mFileExtension;
    LOG(INFO) << "Loading area " << path;
    if(!FileLoader::getSingletonPtr()->load(path, DataProxy(), areaInfo))
      return false;

    auto entities = areaInfo.get<DataProxy>("entities");
    if(!entities.second)
    {
      LOG(ERROR) << "No entities in area: " << area;
      return false;
    }

    auto systemsConfigs = getSaveFile().get<DataProxy>("settings");
    if(!systemsConfigs.second)
      systemsConfigs = areaInfo.get<DataProxy>("settings");

    if(systemsConfigs.second)
    {
      mEngine->configureSystems(systemsConfigs.first);
    }

    for(auto& element : entities.first)
    {
      mEngine->createEntity(element.second);
    }
    return true;
  }

  Entity* GameDataManager::addCharacter(const std::string& name, DataProxy* params)
  {
    DataProxy entityNode;
    DataProxy& saveFile = getSaveFile();
    auto entityNodeOpt = saveFile.get<DataProxy>("characters." + name);

    if(entityNodeOpt.second)
    {
      entityNode = entityNodeOpt.first;
    }
    else if(FileLoader::getSingletonPtr()->load(mCharactersFolder + "/" + name + "." + mFileExtension, DataProxy(), entityNode))
    {
      LOG(INFO) << "Creating new character";
    }
    else
    {
      LOG(ERROR) << "Failed to create character: " << name << " not found in db";
      return 0;
    }

    if(params) {
      mergeInto(entityNode, *params);
    }

    Entity* e = mEngine->createEntity(entityNode);
    e->setFlag("dynamic");
    return e;
  }

  bool GameDataManager::loadCharacters(const std::string& area)
  {
    DataProxy charactersIndex;
    if(!FileLoader::getSingletonPtr()->load(mLevelsFolder + "/placement." + mFileExtension, DataProxy(), charactersIndex))
      return false;

    auto locationCharacters = charactersIndex.get<DataProxy>(area);
    // no charactes on specified area
    if(!locationCharacters.second)
      return true;

    std::string entityId;
    for(auto& node : locationCharacters.first)
    {
      entityId = node.second.as<std::string>();
      addCharacter(entityId);
    }

    return true;
  }

  DataProxy& GameDataManager::getSaveFile()
  {
    if(mCurrentSaveFile == 0)
      mCurrentSaveFile = new DataProxy();

    return *mCurrentSaveFile;
  }

  void GameDataManager::resetSaveFile()
  {
    delete mCurrentSaveFile;
    mCurrentSaveFile = 0;
  }
}
