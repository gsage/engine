#ifndef _DataWrapper_H_
#define _DataWrapper_H_

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

#include <string>
#include <stdexcept>
#include <memory>

#include "Converters.h"
#include "Logger.h"

namespace Gsage {
  class DataWrapper
  {
    public:
      class iterator
      {
        public:

          typedef iterator self_type;
          typedef std::pair<std::string, DataWrapper*> value_type;

          typedef value_type& reference;
          typedef value_type* pointer;

          typedef std::forward_iterator_tag iterator_category;
          typedef int difference_type;

          iterator()
          {
          }

          virtual ~iterator() {
          }

          virtual void increment() = 0;

          virtual reference ref()
          {
            return mCurrent;
          }

          virtual pointer ptr()
          {
            return &mCurrent;
          }

          virtual bool operator==(const self_type& rhs) = 0;

          virtual bool operator!=(const self_type& rhs) = 0;
        protected:
          value_type mCurrent;
      };

      typedef iterator* iteratorPtr;

      // Supported wrapped types
      enum WrappedType {
        JSON_OBJECT = 0,
        MSGPACK_OBJECT,
        LUA_TABLE,
        RAW,
        NIL
      };

      enum Type {
        Int = 0,
        UInt,
        Float,
        Double,
        String,
        Bool,
        Object,
        Array,
        Null,

        // lua specific
        Function,
        Userdata,
        Thread
      };

      DataWrapper(WrappedType t);
      virtual ~DataWrapper();

      virtual iteratorPtr begin() {
        return 0;
      }

      virtual iteratorPtr end() {
        return 0;
      }

      virtual int size() const {
        return 0;
      }

      virtual int count(const std::string& key) const {
        return 0;
      }

      virtual Type getStoredType() = 0;

      /**
       * Put child to object.
       *
       * @param key child key
       * @param value child value
       */
      virtual bool putChild(const std::string& key, DataWrapper& value) = 0;

      /**
       * Put child for array.
       *
       * @param index child index
       * @param value child value
       */
      virtual bool putChild(int index, DataWrapper& value) = 0;

      /**
       * Create child in the object.
       *
       * @param key child key
       */
      virtual DataWrapper* createChildAt(const std::string& key) = 0;

      /**
       * Create child in the array.
       *
       * @param index child index
       */
      virtual DataWrapper* createChildAt(int index) = 0;

      /**
       * Object access.
       *
       * @param index child index
       */
      virtual DataWrapper* getChildAt(const std::string& key);

      /**
       * Array access.
       *
       * @param index child index
       */
      virtual DataWrapper* getChildAt(int index);

      /**
       * @copydoc getChildAt(key)
       */
      virtual const DataWrapper* getChildAt(const std::string& key) const = 0;

      /**
       * @copydoc getChildAt(index)
       */
      virtual const DataWrapper* getChildAt(int index) const = 0;

      /**
       * Swap DataWrapper data with an other value.
       *
       * @param other DataWrapper
       */
      virtual void swap(const DataWrapper* other);

      /**
       * Convert data to string
       */
      virtual std::string toString() const;

      /**
       * Fill data by parsing string
       * .
       * @param s string to parse
       * @returns true if succeed
       */
      virtual bool fromString(const std::string& s);

      /**
       * Get underlying concrete wrapper type
       */
      WrappedType getType() const;

      /**
       * Hint data wrapper that we will write an array to it.
       * Do nothing by default.
       */
      virtual void makeArray();

    protected:
      WrappedType mType;
  };

  typedef std::shared_ptr<DataWrapper> DataWrapperPtr;

  template<typename T>
  class CastHandler
  {
    public:
      CastHandler() {};
      virtual ~CastHandler() {};

      template<typename C>
      bool read(const C* dw, const std::string& key, T& dest) const {
        std::string value;
        if(dw->readExact(key, value)) {
          return typename TranslatorBetween<std::string, T>::type().to(value, dest);
        }

        char* charValue;
        if(dw->readExact(key, value)) {
          return typename TranslatorBetween<std::string, T>::type().to(value, dest);
        }
        return false;
      }

      template<typename C>
      bool read(const C* dw, T& dest) const {
        std::string value;
        if(dw->readExact(value)) {
          return typename TranslatorBetween<std::string, T>::type().to(value, dest);
        }

        return false;
      }

      template<typename C>
      void dump(C* dw, const std::string& key, const T& value) const {
        dw->put(key, typename TranslatorBetween<std::string, T>::type().from(value));
      }

      template<typename C>
      void dump(C* dw, int key, const T& value) const {
        dw->put(key, typename TranslatorBetween<std::string, T>::type().from(value));
      }

      template<typename C>
      void dump(C* dw, const T& value) const {
        dw->set(typename TranslatorBetween<std::string, T>::type().from(value));
      }
  };
}

#endif
