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

#ifndef __SERIALIZABLE_H__
#define __SERIALIZABLE_H__

#include "GsageDefinitions.h"
#include "DataProxy.h"
#include <map>

/**
 * Bind member field
 *
 * @param name String id to read from/dump to
 * @param property Pointer to the member property
 */
#define BIND_PROPERTY(name, property) registerProperty(name, property)
/**
 * Bind member field with priority
 *
 * @param name String id to read from/dump to
 * @param property Pointer to the member property
 * @param priority Int priority value, 0 is the highest
 */
#define BIND_PROPERTY_WITH_PRIORITY(name, property, priority) registerProperty(name, property, 0, priority)
/**
 * Bind member field with optional flag. Gsage::Serializable::read will return true, if this field was not found
 *
 * @param name String id to read from/dump to
 * @param property Pointer to the member property
 */
#define BIND_PROPERTY_OPTIONAL(name, property) registerProperty(name, property, Optional)

/**
 * Bind using setter and getter
 *
 * @param name String id to read from/dump to
 * @param setter Setter function
 * @param getter Getter function
 */
#define BIND_ACCESSOR(name, setter, getter) registerProperty(name, this, setter, getter)

/**
 * Bind using setter and getter with priority
 *
 * @param name String id to read from/dump to
 * @param setter Setter function
 * @param getter Getter function
 * @param priority Int priority value, 0 is the highest
 */
#define BIND_ACCESSOR_WITH_PRIORITY(name, setter, getter, priority) registerProperty(name, this, setter, getter, 0, priority)

/**
 * Bind using setter and getter with optional flag. Gsage::Serializable::read will return true, if this field was not found
 *
 * @param name String id to read from/dump to
 * @param setter Setter function
 * @param getter Getter function
 */
#define BIND_ACCESSOR_OPTIONAL(name, setter, getter) registerProperty(name, this, setter, getter, Optional)

/**
 * Bind using setter. Tries to read value only
 *
 * @param name String id to read from/dump to
 * @param setter Setter function
 */
#define BIND_SETTER_OPTIONAL(name, setter) registerSetter(name, this, setter, Optional)

/**
 * Bind using getter. Property will be dumped only
 *
 * @param name String id to read from/dump to
 * @param getter Getter function
 */
#define BIND_GETTER(name, getter) registerGetter(name, this, getter)

/**
 * Bind using getter. Property will be dumped only. With Optional flag
 *
 * @param name String id to read from/dump to
 * @param getter Getter function
 */
#define BIND_GETTER_OPTIONAL(name, getter) registerGetter(name, this, getter, Optional)

/**
 * Bind member field read only
 *
 * @param name String id to read from/dump to
 * @param property Pointer to the member property
 */
#define BIND_READONLY_PROPERTY(name, property) registerProperty(name, property, Readonly)

/**
 * Bind member field write only
 *
 * @param name String id to read from/dump to
 * @param property Pointer to the member property
 */
#define BIND_WRITEONLY_PROPERTY(name, property) registerProperty(name, property, Writeonly)

enum PropertyFlag
{
  Readonly = 0x01,
  Writeonly = 0x02,
  Optional = 0x04,
};

namespace Gsage
{

  /**
   * Class Reflection Mixin
   */
  class Reflection
  {
    public:
      /**
       * Get all class props
       */
      virtual DataProxy getProps() = 0;

      /**
       * Set all class props
       *
       * @param props DataProxy
       */
      virtual bool setProps(const DataProxy& props) = 0;
  };

  template<typename Type>
  inline bool get(const DataProxy& dict, const std::string& id, Type& dest)
  {
    auto value = dict.get<Type>(id);
    if(!value.second)
      return false;

    dest = value.first;
    return true;
  }

  inline bool get(const DataProxy& dict, std::string& id, DataProxy& dest)
  {
    auto value = dict.get<DataProxy>(id);
    if(!value.second)
      return false;

    dest = value.first;
    return true;
  }

  template<typename Type>
  inline bool put(DataProxy& dict, const std::string& id, const Type& value)
  {
    dict.put(id, value);
    return true;
  }

  inline bool put(DataProxy& dict, const std::string& id, const DataProxy& child)
  {
    if(child.empty())
      return true;
    dict.put(id, child);
    return true;
  }

  inline bool put(DataProxy& dict, const std::string& id, const std::string& value)
  {
    if(value.empty())
      return true;
    dict.put(id, value);
    return true;
  }

