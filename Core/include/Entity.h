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

#ifndef _Entity_H_
#define _Entity_H_

#include <map>
#include <string>
#include <vector>

#include "Serializable.h"

namespace Gsage
{
  class Engine;
  class EntityComponent;

  class Entity
  {
    public:
      Entity();
      virtual ~Entity();
      /**
       * Add component handle to the map of components
       *
       * @param name Name of the system component belongs to
       * @param c Component
       */
      void addComponent(const std::string& name, EntityComponent* c);
      /**
       * Remove component
       *
       * @param name Name of the system component belongs to
       */
      bool removeComponent(const std::string& name);
      /**
       * Get component handle by component name
       *
       * @param name Name of the system component belongs to
       */
      EntityComponent* getComponent(const std::string& name);
      /**
       * Check that entity has all specified components
       *
       * @param components List of the systems
       */
      bool hasComponents(std::vector<std::string> components)
      {
        for(auto component : components)
        {
          if(!hasComponent(component))
            return false;
        }
        return true;
      }
      /**
       * Check that entity has specified component
       *
       * @param component Name of the system component belongs to
       */
      bool hasComponent(const std::string& component) const
      {
        return mComponents.count(component) != 0;
      }
      /**
       * Remove entity and all related components from the system
       *
       * @param name Name of the system component belongs to
       */
      EntityComponent* operator[](const std::string& name) { return getComponent(name); };

      /**
       * Get component of a specific type
       */
      template<class C>
      C* getComponent() {
        return static_cast<C*>(getComponent(C::SYSTEM));
      }

      /**
       * Get entity id
       */
      inline const std::string& getId() const { return mId; }
      /**
       * Adds flag to flag list
       *
       * @param flag String
       */
      void setFlag(const std::string& flag);
      /**
       * Check if entity has flag
       *
       * @param flag String 
       */
      bool hasFlag(const std::string& flag);

      /**
       * Get entity class used in EAL
       * @returns entity class
       */
      const std::string& getClass() const;

      /**
       * Set entity class for EAL
       * @param cls string class id
       */
      void setClass(const std::string& cls);

      /**
       * Get entity construction data
       */
      DataProxy getProps() const;

      /**
       * Get entity variables
       */
      DataProxy& getVars();

      /**
       * Set set entity variables
       * @param props DataProxy
       */
      void setVars(const DataProxy& value);

      /**
       * Get component names in a vector
       * @returns names vector
       */
      std::vector<std::string> getComponentNames() const;

      typedef std::map<const std::string, EntityComponent*> Components;
      typedef Components::const_iterator ComponentsIterator;

      // TODO: expose iterator only
      Components mComponents;
    private:
      friend class Engine;
      std::string mId;
      typedef std::vector<std::string> Flags;
      Flags mFlags;
      std::string mClass;
      DataProxy mVars;
  };
}

#endif
