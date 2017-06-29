#ifndef _DataProxy_H_
#define _DataProxy_H_

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

#include <tuple>
#include <stdexcept>
#include <string>
#include <iterator>
#include <algorithm>

#include "GsageDefinitions.h"
#include "Logger.h"
#include "Converters.h"
#include "serialization/DataWrapper.h"
#include "serialization/SolTableWrapper.h"
#include "serialization/JsonValueWrapper.h"

namespace Gsage
{
  class DataProxy;
}

std::ostream & operator<<(std::ostream & os, Gsage::DataProxy const & dict);

namespace Gsage {
  class DataProxyException : public std::exception 
  {
    public:
      DataProxyException(const std::string& message, const std::string& detail) {
        std::stringstream ss;
        ss << message << ": " << detail;
        mWhat = ss.str();
      }

      virtual const char* what() const throw()
      {
        return mWhat.c_str();
      }
    private:
      std::string mWhat;
  };

  /**
   * CastException is raised when it's impossible to get value as<something>.
   */
  class CastException: public std::exception
  {
    virtual const char* what() const throw()
    {
      return "Failed to cast value";
    }
  };

  class KeyException : public DataProxyException
  {
    public:
      KeyException(const std::string& detail) : DataProxyException("No such key", detail) {}
  };

  /**
   * TypeToWrapper get wrapper class for type id.
   */
  template<int T>
  struct TypeToWrapper
  {
  };

  /**
   * ClassToWrapper helps us to determine which kind of wrapper can be used for an object type.
   * Also helps to select approriate type id.
   */
  template<class T>
  struct ClassToWrapper
  {
    static const DataWrapper::WrappedType id = DataWrapper::NIL;
  };

  /**
   * Specialization for converting type id to SolTableWrapper type.
   */
  template<>
  struct TypeToWrapper<DataWrapper::LUA_TABLE>
  {
    typedef SolTableWrapper type;
  };

  /**
   * Specialization for converting type id to JsonValueWrapper type.
   */
  template<>
  struct TypeToWrapper<DataWrapper::JSON_OBJECT>
  {
    typedef JsonValueWrapper type;
  };

  /**
   * Specialization for converting wrapped type to SolTableWrapper type.
   */
  template<>
  struct ClassToWrapper<sol::table>
  {
    typedef SolTableWrapper type;
    static const DataWrapper::WrappedType id = DataWrapper::LUA_TABLE;
  };

  /**
   * Specialization for converting wrapped type to JsonValueWrapper type.
   */
  template<>
  struct ClassToWrapper<Json::Value>
  {
    typedef JsonValueWrapper type;
    static const DataWrapper::WrappedType id = DataWrapper::JSON_OBJECT;
  };

  /**
   * Unversal wrapper for the data objects: lua, json and msgpack.
   */
  class DataProxy
  {
    public:
      enum DumpFlags
      {
        Merge = 0x01,
        ForceCopy = 0x02,
      };

      typedef int size_type;

      /**
       * DataWrapper pointer type.
       */
      typedef std::shared_ptr<DataWrapper> DataWrapperPtr;

      /**
       * Iterator base for const and non const iterator.
       */
      class base_iterator
      {
        public:
          typedef base_iterator self_type;
          typedef std::pair<std::string, DataProxy> value_type;

          typedef value_type& reference;
          typedef value_type* pointer;

          typedef std::shared_ptr<value_type> ValuePtr;

          typedef std::shared_ptr<DataWrapper::iterator> WrappedIterator;

          typedef std::forward_iterator_tag iterator_category;
          typedef int difference_type;

          base_iterator(DataWrapper::iterator*);

          virtual ~base_iterator();

          /**
           * Increments iterator value.
           */
          self_type operator++() {
            self_type i = *this;
            mWrappedIterator->increment();
            update();
            return i;
          }

          /**
           * Increments iterator value.
           */
          self_type& operator++(int junk) {
            ++(*this);
            return *this;
          }

          bool operator==(const self_type& rhs) {
            return *mWrappedIterator == *rhs.mWrappedIterator;
          }

          bool operator!=(const self_type& rhs) {
            return *mWrappedIterator != *rhs.mWrappedIterator;
          }

        protected:
          WrappedIterator mWrappedIterator;
          ValuePtr mCurrent;
          void update();
      };

      class iterator : public base_iterator
      {
        public:
          iterator(DataWrapper::iterator* iterator);
          virtual ~iterator();

          reference operator*()
          {
            return *mCurrent;
          }

