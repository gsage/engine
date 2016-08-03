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

#ifndef _ComponentStorage_H_
#define _ComponentStorage_H_

#include <mutex>

#include "EngineSystem.h"
#include "ObjectPool.h"

namespace Gsage
{
    class Entity;
    template<typename T>
    class ComponentStorage : public EngineSystem
    {
      public:
        typedef T type;

        ComponentStorage(const unsigned int& poolSize = COMPONENT_POOL_SIZE) : mComponents(poolSize) { };

        virtual ~ComponentStorage() {};

        /**
         * Does some manipulations before reading DataNode
         * @param component Component to prepare
         * @returns true if succeed
         */
        virtual bool prepareComponent(T* component)
        {
          return true;
        }

        /**
         * Create component in the storage
         * @param data node to get values from
         * @param owner Entity, that owns the instance
         */
        Component* createComponent(const DataNode& data, Entity* owner)
        {
          mComponentCreationMutex.lock();

          T* c = allocateComponent();
          c->setOwner(owner);
          if(!prepareComponent(c) || !c->read(data) || !fillComponentData(c, data))
          {
            removeComponent(c);
            return NULL; // failed to read data from node
          }

          mComponentCreationMutex.unlock();
          return c;
        }

        /**
         * Fill component data from data node
         * @param component Component to fill
         * @param data Data node
         *
         * @returns fill succeed
         */
        virtual bool fillComponentData(T* component, const DataNode& data) { return true; };
        /**
         * Remove component by base class
         * @param component Pointer to the component for removal
         */
        virtual bool removeComponent(Component* component)
        {
          mComponentRemovalMutex.lock();
          bool res = removeComponent((T*) component);
          mComponentRemovalMutex.unlock();
          return res;
        }
        /**
         * Removes component from the system
         * @param component Component pointer
         */
        virtual bool removeComponent(T* component)
        {
          mComponents.erase(component);
          return true;
        }

        /**
         * Clears components storage
         */
        virtual void unloadComponents()
        {
          typename ObjectPool<T>::PointerVector& elements = mComponents.getElements();
          while(elements.size() > 0)
          {
            removeComponent(elements.back());
          }
        }

        /**
         * Update each component in componentStorage
         * @param time Elapsed time
         */
        virtual void update(const double& time)
        {
          typename ObjectPool<T>::PointerVector components = mComponents.getElements();
          const int len = components.size();
          if(len == 0)
            return;

          if(mConfigDirty)
            configUpdated();

          for(int i = 0; i < len; ++i)
          {
            updateComponent(components[i], components[i]->getOwner(), time);
          }
        }
        /**
         * Update a single component
         */
        virtual void updateComponent(T* component, Entity* entity, const double& time) = 0;
        /**
         * Get component count
         */
        const long getComponentCount() { return mComponents.getElements().size(); }
      protected:
        typedef ObjectPool<T> Components;
        Components mComponents;

        T* allocateComponent()
        {
          return mComponents.create();
        }

        std::mutex mComponentCreationMutex;
        std::mutex mComponentRemovalMutex;
    };
}

#endif
