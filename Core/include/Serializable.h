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
#include "PtreeExtensions.h"

#define BIND_PROPERTY(name, property) registerProperty(name, property)
#define BIND_PROPERTY_OPTIONAL(name, setter, getter) registerProperty(name, property, Optional)

#define BIND_ACCESSOR(name, setter, getter) registerProperty(name, this, setter, getter)
#define BIND_ACCESSOR_OPTIONAL(name, setter, getter) registerProperty(name, this, setter, getter, Optional)

#define BIND_GETTER(name, getter) registerGetter(name, this, getter)
#define BIND_GETTER_OPTIONAL(name, getter) registerGetter(name, this, getter, Optional)

#define BIND_READONLY_PROPERTY(name, property) registerProperty(name, property, Readonly)
#define BIND_WRITEONLY_PROPERTY(name, property) registerProperty(name, property, Writeonly)

enum PropertyFlag
{
  Readonly = 0x01,
  Writeonly = 0x02,
  Optional = 0x04
};

namespace Gsage
{

  template<typename Type>
  inline bool get(const DataNode& node, const std::string& id, Type& dest)
  {
    boost::optional<Type> value = node.get_optional<Type>(id);
    if(!value)
      return false;

    dest = value.get();
    return true;
  }

  inline bool get(const DataNode& node, std::string& id, DataNode& dest)
  {
    boost::optional<const DataNode&> value = node.get_child_optional(id);
    if(!value)
      return false;

    dest = value.get();
    return true;
  }

  template<typename Type>
  inline bool put(DataNode& node, const std::string& id, const Type& value)
  {
    node.put(id, value);
    return true;
  }

  inline bool put(DataNode& node, const std::string& id, const DataNode& child)
  {
    if(child.empty())
      return true;
    node.put_child(id, child);
    return true;
  }

  inline bool put(DataNode& node, const std::string& id, const std::string& value)
  {
    if(value.empty())
      return true;
    node.put(id, value);
    return true;
  }

  /**
   * Class that has bindings for quick reading fields from DataNode and writing it to it
   */
  template<typename C>
  class Serializable
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
           * Read property from DataNode
           * @param node DataNode
           */
          virtual bool read(const DataNode& node) = 0;
          /**
           * Write property to the DataNode
           * @param node DataNode
           */
          virtual bool dump(DataNode& node) = 0;

          std::string mName;

        protected:
          int mFlags;
          bool isFlagSet(const PropertyFlag& flag)
          {
            return (mFlags & flag) == flag;
          }
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
           * @param node Should contain value with key, that is defined in constructor of object
           */
          bool read(const DataNode& node)
          {
            if(AbstractProperty::isFlagSet(Readonly) || AbstractProperty::isFlagSet(Optional))
              return true;

            return get(node, AbstractProperty::mName, *mPropertyPtr);
          }

          /**
           * Write property to the data node
           * @param node DataNode will contain value with specified key
           */
          bool dump(DataNode& node)
          {
            if(AbstractProperty::isFlagSet(Writeonly))
              return true;

            if(mPropertyPtr == NULL)
              return false;

            return put(node, AbstractProperty::mName, *mPropertyPtr);
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
           * @param node Should contain value with key, that is defined in constructor of object
           */
          bool read(const DataNode& node)
          {
            T value;
            // setter is not set, it is normal
            if(mSetter == 0)
              return true;
            if(!get(node, AbstractProperty::mName, value))
              return AbstractProperty::isFlagSet(Optional);

            (mInstance->*mSetter)(value);
            return true;
          }

          /**
           * Call class getter and write return value to the data node
           * @param node DataNode will contain value with specified key
           */
          bool dump(DataNode& node)
          {
            if(mGetter == 0)
              return false;

            return put(node, AbstractProperty::mName, (mInstance->*mGetter)());
          }
        private:
          TInstance* mInstance;
          TGetter mGetter;
          TSetter mSetter;
      };

      virtual ~Serializable()
      {
        for(unsigned int i = 0; i < mProperties.size(); i++)
        {
          AbstractProperty* p = mProperties[i];
          delete p;
        }
        mProperties.clear();
      }

