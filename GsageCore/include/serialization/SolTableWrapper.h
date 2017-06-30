#ifndef _SolTableWrapper_H_
#define _SolTableWrapper_H_

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
#include "sol.hpp"
#include <typeinfo>

namespace Gsage {

  class SolTableWrapper : public DataWrapper
  {
    public:

      class iterator : public DataWrapper::iterator
      {
        public:
          enum Type {
            BEGIN,
            END
          };

          iterator(sol::table& table, Type itertype);

          virtual ~iterator();

          void update();

          virtual void increment();

          virtual bool operator==(const self_type& rhs);

          virtual bool operator!=(const self_type& rhs);
        private:

          sol::table::iterator mTableIterator;
          sol::table& mTable;

          SolTableWrapper* mCurrentValue;
          int mIndex;
      };

      SolTableWrapper(const sol::table& value);

      SolTableWrapper();

      virtual ~SolTableWrapper();

      template<typename K, typename T>
      void put(const K& key, const T& value)
      {
        mObject[correctIndex(key)] = value;
      }

      template<typename T>
      bool read(const std::string& key, T& dest) const
      {
        if(!readExact(key, dest)) {
          return CastHandler<T>().read(this, key, dest);
        }
        return true;
      }

      template<typename T>
      bool readExact(const std::string& key, T& dest) const
      {
        sol::optional<T> res = mObject[key];
        if (res) {
          dest = res.value();
          return true;
        }

        return false;
      }

      int size() const {
        return mObject.size();
      }

      int count(const std::string& key) const {
        if(mObject[key] == sol::lua_nil) {
          return 0;
        }

        return 1;
      }

      template<typename T>
      bool read(T& dest) const
      {
        if(!readExact(dest)) {
          return CastHandler<T>().read(this, dest);
        }
        return true;
      }

      template<typename T>
      bool readExact(T& dest) const
      {
        sol::optional<T> res = mObject.as<T>();
        if (res) {
          dest = res.value();
          return true;
        }

        return false;
      }

      template<typename T>
      void set(const T& value)
      {
        mObject.set(sol::make_object(mObject.lua_state(), value));
      }

      iteratorPtr begin()
      {
        return new iterator(mObject, iterator::BEGIN);
      }

      iteratorPtr end()
      {
        return new iterator(mObject, iterator::END);
      }

      sol::table& getObject()
      {
        return mObject;
      }

      bool putChild(const std::string& key, DataWrapper& value);

      bool putChild(int key, DataWrapper& value);

      virtual DataWrapper* createChildAt(const std::string& key);

      virtual DataWrapper* createChildAt(int key);

      virtual const DataWrapper* getChildAt(const std::string& key) const;

      virtual const DataWrapper* getChildAt(int key) const;

      virtual Type getStoredType() const;

      virtual Type getStoredType();

      virtual void makeArray();
    private:
      /**
       * Type map to wrap underlying object types into general representation
       */
      typedef std::map<sol::type, Type> TypeMap;

      sol::table mObject;

      template<class K>
      K correctIndex(const K& index) const
      {
        return index;
      }

      bool mArrayMode;
  };

  template<>
  int SolTableWrapper::correctIndex(const int& index) const;
}

#endif