          pointer operator->() {
            return mCurrent.get();
          }
      };

      class const_iterator : public base_iterator
      {
        public:
          const_iterator(DataWrapper::iterator* iterator);
          virtual ~const_iterator();

          reference operator*()
          {
            return *(const_cast<const pointer>(mCurrent.get()));
          }

          const pointer operator->() {
            return const_cast<const pointer>(mCurrent.get());
          }
      };

      /**
       * Wrap DataProxy around sol::table
       *
       * @param object object to wrap
       */
      static DataProxy create(const sol::table& object);

      /**
       * Create DataProxy with Json::Value copy
       *
       * @param object object to copy from
       */
      static DataProxy create(const Json::Value& object);

      /**
       * Create DataProxy with empty object inside.
       * Does not work for lua table as it should have lua_State inside.
       *
       * @param type of the wrapped object
       */
      static DataProxy create(DataWrapper::WrappedType type);

      /**
       * Wrap DataProxy around sol::table
       *
       * @param object object to wrap
       */
      static DataProxy wrap(sol::table& object);

      /**
       * Wrap DataProxy around Json::Value
       *
       * @param object object to wrap
       */
      static DataProxy wrap(Json::Value& object);

      DataProxy(DataWrapper* dataWrapper);
      DataProxy(DataWrapperPtr dataWrapper);
      DataProxy(bool allocate = true);
      virtual ~DataProxy();

      /**
       * Reads value into a reference.
       *
       * @param dest reference to read into
       * @returns true if succeed.
       */
      template<class T>
      bool getValue(T& dest) const
      {
        return read(dest);
      }

      /**
       * Gets value using type caster, if possible.
       *
       * @param def default value to return if get failed.
       * @returns requested value or default.
       */
      template<class T>
      T getValueOptional(const T& def) const
      {
        T res;
        if(!getValue(res))
        {
          res = def;
        }
        return res;
      }

      /**
       * Get value using type caster.
       *
       * @returns pair, containing value and success flag
       * Value will be undefined, if flag is false
       */
      template<class T>
      std::pair<T, bool> getValue()
      {
        T res;
        bool success = getValue(res);
        return std::make_pair(res, success);
      }

      /**
       * Get value using type caster.
       *
       * @returns value
       * @throws CastException
       */
      template<class T>
      T as() const
      {
        T res;
        if(!getValue(res))
          throw CastException();

        return res;
      }

      /**
       * Get child at key using type caster.
       *
       * @param key child ID
       *
       * @returns pair value, success
       */
      template<class T>
      std::pair<T, bool> get(const std::string& key)
      {
        return static_cast<const DataProxy&>(*this).get<T>(key);
      }

      /**
       * Get child at key using type caster.
       *
       * @param key child ID
       *
       * @returns pair value, success
       */
      template<class T>
      std::pair<T, bool> get(const std::string& key) const
      {
        T res;
        return std::make_pair(res, read(key, res));
      }

      /**
       * Get child at key using type caster, fallback to def, if failed.
       *
       * @param key child ID
       * @param def fallback value
       *
       * @returns value or default
       */
      template<class T>
      T get(const std::string& key, const T& def) const
      {
        auto pair = get<T>(key);
        if(pair.second)
        {
          return pair.first;
        }
        return def;
      }

      /**
       * Put value to key.
       * Thread unsafe
       *
       * @param key to put to
       * @param value to put
       * @param traverse split key by . and put childs for each key part
       */
      template<typename T>
      void put(const std::string& key, const T& value)
      {
        std::vector<std::string> parts = split(key, '.');
        std::string lastPart = parts[parts.size() - 1];
        parts.pop_back();
        DataWrapperPtr wrapper = parts.size() > 0 ? traverseWrite(parts) : mDataWrapper;
        putImpl(wrapper, lastPart, value);
      }

      /**
       * Put to array index
       *
       * @param key array index
       * @param value value to put
       */
      template<typename T>
      void put(int key, const T& value)
      {
        putImpl(mDataWrapper, key, value);
      }

      template<int T>
      auto getWrapper() const
      {
        return getWrapper<T>(mDataWrapper);
      }

      template<int T>
      auto getWrapper()
      {
        return getWrapper<T>(mDataWrapper);
      }

      template<int T>
      auto getWrapper(DataWrapperPtr ptr) const
      {
        return static_cast<typename TypeToWrapper<T>::type*>(ptr.get());
      }

      template<int T>
      auto getWrapper(DataWrapperPtr ptr)
      {
        return static_cast<typename TypeToWrapper<T>::type*>(ptr.get());
      }

