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

#ifndef _Engine_H_
#define _Engine_H_

#define KEY_ID "id"
#define KEY_FLAGS "flags"

#include "EventDispatcher.h"
#include "GsageDefinitions.h"
#include "Entity.h"
#include "EngineSystem.h"

#include "ObjectPool.h"
#include <map>
#include <vector>
#include "DataProxy.h"

namespace Gsage
{
  class EntityComponent;
  class EngineSystem;

  /**
   * Core class of the game engine
   */
  class Engine : public EventDispatcher
  {
    public:
      typedef std::map<std::string, EngineSystem*> EngineSystems;
      typedef ObjectPool<Entity> Entities;
      Engine(const unsigned int& poolSize = ENTITY_POOL_SIZE);
      virtual ~Engine();
      /**
       * Initializes all the systems in the order in which it were added
       * @param configuration DataProxy with engine configuration
       * @param environment global application environment
       * @returns false if fails to set up one of them
       */
      bool initialize(const DataProxy& configuration, const DataProxy& environment);
      /**
       * Configure all systems in the engine
       * @param configuration DataProxy with system configs
       * @returns false if fails to configure one of the systems
       */
      bool configureSystems(const DataProxy& config);
      /**
       * Updates each system
       * @param time Delta time
       */
      void update(const double& time);
      /**
       * Add system to the engine
       */
      template<typename T>
      T* addSystem()
      {
        T* system = new T();
        if(addSystem(T::type::SYSTEM, system))
        {
          mManagedByEngine.push_back(T::type::SYSTEM);
        }
        else
        {
          delete system;
          return 0;
        }
        return system;
      }
      /**
       * Add system to the engine
       * @param name System name to associate with. Entity factory uses name to select proper system
       * @param system System that handles components
       */
      bool addSystem(const std::string& name, EngineSystem* system);
      /**
       * Check that engine has specified system
       * @param name System name
       */
      bool hasSystem(const std::string& name) { return mEngineSystems.count(name) != 0; }
      /**
       * Get system by name
       * @param name System name
       */
      EngineSystem* getSystem(const std::string& name);
      /**
       * Get system by name
       */
      template<typename C>
      C* getSystem()
      {
        EngineSystem* system = getSystem(C::type::SYSTEM);
        if(!system)
          return 0;

        std::string type = system->getSystemInfo().get("type", "");
        if(!type.empty() && type != C::ID) {
          return 0;
        }

        return static_cast<C*>(system);
      }
      /**
       * Remove system by name
       * @param name System name
       * @returns true if system was deleted, false otherwise
       */
      bool removeSystem(const std::string& name);
      /**
       * Remove all systems, that were create by the engine
       * Remove all active systems
       */
      void removeSystems();
      /**
       * Gets all engine systems
       */
      EngineSystems& getSystems();
      /**
       * Create entity from the entity data
       * @param data Deserialized data object
       */
      Entity* createEntity(DataProxy& data);
      /**
       * Remove entity by id
       *
       * @param id entity id
       */
      bool removeEntity(const std::string& id);
      /**
       * Remove entity by pointer
       *
       * @param entity Entity pointer
       */
      bool removeEntity(Entity* entity);
      /**
       * Destroy all entities
       */
      void unloadAll();
      /**
       * Get component by entity and name
       * @param entity Entity with all components
       * @param name Component name
       *
       * @deprecated
       */
      template<typename C> C* getComponent(Entity& entity, const std::string& name) { return static_cast<C*>(entity[name]); }
      /**
       * Get component by entity and name
       * @param entity Entity with all components
       * @param name Component name
       *
       * @deprecated
       */
      template<typename C> C* getComponent(Entity* entity, const std::string& name) { return getComponent<C>(*entity, name); }
      /**
       * Get component by entity and name
       * @param entity Entity with all components
       * @param name Component name
       */
      template<typename C> C* getComponent(Entity& entity) { return static_cast<C*>(entity[C::SYSTEM]); }
      /**
       * Get component by entity
       * @param entity Entity with all components
       * @param name Component name
       */
      template<typename C> C* getComponent(Entity* entity) { return getComponent<C>(*entity); }
      /**
       * Get component by entity
       * @param entity Entity with all components
       * @param name Component name
       */
      template<typename C> C* getComponent(const std::string& name)
      {
        Entity* e = getEntity(name);
        if(!e)
          return 0;

        return static_cast<C*>((*e)[C::SYSTEM]);
      }
      /**
       * Get component by entity
       * @param entity Entity with all components
       * @param name Component name
       */
      EntityComponent* getComponent(Entity* e, const std::string& name)
      {
        return (*e)[name];
      }
      /**
       * Get entity list
       */
      ObjectPool<Entity>::PointerVector getEntities() { return mEntities.getElements(); };
      /**
       * Get entity by id
       * @param id Entity id
       */
      Entity* getEntity(const std::string& id);

      /**
       * Get environment
       */
      const DataProxy& env() const { return mEnvironment; }

      /**
       * Get settings
       */
      const DataProxy& settings() const { return mConfiguration; }
    private:
      /**
       * Create component for entity
       *
       * @param entity Pointer to entity object
       * @param type System type to create into
       * @param node DataProxy with configs
       */
      bool createComponent(Entity* entity, const std::string& type, const DataProxy& node);
      /**
       * Update entity with new config
       *
       * @param entity Entity to update
       * @param node DataProxy with new config
       */
      bool readEntityData(Entity* entity, const DataProxy& node);

      bool mInitialized;

      EngineSystems mEngineSystems;
      Entities mEntities;
      unsigned long mEntityCounter;
      typedef std::map<const std::string, Entity*> EntityMap;
      EntityMap mEntityMap;

      typedef std::vector<std::string> SystemNames;
      SystemNames mSetUpOrder;
      SystemNames mManagedByEngine;

      DataProxy mConfiguration;
      DataProxy mEnvironment;
  };
}

#endif