  /**
   * Class that has bindings for quick reading fields from DataProxy and writing it to it
   */
  template<typename C>
  class Serializable : public Reflection
  {
    public:
      /**
       * Non templated property base
       * Used to store all props in the vector
       */
      class AbstractProperty
      {
        public:
          AbstractProperty(const std::string& name, const int flags)
            : mName(name)
            , mFlags(flags)
          {};
          virtual ~AbstractProperty() {};
          /**
           * Read property from DataProxy
           * @param dict DataProxy
           */
          virtual bool read(const DataProxy& dict) = 0;
          /**
           * Write property to the DataProxy
           * @param dict DataProxy
           */
          virtual bool dump(DataProxy& dict) = 0;

          std::string mName;
          bool isFlagSet(const PropertyFlag& flag)
          {
            return (mFlags & flag) == flag;
          }

        protected:
          int mFlags;
      };

      /**
       * Wrapper for any field of class
       */
      template<typename T>
      class Property : public AbstractProperty
      {
        public:
          Property(std::string name, T* propertyPtr, int flags = 0x00) 
            : mPropertyPtr(propertyPtr)
            , AbstractProperty(name, flags)
          {
          }
          /**
           * Read property value from the node
           * @param dict Should contain value with key, that is defined in constructor of object
           */
          bool read(const DataProxy& dict)
          {
            if(AbstractProperty::isFlagSet(Readonly) || AbstractProperty::isFlagSet(Optional))
              return true;

            return get(dict, AbstractProperty::mName, *mPropertyPtr);
          }

          /**
           * Write property to the data node
           * @param dict DataProxy will contain value with specified key
           */
          bool dump(DataProxy& dict)
          {
            if(AbstractProperty::isFlagSet(Writeonly))
              return true;

            if(mPropertyPtr == NULL)
              return false;

            return put(dict, AbstractProperty::mName, *mPropertyPtr);
          }
        private:
          std::string mName;
          T* mPropertyPtr;
      };

      /**
       * Wrapper for property setter and getter
       */
      template<typename T, typename TSetter, typename TGetter, class TInstance = C>
      class PropertyAccessor : public AbstractProperty
      {
        public:
          PropertyAccessor(std::string name, TInstance* instance, TGetter getter, TSetter setter, int flags)
            : mInstance(instance)
            , mGetter(getter)
            , mSetter(setter)
            , AbstractProperty(name, flags)
          {
          }
          /**
           * Read property value from the node and call class setter
           * @param dict Should contain value with key, that is defined in constructor of object
           */
          bool read(const DataProxy& dict)
          {
            T value;
            // setter is not set, it is normal
            if(mSetter == 0)
              return true;
            if(!get(dict, AbstractProperty::mName, value))
              return AbstractProperty::isFlagSet(Optional);

            (mInstance->*mSetter)(value);
            return true;
          }

          /**
           * Call class getter and write return value to the data node
           * @param dict DataProxy will contain value with specified key
           */
          bool dump(DataProxy& dict)
          {
            if(mGetter == 0)
              return true;

            return put(dict, AbstractProperty::mName, (mInstance->*mGetter)());
          }
        private:
          TInstance* mInstance;
          TGetter mGetter;
          TSetter mSetter;
      };

      virtual ~Serializable()
      {
        for(auto pair : mProperties) {
          for(unsigned int i = 0; i < pair.second.size(); i++)
          {
            AbstractProperty* p = pair.second[i];
            delete p;
          }
          pair.second.clear();
        }
        mProperties.clear();
      }

      /**
       * Read a particular property
       * @param dict To read property from
       * @param id Property id
       */
      virtual bool read(const DataProxy& dict, const std::string& id)
      {
        if(mPropMappings.count(id) == 0)
          return false;

        return mPropMappings[id]->read(dict);
      }

      /**
       * Iterates through all specified properties and reads each from the node
       * @param dict DataProxy to read
       */
      virtual bool read(const DataProxy& dict)
      {
        bool allSucceed = true;
        for(auto pair : mProperties) {
          for(AbstractProperty* prop : pair.second)
          {
            if(!prop->read(dict))
            {
              allSucceed = false;
            }
          }
        }
        return allSucceed;
      }

      /**
       * Iterates through all specified properties and puts each to the node
       * @param dict DataProxy to write
       */
      virtual bool dump(DataProxy& dict)
      {
        bool allSucceed = true;
        for(auto pair : mProperties) {
          for(AbstractProperty* prop : pair.second)
          {
            if(!prop->dump(dict))
              allSucceed = false;
          }
        }
        return allSucceed;
      }

      /**
       * Get DataProxy with all properties
       *
       * @return DataProxy
       */
      virtual DataProxy getProps()
      {
        DataProxy dp;
        dump(dp);
        return dp;
      }