      /**
       * Set value.
       *
       * @param value to set
       */
      template<typename T>
      void set(const T& value)
      {
        switch(mDataWrapper->getType()) {
          case DataWrapper::LUA_TABLE:
            getWrapper<DataWrapper::LUA_TABLE>()->set(value);
            break;
          case DataWrapper::JSON_OBJECT:
            getWrapper<DataWrapper::JSON_OBJECT>()->set(value);
            break;
          default:
            LOG(WARNING) << "Can't set " << mDataWrapper->getType();
        }
      }

      /**
       * Read value to the reference
       *
       * @param key to read
       * @param dest destination
       * @returns true if succeed
       */
      template<typename T>
      bool read(const std::string& key, T& dest) const
      {
        std::vector<std::string> parts = split(key, '.');
        std::string lastPart = parts[parts.size()-1];
        parts.pop_back();

        DataWrapperPtr wrapper;

        if(parts.size() > 0) {
          wrapper = traverseSearch(parts);
        } else {
          wrapper = mDataWrapper;
        }

        if(!wrapper) {
          return false;
        }

        return readImpl(wrapper, lastPart, dest);
      }

      /**
       * Read at array index
       *
       * @param key index
       * @param dest destination
       * @returns true if succeed
       */
      template<typename T>
      bool read(const int& key, T& dest) const
      {
        return readImpl(mDataWrapper, key, dest);
      }

      /**
       * Read this object value to reference
       *
       * @param dest destination
       * @returns true if succeed
       */
      template<typename T>
      bool read(T& dest) const {
        switch(mDataWrapper->getType()) {
          case DataWrapper::LUA_TABLE:
            return getWrapper<DataWrapper::LUA_TABLE>()->read(dest);
          case DataWrapper::JSON_OBJECT:
            return getWrapper<DataWrapper::JSON_OBJECT>()->read(dest);
          default:
            LOG(WARNING) << "Can't read type: " << mDataWrapper->getType();
        }
        return false;
      }

      template<class K>
      DataProxy operator[] (const K& index) const
      {
        DataProxy value(getWrappedType());
        if(!read(index, value)) {
          throw KeyException(std::to_string(index));
        }
        return value;
      }

      DataProxy operator[] (const char* key) const;

      int size() const;

      int count(const std::string& key) const;

      template<class T>
      bool copyKey(DataProxy& dest, T& key, DataProxy& value, int flags = 0) const
      {
        switch(value.getStoredType()) {
          case DataWrapper::String:
            dest.put(key, value.as<std::string>());
            break;
          case DataWrapper::Double:
            dest.put(key, value.as<double>());
            break;
          case DataWrapper::Int:
            dest.put(key, value.as<int>());
            break;
          case DataWrapper::UInt:
            dest.put(key, value.as<unsigned int>());
            break;
          case DataWrapper::Bool:
            dest.put(key, value.as<bool>());
            break;
          case DataWrapper::Array:
          case DataWrapper::Object:
            flags & Merge ? dest.mergeChild(key, value) : dest.put(key, value);
            break;
          default:
            return false;
        }
        return true;
      }

      DataWrapper::Type getStoredType() const;

      DataWrapper::WrappedType getWrappedType() const;

      template<typename T>
      bool dump(T& dest, int flags = 0x00) const {
        if(mDataWrapper->getType() == ClassToWrapper<T>::id && (flags & ForceCopy) == 0) {
          dest = static_cast<typename ClassToWrapper<T>::type*>(mDataWrapper.get())->getObject();
        } else {
          DataProxy wrapper = DataProxy::wrap(dest);
          return dump(wrapper, flags);
        }
        return true;
      }

      template<class K, class T>
      void putImpl(DataWrapperPtr wrapper, const K& key, const T& value)
      {
        switch(wrapper->getType()) {
          case DataWrapper::LUA_TABLE:
            getWrapper<DataWrapper::LUA_TABLE>(wrapper)->put(key, value);
            break;
          case DataWrapper::JSON_OBJECT:
            getWrapper<DataWrapper::JSON_OBJECT>(wrapper)->put(key, value);
            break;
          default:
            LOG(WARNING) << "Can't put " << key << " to the wrapper of type: " << mDataWrapper->getType();
        }
      }

