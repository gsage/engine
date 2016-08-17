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

#include "Engine.h"
#include "EngineSystem.h"

#include "Component.h"

#include "Logger.h"

using namespace Gsage;

Engine::Engine(const unsigned int& poolSize) :
  mEntities(poolSize),
  mInitialized(false)
{
}

Engine::~Engine()
{
  // delete only systems that were created by the engine itself
  removeSystems();
}

bool Engine::initialize(const DataNode& configuration)
{
  mConfiguration = configuration;

  bool succeed = true;
  for(auto& systemName : mSetUpOrder)
  {
    if(mEngineSystems[systemName]->isReady())
    {
      LOG(INFO) << "\"" << systemName << "\"" << " is already initialized, skipped";
      continue;
    }

    auto config = mConfiguration.get_child_optional(systemName);

    if(!mEngineSystems[systemName]->initialize(config ? config.get() : DataNode()))
    {
      LOG(ERROR) << "Failed to initialize system \"" << systemName << "\"";
      succeed = false;
      break;
    }
    else
    {
      LOG(INFO) << "Initialized system \"" << systemName << "\"";
    }
  }
  mInitialized = true;
  return succeed;
}

bool Engine::configureSystems(const DataNode& config)
{
  for(std::string& systemName : mSetUpOrder)
  {
    LOG(INFO) << "Configuring system " << systemName;
    auto systemConfig = config.get_child_optional(systemName);
    mEngineSystems[systemName]->configure(systemConfig ? systemConfig.get() : DataNode());
  }
}

void Engine::update(const double& time)
{
  for(auto& pair : mEngineSystems)
  {
    if(pair.second->isEnabled())
      pair.second->update(time);
  }
}

bool Engine::addSystem(const std::string& name, EngineSystem* system)
{
  if(mEngineSystems.count(name) != 0)
  {
    LOG(ERROR) << "Failed to add system \"" << name << "\": system with such id exists";
    return false;
  }

  mSetUpOrder.push_back(name);
  mEngineSystems[name] = system;
  system->setEngineInstance(this);
  if(mInitialized)
  {
    auto config = mConfiguration.get_child_optional(name);
    system->initialize(config ? config.get() : DataNode());
  }
  return true;
}

EngineSystem* Engine::getSystem(const std::string& name)
{
  if(!hasSystem(name))
    return 0;

  return mEngineSystems[name];
}

Engine::EngineSystems& Engine::getSystems()
{
  return mEngineSystems;
}

bool Engine::removeSystem(const std::string& name)
{
  if(!hasSystem(name))
    return false;

  SystemNames::iterator it = std::find(mManagedByEngine.begin(), mManagedByEngine.end(), name);
  if(it != mManagedByEngine.end())
    delete mEngineSystems[name];

  mEngineSystems.erase(name);
  return true;
}

void Engine::removeSystems()
{
  for(auto& name : mManagedByEngine)
  {
    delete mEngineSystems[name];
  }
  mEngineSystems.clear();
  mManagedByEngine.clear();
}

Entity* Engine::createEntity(const DataNode& data)
{
  // can't create entity with no id
  if(data.count(KEY_ID) == 0)
  {
    LOG(ERROR) << "Failed to create entity, id not present";
    return 0;
  }
  std::string id = data.get<std::string>(KEY_ID);

  Entity* entity = getEntity(id);
  if(!entity)
  {
    entity = mEntities.create();
    entity->mId = id;
    mEntityMap[id] = entity;
  }
  readEntityData(entity, data);
  return entity;
}

bool Engine::removeEntity(const std::string& id)
{
  return removeEntity(getEntity(id));
}

bool Engine::removeEntity(Entity* entity)
{
  if(entity == 0 || mEntityMap.count(entity->getId()) == 0)
    return false;
  for(auto& pair : entity->mComponents)
  {
    if(!hasSystem(pair.first))
      continue;

    if(!mEngineSystems[pair.first]->removeComponent(pair.second))
      LOG(WARNING) << "Got false return value while removing " << pair.first << " component";
  }

  mEntities.erase(entity);
  mEntityMap.erase(entity->getId());
  return true;
}

void Engine::unloadAll()
{
  for(auto& pair : mEngineSystems)
  {
    LOG(INFO) << "Unload components from system " << pair.first;
    pair.second->unloadComponents();
  }
  mEntities.clear();
  mEntityMap.clear();
}

Entity* Engine::getEntity(const std::string& id)
{
  if(mEntityMap.count(id) == 0)
    return 0;
  return mEntityMap[id];
}

bool Engine::createComponent(Entity* entity, const std::string& type, const DataNode& node)
{
  if(!hasSystem(type))
  {
    LOG(ERROR) << "Component for system \"" << type << "\" was not created: no such system";
    return false;
  }

  Component* component = getSystem(type)->createComponent(node, entity);
  if(component == 0)
  {
    LOG(ERROR) << "Component for system \"" << type << "\" was not created: read error";
    return false;
  }

  entity->addComponent(type, component);
}

bool Engine::readEntityData(Entity* entity, const DataNode& node)
{
  auto flags = node.get_child_optional("flags");
  if(flags)
  {
    for(auto& pair : flags.get())
    {
      entity->setFlag(pair.second.get_value<std::string>());
    }
  }

  for(auto& pair : node)
  {
    if(pair.first == KEY_ID || pair.first == KEY_FLAGS)
      continue;

    if(!entity->hasComponent(pair.first))
    {
      createComponent(entity, pair.first, pair.second);
    }
    else
    {
      Component* c = getComponent<Component>(entity, pair.first);
      if(!c)
      {
        LOG(ERROR) << "Failed to update component of system \"" << pair.first << "\". It exists in entity, but can not be resolved in any system";
        continue;
      }
      c->read(pair.second);
    }
  }
  return true;
}