      /**
       * Set object properties
       *
       * @param props DataProxy
       */
      virtual bool setProps(const DataProxy& props)
      {
        return read(props);
      }

      /**
       * Register property as serializable
       * @param name Key to search in DataProxy
       * @param dest Pointer to field to wrap
       * @param flags property flags
       */
      template<typename TDest>
      void registerProperty(const std::string& name, TDest* dest, int flags = 0x00, int priority = 0)
      {
        addProperty(new Property<TDest>(name, dest, flags), priority);
      }

      /**
       * Register property setter/getter as serializable.
       * This method can be used with any class, not self only.
       * @param name Key to search in DataProxy
       * @param instance Instance of object, that contains getters and setters
       * @param setter Property setter
       * @param getter Property getter
       */
      template<typename TDest, class TInstance, class TRetVal>
      void registerProperty(const std::string& name, TInstance* instance, TRetVal (TInstance::*setter)(const TDest& value), TDest (TInstance::*getter)(), int flags = 0x00, int priority = 0)
      {
        addProperty(new PropertyAccessor<TDest, TRetVal (TInstance::*)(const TDest& value), TDest (TInstance::*)(), TInstance>(name, instance, getter, setter, flags), priority);
      }
      /**
       * Register property setter/getter as serializable.
       * This method can be used with any class, not self only.
       * @param name Key to search in DataProxy
       * @param instance Instance of object, that contains getters and setters
       * @param setter Property setter
       * @param getter Property getter
       */
      template<typename TDest, class TInstance, class TRetVal>
      void registerProperty(const std::string& name, TInstance* instance, TRetVal (TInstance::*setter)(const TDest& value), const TDest& (TInstance::*getter)()const, int flags = 0x00, int priority = 0)
      {
        addProperty(new PropertyAccessor<TDest, TRetVal (TInstance::*)(const TDest& value), const TDest& (TInstance::*)()const, TInstance>(name, instance, getter, setter, flags), priority);
      }
      /**
       * Register property getter as serializable.
       * This method can be used with any class, not self only.
       * @param name Key to search in DataProxy
       * @param instance Instance of object, that contains getters and setters
       * @param getter Property getter
       */
      template<typename TDest, class TInstance>
      void registerGetter(const std::string& name, TInstance* instance, TDest (TInstance::*getter)(), int flags = 0x00, int priority = 0)
      {
        addProperty(new PropertyAccessor<TDest, void (TInstance::*)(const TDest& value), TDest (TInstance::*)(), TInstance>(name, instance, getter, 0, flags), priority);
      }
      /**
       * Register property setter as serializable.
       * This method can be used with any class, not self only.
       *
       * @param name Key to search in DataProxy
       * @param instance Instance of object, that contains getters and setters
       * @param getter Property getter
       */
      template<typename TDest, class TInstance, class TRetVal>
      void registerSetter(const std::string& name, TInstance* instance, TRetVal (TInstance::*setter)(const TDest& value), int flags = 0x00, int priority = 0)
      {
        addProperty(new PropertyAccessor<TDest, TRetVal (TInstance::*)(const TDest& value), TDest (TInstance::*)(), TInstance>(name, instance, 0, setter, flags), priority);
      }
      /**
       * Register property setter/getter as serializable
       * @param name Key to search in DataProxy
       * @param setter Property setter
       * @param getter Property getter
       */
      template<typename TDest, typename TRetVal>
      void registerProperty(const std::string& name, TRetVal (C::*setter)(const TDest& value), TDest (C::*getter)(), int flags = 0x00, int priority = 0)
      {
        addProperty(new PropertyAccessor<TDest, TRetVal (C::*)(const TDest& value), TDest (C::*)()>(name, static_cast<C*>(this), getter, setter, flags), priority);
      }

    protected:
      // vector is used to keep an order
      typedef std::vector<AbstractProperty*> PropertyChain;
      // several vectors are used to create property chains with different priorities
      typedef std::map<int, PropertyChain, std::greater<int>> Properties;
      Properties mProperties;

      // map is used to quickly access property by key
      typedef std::map<std::string, AbstractProperty*> PropMappings;
      PropMappings mPropMappings;

      /**
       * Add property to the property list
       * @param property AbstractProperty accessor
       * @param priority Priority of the property, properties with higher value are read first. Default priority is the lowest
       */
      void addProperty(AbstractProperty* property, int priority = 0)
      {
        if(mProperties.count(priority) == 0) {
          mProperties[priority] = PropertyChain();
        }

        mProperties[priority].push_back(property);
        mPropMappings[property->mName] = property;
      }
  };
}

#endif