      template<class K, class T>
      bool readImpl(DataWrapperPtr wrapper, const K& key, T& dest) const
      {
        switch(wrapper->getType()) {
          case DataWrapper::LUA_TABLE:
            return getWrapper<DataWrapper::LUA_TABLE>(wrapper)->read(key, dest);
          case DataWrapper::JSON_OBJECT:
            return getWrapper<DataWrapper::JSON_OBJECT>(wrapper)->read(key, dest);
          default:
            LOG(WARNING) << "Can't read " << key << " of the wrapper of type: " << mDataWrapper->getType();
        }
        return false;
      }

      iterator begin()
      {
        return iterator(mDataWrapper->begin());
      }

      iterator end()
      {
        return iterator(mDataWrapper->end());
      }

      const_iterator begin() const
      {
        return const_iterator(mDataWrapper->begin());
      }

      const_iterator end() const
      {
        return const_iterator(mDataWrapper->end());
      }

      /**
       * @returns true if DataProxy is empty
       */
      bool empty() const
      {
        return size() == 0 && mDataWrapper->getStoredType() == DataWrapper::Null;
      }

      /**
       * Push element to the last index.
       * Calling this method switches DataProxy mode to array.
       *
       * @param value to push
       */
      template<class T>
      void push(const T& value)
      {
        int index = size();
        mDataWrapper->makeArray();
        return put(index, value);
      }

      /**
       * Create a new child.
       *
       * @param key child path
       */
      DataProxy createChild(const std::string& key);

      /**
       *
       * Convert DataProxy to string.
       * Not all types are supported.
       */
      std::string toString() const;

      /**
       * Create DataProxy from string.
       * Not all types are supported.
       *
       * @param s string to use
       * @returns true if success
       */
      bool fromString(const std::string& s);

      template<class K>
      DataProxy getOrCreateChild(const K& key)
      {
        DataWrapper* wrapper = mDataWrapper->getChildAt(key);
        if(!wrapper) {
          wrapper = mDataWrapper->createChildAt(key);
        }

        return DataProxy(wrapper);
      }

      /**
       * Special handling for const char* get.
       *
       * @copydoc Dictionary::get(key, value)
       */
      std::string get(const std::string& key, const char* def) const;
    protected:

      template<class K>
      void mergeChild(const K& key, const DataProxy& value)
      {
        DataProxy dp = getOrCreateChild(key);
        DataWrapper::Type type = dp.getStoredType();
        if(type == DataWrapper::Array || type == DataWrapper::Object) {
          value.dump(dp, Merge | ForceCopy);
        } else {
          put(key, value);
          return;
        }
        mDataWrapper->putChild(key, *dp.mDataWrapper);
      }

      DataWrapperPtr mDataWrapper;

      DataWrapperPtr traverseSearch(const std::vector<std::string>& parts) const;

      DataWrapperPtr traverseWrite(const std::vector<std::string>& parts);
  };

  template<>
  DataProxy DataProxy::operator[]<std::string>(const std::string& key) const;

  template<>
  void DataProxy::putImpl<int, DataProxy>(DataWrapperPtr dw, const int& key, const DataProxy& value);

  template<>
  void DataProxy::putImpl<std::string, DataProxy>(DataWrapperPtr dw, const std::string& key, const DataProxy& value);

  template<>
  bool DataProxy::readImpl<std::string, DataProxy>(DataWrapperPtr dw, const std::string& key, DataProxy& dest) const;

  template<>
  bool DataProxy::readImpl<int, DataProxy>(DataWrapperPtr dw, const int& key, DataProxy& dest) const;

  template<>
  bool DataProxy::read<DataProxy>(DataProxy& dest) const;

  template<>
  bool DataProxy::dump(DataProxy& dest, int flags) const;

  bool dump(const DataProxy& value, const std::string& path, DataWrapper::WrappedType type);

  std::string dumps(const DataProxy& value, DataWrapper::WrappedType type);

  std::tuple<DataProxy, bool> load(const std::string& path, DataWrapper::WrappedType type);

  /**
   *
   */
  DataProxy loads(const std::string& value, DataWrapper::WrappedType type);

  /**
   *
   */
  bool loads(DataProxy& dest, const std::string& value, DataWrapper::WrappedType type);

  /**
   * Get DataProxy which is union of two proxies
   *
   * @param first DataProxy
   * @param second DataProxy
   *
   * @returns DataProxy
   */
  DataProxy merge(const DataProxy& first, const DataProxy& second);

  /**
   * Merge one proxy into another
   *
   * @param base DataProxy to merge into
   * @param child DataProxy to merge
   */
  void mergeInto(DataProxy& base, const DataProxy& child);
}

#endif
