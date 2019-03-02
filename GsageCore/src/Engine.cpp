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

#include "Component.h"

#include "Logger.h"

#include <chrono>

namespace Gsage
{
  Engine::Engine(const unsigned int& poolSize) :
    mEntities(poolSize),
    mInitialized(false),
    mEntityCounter(0)
  {
  }

  Engine::~Engine()
  {
    // delete only systems that were created by the engine itself
    removeSystems();
  }

  bool Engine::initialize(const DataProxy& configuration, const DataProxy& environment)
  {
    mConfiguration = configuration;
    mEnvironment = environment;

    bool succeed = true;
    for(auto& systemName : mSetUpOrder)
    {
      if(mEngineSystems[systemName]->isReady())
      {
        LOG(INFO) << "\"" << systemName << "\"" << " is already initialized, skipped";
        continue;
      }

      if(!configureSystem(systemName)) {
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

  bool Engine::configureSystems(const DataProxy& config)
  {
    mergeInto(mConfiguration, config);
    for(std::string& systemName : mSetUpOrder)
    {
      LOG(INFO) << "Configuring system " << systemName;
      auto systemConfig = mConfiguration.get<DataProxy>(systemName);
      mEngineSystems[systemName]->configure(systemConfig.second ? systemConfig.first : DataProxy());
    }

    fireEvent(SettingsEvent(SettingsEvent::UPDATE, mConfiguration));
    return true;
  }

  void Engine::update(const double& time)
  {
    fireEvent(EngineEvent(EngineEvent::UPDATE));
    for(auto& pair : mEngineSystems)
    {
      if(!pair.second->isEnabled() || !pair.second->isReady())
        continue;

      if(pair.second->dedicatedThread()) {
        if(pair.second->becameReady()) {
          fireEvent(SystemChangeEvent(SystemChangeEvent::SYSTEM_ADDED, pair.first, pair.second));
        }
      } else {
        pair.second->update(time);
      }

      for(int i = 0; i < mMainThreadCallbacks.size(); ++i) {
        QueuedCallbackPtr cb;
        mMainThreadCallbacks.get(cb);
        cb->execute();
      }

      if(pair.second->needsRestart()) {
        LOG(INFO) << "Restarting system " << pair.first;
        pair.second->shutdown();
        pair.second->initialize(pair.second->getConfig());
      }
    }
  }

  bool Engine::addSystem(const std::string& name, EngineSystem* system, bool configure)
  {
    system->setName(name);
    if(mEngineSystems.count(name) != 0)
    {
      LOG(ERROR) << "Failed to add system \"" << name << "\": system with such id exists";
      return false;
    }

    mSetUpOrder.push_back(name);
    mEngineSystems[name] = system;
    system->setEngineInstance(this);
    if(mInitialized && configure)
    {
      if(!configureSystem(name)) {
        LOG(ERROR) << "Failed to setup " << name << " system";
        return false;
      }
    }

    return true;
  }

  bool Engine::configureSystem(const std::string& name) {
    return configureSystem(name, false);
  }

  bool Engine::configureSystem(const std::string& name, bool restart) {
    EngineSystem* s = getSystem(name);
    if(!s) {
      return false;
    }

    bool firstSetup = !s->isReady();

    DataProxy config;
    mConfiguration.read(name, config);

    bool dedicatedThread = config.get("dedicatedThread", false) && s->allowMultithreading();
    int numThreads = config.get("numThreads", 1);

    if(firstSetup && dedicatedThread) {
      // spawn n system threads
      for(int i = 0; i < numThreads; i++) {
        std::stringstream ss;
        ss << name << i;
        const std::string workerName = ss.str();
        mWorkers[workerName] = std::make_unique<SystemWorker>(s);
        LOG(INFO) << "Spawn system worker " << ss.str();
        mWorkers[workerName]->mSystemConfig = config;
        mWorkers[workerName]->start();
      }
    } else {
      if(!s->isReady() && !s->initialize(config)) {
        return false;
      }

      if(restart) {
        s->setConfig(config);
        s->restart();
      } else {
        bool res = s->configure(config);
        if(!res) {
          return false;
        }
      }
    }

    if(firstSetup && !dedicatedThread) {
      fireEvent(SystemChangeEvent(SystemChangeEvent::SYSTEM_ADDED, name, s));
    }
    return true;
  }

  bool Engine::configureSystem(const std::string& name, const DataProxy& config, bool restart) {
    mConfiguration.put(name, config);
    return configureSystem(name, restart);
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

    fireEvent(SystemChangeEvent(SystemChangeEvent::SYSTEM_REMOVED, name, getSystem(name)));

    SystemNames::iterator it = std::find(mManagedByEngine.begin(), mManagedByEngine.end(), name);
    if(it != mManagedByEngine.end()) {
      mManagedByEngine.erase(it);
      delete mEngineSystems[name];
    }

    mEngineSystems.erase(name);
    return true;
  }

  void Engine::removeSystems()
  {
    for(auto& pair : mEngineSystems) {
      fireEvent(SystemChangeEvent(SystemChangeEvent::SYSTEM_REMOVED, pair.first, pair.second));
    }

    for(auto& name : mManagedByEngine) {
      delete mEngineSystems[name];
    }

    mEngineSystems.clear();
    mManagedByEngine.clear();
  }

  Entity* Engine::createEntity(DataProxy& data)
  {
    // can't create entity with no id
    if(data.count(KEY_ID) == 0)
    {
      // autogenerate id
      data.put(KEY_ID, std::string("entity") + std::to_string(mEntityCounter++));
    }
    std::string id = data.get<std::string>(KEY_ID).first;

    Entity* entity = getEntity(id);
    bool created = false;
    if(!entity)
    {
      entity = mEntities.create();
      entity->mId = id;
      created = true;
    }
    entity->setClass(data.get<std::string>("class", "default"));
    auto pair = data.get<DataProxy>("props");
    if(pair.second) {
      entity->setProps(pair.first);
    }
    mEntityMap[id] = entity;
    readEntityData(entity, data);
    if(created) {
      fireEvent(EntityEvent(EntityEvent::CREATE, entity->getId()));
    }
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
    fireEvent(EntityEvent(EntityEvent::REMOVE, entity->getId()));
    for(auto& pair : entity->mComponents)
    {
      if(!hasSystem(pair.first))
        continue;

      if(!mEngineSystems[pair.first]->removeComponent(pair.second))
        LOG(WARNING) << "Got false return value while removing " << pair.first << " component";
    }

    mEntityMap.erase(entity->getId());
    mEntities.erase(entity);
    return true;
  }

  void Engine::unloadAll()
  {
    for(auto pair : mEntityMap) {
      fireEvent(EntityEvent(EntityEvent::REMOVE, pair.first));
    }

    for(auto& pair : mEngineSystems)
    {
      LOG(INFO) << "Unload components from system " << pair.first;
      pair.second->unloadComponents();
    }
    mEntities.clear();
    mEntityMap.clear();
  }

  void Engine::unloadMatching(UnloadMatcherFunc f)
  {
    std::vector<Entity*> removed;

    for(auto pair : mEntityMap) {
      if(f(pair.second)) {
        fireEvent(EntityEvent(EntityEvent::REMOVE, pair.second->getId()));

        removed.push_back(pair.second);
        for(auto& p : pair.second->mComponents)
        {
          if(!hasSystem(p.first))
            continue;

          if(!mEngineSystems[p.first]->removeComponent(p.second)) {
            LOG(WARNING) << "Got false return value while removing " << p.first << " component";
          }
        }
      }
    }

    for(auto entity : removed) {
      LOG(INFO) << "Removed entity \"" << entity->getId() << "\"";
      mEntityMap.erase(entity->getId());
      mEntities.erase(entity);
    }
  }

  Entity* Engine::getEntity(const std::string& id)
  {
    if(mEntityMap.count(id) == 0)
      return nullptr;
    return mEntityMap[id];
  }

  void Engine::shutdown(bool terminate)
  {
    shutdownThreads(terminate);
  }

  Engine::QueuedCallbackPtr Engine::executeInMainThread(MainThreadCallback func)
  {
    QueuedCallbackPtr qcb = std::make_shared<QueuedCallback>(func);
    mMainThreadCallbacks << qcb;
    return qcb;
  }

  bool Engine::createComponent(Entity* entity, const std::string& type, const DataProxy& dict)
  {
    if(!hasSystem(type))
    {
      LOG(ERROR) << "Component for system \"" << type << "\" was not created: no such system";
      return false;
    }

    EntityComponent* component = getSystem(type)->createComponent(dict, entity);
    if(component == 0)
    {
      LOG(ERROR) << "Component for system \"" << type << "\" was not created: read error";
      return false;
    }

    entity->addComponent(type, component);
    return true;
  }

  bool Engine::readEntityData(Entity* entity, const DataProxy& dict)
  {
    auto flags = dict.get<DataProxy>("flags");
    if(flags.second)
    {
      for(auto& pair : flags.first)
      {
        entity->setFlag(pair.second.as<std::string>());
      }
    }

    for(auto& pair : dict)
    {
      if(pair.first == KEY_ID || pair.first == KEY_FLAGS ||  pair.first == "class" || pair.first == "props")
        continue;

      if(!entity->hasComponent(pair.first))
      {
        createComponent(entity, pair.first, pair.second);
      }
      else
      {
        EntityComponent* c = getComponent<EntityComponent>(entity, pair.first);
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

  void Engine::shutdownThreads(bool terminate)
  {
    for(auto& pair : mWorkers) {
      LOG(INFO) << (terminate ? "Terminating" : "Stopping") << " worker " << pair.first;
      pair.second->shutdown(terminate);
    }
  }

  Engine::SystemWorker::SystemWorker(EngineSystem* system)
    : mSystem(system)
    , mShutdown(false)
    , mPreviousUpdateTime(std::chrono::high_resolution_clock::now())
    , mThread(0)
  {
  }

  Engine::SystemWorker::~SystemWorker()
  {
    if(!mShutdown) {
      shutdown(true);
    }
  }

  void Engine::SystemWorker::start()
  {
    if(mThread != 0) {
      LOG(ERROR) << "Tried to start started worker";
      return;
    }
    mThread = new std::thread(&Engine::SystemWorker::run, this);
  }

  void Engine::SystemWorker::shutdown(bool terminate)
  {
    mShutdown = true;
    if(terminate) {
      // TODO handle forcefull termination logic
    }

    if(mThread->joinable()){
      mThread->join();
    }
  }

  void Engine::SystemWorker::run()
  {
    if(!mSystem->isReady()) {
      if(!mSystem->initialize(mSystemConfig)) {
        LOG(INFO) << "Failed to initialize system " << mSystem->getName() << ", worker stopped";
        return;
      }
    }

    while (!mShutdown) {
      auto now = std::chrono::high_resolution_clock::now();
      double frameTime = std::chrono::duration_cast<std::chrono::duration<double>>(now - mPreviousUpdateTime).count();
      mSystem->update(frameTime);
      mPreviousUpdateTime = now;
    }

    mSystem->shutdown();
  }
}
