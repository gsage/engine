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

#ifndef _OgreObjectManager_H_
#define _OgreObjectManager_H_

#include <map>

#include "GsageDefinitions.h"
#include "DataProxy.h"
#include "ObjectPool.h"
#include "Logger.h"
#include "EventDispatcher.h"

namespace Ogre
{
  class SceneManager;
  class SceneNode;
  class Entity;
}

namespace Gsage {
  class OgreObject;

  /**
   * Event related to factory lifecycle
   */
  class OgreObjectManagerEvent : public Event
  {
    public:
      OgreObjectManagerEvent(const std::string& type, const std::string& factoryId) : Event(type), mId(factoryId) {}
      virtual ~OgreObjectManagerEvent() {}

      /**
       * Factory was unregistered
       */
      static const std::string FACTORY_UNREGISTERED;

      /**
       * Get factory id that was unregistered
       */
      const std::string& getId() const { return mId; }
    private:
      std::string mId;
  };

  class OgreObjectManager : public EventDispatcher
  {
    public:

      OgreObjectManager();
      virtual ~OgreObjectManager();

      OgreObject* create(const DataProxy& dict, const std::string& owner, Ogre::SceneManager* sceneManager, const std::string& boneId, Ogre::Entity* parentEntity);

      /**
       * Create object from the DataProxy
       * @param dict DataProxy with all values (dict should contain type field)
       * @param owner Owner entity of the created object
       * @param sceneManager Ogre::SceneManager to create object in
       * @param parent Parent object to attach to
       * */
      OgreObject* create(const DataProxy& dict, const std::string& owner, Ogre::SceneManager* sceneManager, Ogre::SceneNode* parent = 0);

      /**
       * Create object from the DataProxy
       * @param dict DataProxy with all values
       * @param owner Owner entity of the created object
       * @param sceneManager Ogre::SceneManager to create object in
       * @param type Object type string, defined explicitly
       * @param parent Parent object to attach to
       */
      OgreObject* create(const DataProxy& dict, const std::string& owner, Ogre::SceneManager* sceneManager, const std::string& type, Ogre::SceneNode* parent = 0);

      /**
       * @see OgreObjectManager::create
       */
      template<typename C>
      C* create(const DataProxy& dict, const std::string& owner, Ogre::SceneManager* sceneManager, Ogre::SceneNode* parent = 0)
      {
        return static_cast<C*>(create(dict, owner, sceneManager, C::TYPE, parent));
      }

      /**
       * Destroy object
       * @param object Object to destroy
       */
      void destroy(OgreObject* object);

      /**
       * Register new element type, so factory will be able to create it
       * @param type String type representation
       */
      template<typename C>
      void registerElement(const std::string& type)
      {
        LOG(INFO) << "Registered factory for type \"" << type << "\"";
        mObjects[type] = new ConcreteOgreObjectPool<C>();
      }

      /**
       * Register new element type, so factory will be able to create id.
       * For this method, visual element should have static std::string TYPE defined
       */
      template<typename C>
      void registerElement()
      {
        registerElement<C>(C::TYPE);
      }

      /**
       * Unregister existing element type
       * It will destroy all existing objects of that type
       * @param type String type representation
       */
      bool unregisterElement(const std::string& type)
      {
        if(mObjects.count(type) == 0)
          return false; // no such type registered

        fireEvent(OgreObjectManagerEvent(OgreObjectManagerEvent::FACTORY_UNREGISTERED, type));
        delete mObjects[type];
        mObjects.erase(type);
        return true;
      }

      /**
       * Unregister existing element type
       * It will destroy all existing objects of that type
       */
      template<typename C>
      bool unregisterElement()
      {
        unregisterElement(C::TYPE);
      }
    private:
      class OgreObjectPool
      {
        public:
          virtual ~OgreObjectPool() {};
          virtual OgreObject* allocate() = 0;
          virtual void remove(OgreObject* object) = 0;
      };

      template<typename C>
      class ConcreteOgreObjectPool : public OgreObjectPool
      {
        public:
          virtual ~ConcreteOgreObjectPool()
          {
            clear();
          }

          OgreObject* allocate()
          {
            return mObjects.create();
          }

          void remove(OgreObject* object)
          {
            mObjects.erase(static_cast<C*>(object));
          }

          void clear()
          {
            for(C* object : mObjects.getElements())
            {
              object->destroy();
            }

            mObjects.clear();
          }
        private:
          ObjectPool<C> mObjects;
      };

      typedef std::map<const std::string, OgreObjectPool*> OgreObjectsCollections;

      OgreObjectsCollections mObjects;

  };
}
#endif