      /**
       * Read a particular property
       * @param node To read property from
       * @param id Property id
       */
      virtual bool read(const DataNode& node, const std::string& id)
      {
        if(mPropMappings.count(id) == 0)
          return false;

        return mPropMappings[id]->read(node);
      }

      /**
       * Iterates through all specified properties and reads each from the node
       * @param node DataNode to read
       */
      virtual bool read(const DataNode& node)
      {
        bool allSucceed = true;
        for(AbstractProperty* prop : mProperties)
        {
          if(!prop->read(node))
          {
            allSucceed = false;
          }
        }
        return allSucceed;
      }

      /**
       * Iterates through all specified properties and puts each to the node
       * @param node DataNode to write
       */
      virtual bool dump(DataNode& node)
      {
        bool allSucceed = true;
        for(AbstractProperty* prop : mProperties)
        {
          if(!prop->dump(node))
            allSucceed = false;
        }
        return allSucceed;
      }

      /**
       * Register property as serializable
       * @param name Key to search in DataNode
       * @param dest Pointer to field to wrap
       * @param flags property flags
       */
      template<typename TDest>
      void registerProperty(const std::string& name, TDest* dest, int flags = 0x00)
      {
        addProperty(new Property<TDest>(name, dest, flags));
      }

      /**
       * Register property setter/getter as serializable.
       * This method can be used with any class, not self only.
       * @param name Key to search in DataNode
       * @param instance Instance of object, that contains getters and setters
       * @param setter Property setter
       * @param getter Property getter
       */
      template<typename TDest, class TInstance, class TRetVal>
      void registerProperty(const std::string& name, TInstance* instance, TRetVal (TInstance::*setter)(const TDest& value), TDest (TInstance::*getter)(), int flags = 0x00)
      {
        addProperty(new PropertyAccessor<TDest, TRetVal (TInstance::*)(const TDest& value), TDest (TInstance::*)(), TInstance>(name, instance, getter, setter, flags));
      }
      /**
       * Register property setter/getter as serializable.
       * This method can be used with any class, not self only.
       * @param name Key to search in DataNode
       * @param instance Instance of object, that contains getters and setters
       * @param setter Property setter
       * @param getter Property getter
       */
      template<typename TDest, class TInstance, class TRetVal>
      void registerProperty(const std::string& name, TInstance* instance, TRetVal (TInstance::*setter)(const TDest& value), const TDest& (TInstance::*getter)()const, int flags = 0x00)
      {
        addProperty(new PropertyAccessor<TDest, TRetVal (TInstance::*)(const TDest& value), const TDest& (TInstance::*)()const, TInstance>(name, instance, getter, setter, flags));
      }
      /**
       * Register property getter as serializable.
       * This method can be used with any class, not self only.
       * @param name Key to search in DataNode
       * @param instance Instance of object, that contains getters and setters
       * @param getter Property getter
       */
      template<typename TDest, class TInstance>
      void registerGetter(const std::string& name, TInstance* instance, TDest (TInstance::*getter)(), int flags = 0x00)
      {
        addProperty(new PropertyAccessor<TDest, void (TInstance::*)(const TDest& value), TDest (TInstance::*)(), TInstance>(name, instance, getter, 0, flags));
      }
      /**
       * Register property setter/getter as serializable
       * @param name Key to search in DataNode
       * @param setter Property setter
       * @param getter Property getter
       */
      template<typename TDest, typename TRetVal>
      void registerProperty(const std::string& name, TRetVal (C::*setter)(const TDest& value), TDest (C::*getter)(), int flags = 0x00)
      {
        addProperty(new PropertyAccessor<TDest, TRetVal (C::*)(const TDest& value), TDest (C::*)()>(name, static_cast<C*>(this), getter, setter, flags));
      }

    protected:
      // vector is used to keep an order
      typedef std::vector<AbstractProperty*> Properties;
      Properties mProperties;

      // map is used to quickly access property by key
      typedef std::map<std::string, AbstractProperty*> PropMappings;
      PropMappings mPropMappings;

      /**
       * Add property to the property list
       * @param property AbstractProperty accessor
       */
      void addProperty(AbstractProperty* property)
      {
        mProperties.push_back(property);
        mPropMappings[property->mName] = property;
      }
  };
}

#endif
