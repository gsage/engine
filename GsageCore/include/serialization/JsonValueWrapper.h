#ifndef _JsonValueWrapper_H_
#define _JsonValueWrapper_H_

/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Gsage Authors

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

#include "serialization/DataWrapper.h"
#include "json/json.h"

namespace Gsage {

  class JsonValueWrapper : public DataWrapper
  {
    public:
      class iterator : public DataWrapper::iterator
      {
        public:
          iterator(Json::Value::iterator wrappedIterator, Json::Value& object);

          virtual ~iterator();

          void update();

          virtual void increment();

          virtual bool operator==(const self_type& rhs);

          virtual bool operator!=(const self_type& rhs);
        private:
          Json::Value::iterator mIterator;
          JsonValueWrapper* mCurrentValue;
          Json::Value& mObject;
      };

      JsonValueWrapper(const Json::Value& value);
      JsonValueWrapper(Json::Value* value);
      JsonValueWrapper(bool allocate = false);
      virtual ~JsonValueWrapper();

      template<typename T>
      void put(const std::string& key, const T& value)
      {
        CastHandler<T>().dump(this, key, value);
      }

      template<typename T>
      void put(int key, const T& value)
      {
        CastHandler<T>().dump(this, key, value);
      }

      void put(const std::string& key, const char* value)
      {
        put(key, std::string(value));
      }

      void put(const std::string& key, const std::string& value);

      void put(int key, const char* value)
      {
        put(key, std::string(value));
      }

      void put(int key, const std::string& value);

      void set(const std::string& value);

      void set(const char* value)
      {
        set(std::string(value));
      }

      template<typename T>
      void set(const T& value)
      {
        CastHandler<T>().dump(this, value);
      }

      template<typename T>
      bool read(const std::string& key, T& dest) const
      {
        if(!readExact(key, dest)) {
          try {
            return CastHandler<T>().read(this, key, dest);
          } catch (...) {
            Json::ValueType jsonType = getJsonType<T>();
            LOG(WARNING) << "Exception in cast handler when trying to object as json type: " << jsonType << ", actual type: " << getObject()[key].type();
            return false;
          }
        }
        return true;
      }

      template<typename T>
      bool readExact(const std::string& key, T& dest) const
      {
        const Json::Value& object = getObject()[key];
        if(object.isNull()) {
          return false;
        }

        Json::ValueType jsonType = getJsonType<T>();
        if(object.type() == jsonType || (std::is_arithmetic<T>::value && object.type() != Json::stringValue)) {
          return readValue(object, dest);
        }

        return false;
      }

      template<typename T>
      bool read(T& dest) const
      {
        if(!readExact(dest)) {
          try {
            return CastHandler<T>().read(this, dest);
          } catch (...) {
            Json::ValueType jsonType = getJsonType<T>();
            LOG(WARNING) << "Exception in cast handler when trying to object as json type: " << jsonType << ", actual type: " << getObject().type();
            return false;
          }
        }

        return true;
      }

      template<typename T>
      bool readExact(T& dest) const
      {
        const Json::Value& object = getObject();
        if(object.isNull()) {
          return false;
        }

        Json::ValueType jsonType = getJsonType<T>();
        if(object.type() == jsonType || (std::is_arithmetic<T>::value && object.type() != Json::stringValue)) {
          return readValue(object, dest);
        }

        return false;
      }

      bool readExact(std::string& dest) const {
        return readString(getObject(), dest);
      }

      bool readExact(const std::string& key, std::string& dest) const {
        return readString(getObject()[key], dest);
      }

      int size() const {
        return getObject().size();
      }

      int count(const std::string& key) const {
        return getObject().isMember(key) ? 1 : 0;
      }

      template<typename T>
      Json::ValueType getJsonType() const {
        return Json::objectValue;
      }

      template<typename T>
      bool readValue(const Json::Value& v, T& dest) const {
        return false;
      }

      iteratorPtr begin()
      {
        return new iterator(getObject().begin(), getObject());
      }

      iteratorPtr end()
      {
        return new iterator(getObject().end(), getObject());
      }

      Json::Value& getObject()
      {
        return *mObject;
      }

      const Json::Value& getObject() const
      {
        return *mObject;
      }

      bool putChild(const std::string& key, DataWrapper& value);

      bool putChild(int key, DataWrapper& value);

      virtual DataWrapper* createChildAt(const std::string& key);

      virtual DataWrapper* createChildAt(int key);

      virtual const DataWrapper* getChildAt(const std::string& key) const;

      virtual const DataWrapper* getChildAt(int key) const;

      Type getStoredType();

      std::string toString() const;

      bool fromString(const std::string& s);
    private:

      bool readString(const Json::Value& value, std::string& dest) const;

      Json::Value* mObject;

      bool mSelfAllocatedObject;

      DataWrapper::Type mapObjectType(const Json::Value& object);
  };

#define _PRIMITIVE_TYPE_SPECIALIZATION(T) \
  template<> \
  Json::ValueType JsonValueWrapper::getJsonType<T>() const; \
  \
  template<> \
  bool JsonValueWrapper::readValue<T>(const Json::Value& v, T& dest) const; \
  \
  template<> \
  void JsonValueWrapper::put<T>(const std::string& key, const T& value); \
  \
  template<> \
  void JsonValueWrapper::put<T>(int key, const T& value); \
  \
  template<> \
  void JsonValueWrapper::set<T>(const T& value);

  _PRIMITIVE_TYPE_SPECIALIZATION(int)
  _PRIMITIVE_TYPE_SPECIALIZATION(unsigned int)
  _PRIMITIVE_TYPE_SPECIALIZATION(double)
  _PRIMITIVE_TYPE_SPECIALIZATION(float)
  _PRIMITIVE_TYPE_SPECIALIZATION(bool)
#undef _PRIMITIVE_TYPE_SPECIALIZATION

  template<>
  Json::ValueType JsonValueWrapper::getJsonType<std::string>() const;

  template<>
  bool JsonValueWrapper::readValue<std::string>(const Json::Value& v, std::string& dest) const;
}


#endif
