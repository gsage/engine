#ifndef _Dict_H_
#define _Dict_H_
/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Artem Chernyshev

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

#include <GsageDefinitions.h>
#include <string>
#include <type_traits>
#include <iterator>
#include <algorithm>
#include "DataProxy.h"

namespace Gsage {
  class DictionaryKey;
  class Dictionary;
}
std::ostream & operator<<(std::ostream & os, Gsage::DictionaryKey const & key);
std::ostream & operator<<(std::ostream & os, Gsage::Dictionary const & dict);

#define _OVERRIDE_KEY_OPERATOR(op) \
  friend bool operator op(const DictionaryKey& k1, const DictionaryKey& k2) { \
    if(k1.mIndex == -1) { \
      return k1.mId op k2.mId; \
    }\
    return k1.mIndex op k2.mIndex;\
  }\

#include "Logger.h"
#include <tuple>
#include <stdexcept>

namespace Gsage {
  namespace detail {
    struct AccessPolicy {
        AccessPolicy(const std::type_info& ti) : mTypeInfo(ti) {}
        virtual void staticDelete(void** value) = 0;
        virtual void copyFromValue(void const* src, void** dest) = 0;
        virtual void clone(void* const* src, void** dest) = 0;
        virtual void move(void* const* src, void** dest) = 0;
        virtual void* getValue(void** src) = 0;
        virtual const void* getValue(void* const* src) const = 0;
        virtual size_t getSize() = 0;

        template<typename T>
        bool containsType() {
          return typeid(T) == mTypeInfo;
        }
      private:
        const std::type_info& mTypeInfo;
    };

    template<typename T>
    struct TypedAccessPolicy : public AccessPolicy
    {
      TypedAccessPolicy() : AccessPolicy(typeid(T)) {}

      virtual size_t getSize()
      {
        return sizeof(T);
      }
    };

    template<typename T>
    struct SmallAccessPolicy : public TypedAccessPolicy<T>
    {
      virtual void staticDelete(void** x) { }
      virtual void copyFromValue(void const* src, void** dest)
      { new(dest) T(*reinterpret_cast<T const*>(src)); }
      virtual void clone(void* const* src, void** dest) { *dest = *src; }
      virtual void move(void* const* src, void** dest) { *dest = *src; }
      virtual void* getValue(void** src) { return reinterpret_cast<void*>(src); }
      virtual const void* getValue(void* const* src) const { return reinterpret_cast<const void*>(src); }
    };

    template<typename T, size_t N>
    struct ArrayAccessPolicy : public SmallAccessPolicy<T>
    {
      virtual void copyFromValue(void const* src, void** dest)
      {
        T* dstArray = new T[N]();
        const T* arr = reinterpret_cast<const T*>(src);

        for(int i = 0; i < N; i++) {
          dstArray[i] = arr[i];
        }

        *dest = dstArray;
      }
    };

    template<typename T>
    struct BigAccessPolicy : public TypedAccessPolicy<T>
    {
      virtual void staticDelete(void** x) { if (*x != NULL)
        delete(*reinterpret_cast<T**>(x)); *x = NULL; }
      virtual void copyFromValue(void const* src, void** dest) {
        *dest = new T(*reinterpret_cast<T const*>(src)); }
      virtual void clone(void* const* src, void** dest) {
        if(*src != NULL) *dest = new T(**reinterpret_cast<T* const*>(src)); }
      virtual void move(void* const* src, void** dest) {
        (*reinterpret_cast<T**>(dest))->~T();
        **reinterpret_cast<T**>(dest) = **reinterpret_cast<T* const*>(src); }
      virtual void* getValue(void** src) { return *src; }
      virtual const void* getValue(void* const* src) const { return *src; }
    };

    template<typename T>
    struct AccessPolicyFactory {
      static AccessPolicy* getPolicy() {
        static BigAccessPolicy<T> policy;
        return &policy;
      }
    };

    template<typename T>
    struct AccessPolicyFactory<T*> {
      static AccessPolicy* getPolicy() {
        static SmallAccessPolicy<T*> policy;
        return &policy;
      }
    };

    template<typename T, size_t N>
    struct AccessPolicyFactory<T[N]> {
      static AccessPolicy* getPolicy() {
        static ArrayAccessPolicy<T, N> policy;
        return &policy;
      }
    };

    /// Specializations for small types.
#define SMALL_POLICY(TYPE) template<> struct \
    AccessPolicyFactory<TYPE> { \
      static AccessPolicy* getPolicy() {\
        static SmallAccessPolicy<TYPE> policy;\
        return &policy;\
      }\
    };\

    SMALL_POLICY(signed char);
    SMALL_POLICY(unsigned char);
    SMALL_POLICY(signed short);
    SMALL_POLICY(unsigned short);
    SMALL_POLICY(signed int);
    SMALL_POLICY(unsigned int);
    SMALL_POLICY(signed long);
    SMALL_POLICY(unsigned long);
    SMALL_POLICY(float);
    SMALL_POLICY(bool);
    SMALL_POLICY(double);

#undef SMALL_POLICY
  }
  /**
   * DictionaryKey provides ordered map of strings.
   * Allows ordering by int value instead of string itself.
   *
   * Dictionary is used as an array in that case.
   */
  class DictionaryKey
  {
    public:
      DictionaryKey() {}

      DictionaryKey(int index, const std::string& id)
        : mIndex(index)
          , mId(id)
      {}

      DictionaryKey(const std::string& id)
        : mIndex(-1)
          , mId(id)
      {}

      DictionaryKey(const char* id)
        : mIndex(-1)
          , mId(id)
      {}

      virtual ~DictionaryKey() {};

      const std::string& str() const {
        return mId;
      }

      _OVERRIDE_KEY_OPERATOR(>);
      _OVERRIDE_KEY_OPERATOR(<);
      _OVERRIDE_KEY_OPERATOR(>=);
      _OVERRIDE_KEY_OPERATOR(<=);
      _OVERRIDE_KEY_OPERATOR(==);

      friend bool operator==(const DictionaryKey& k1, const std::string& id) {
        return k1.mId == id;
      }

      friend bool operator==(const DictionaryKey& k1, const char* id) {
        return k1.mId == id;
      }

      operator const std::string&() const {
        return str();
      }

      DictionaryKey& operator=(const std::string& id) {
        mId = id;
        return *this;
      }
      friend class Dictionary;
    private:
      int mIndex;
      std::string mId;
  };

  /**
   * Variable type dictionary.
   *
   * Stores all values as strings.
   * Mainly used to load all config files in various format into the application.
   *
   * Can be encoded and decoded easily.
   *
   * msgpack and json encoding are supported at the moment.
   *
   * TODO: yaml.
   */
  class Dictionary
  {
    public:
      /**
       * Children map.
       */
      typedef std::map<DictionaryKey, Dictionary> Children;
      /**
       * Children iterator.
       */
      typedef Children::iterator iterator;
      /**
       * Children const iterator.
       */
      typedef Children::const_iterator constIterator;

      typedef DictionaryKey key_type;
      typedef std::pair<DictionaryKey, Dictionary> value_type;

      Dictionary(bool isArray = false) : mIsArray(isArray), mChildren(new Children()) {}
      Dictionary(const std::string& data, bool isArray = false) : mIsArray(isArray), mChildren(new Children()) { }

      virtual ~Dictionary() {}

      /**
       * Reads dictionary value into a reference.
       *
       * @param dest reference to read into
       * @returns true if succeed.
       */
      template<class T>
      bool getValue(T& dest) const
      {
        //return policy->convert<T>(dest);
        /*if(mPolicy->containsType<std::string>()) {
          return typename TranslatorBetween<std::string, T>::type().to(*reinterpret_cast<const std::string*>(mPolicy->getValue(&mValue)), dest);
        } else {
          return false;
        }*/
        return typename TranslatorBetween<std::string, T>::type().to(mValue, dest);
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
       * Read value from children map to the reference and try cast it using type caster.
       *
       * @param key child ID
       * @param dest value to read into
       *
       * @returns success flag
       */
      template<class T>
      bool read(const std::string& key, T& dest) const
      {
        DictionaryKey k = createKey(key);
        if(mChildren->count(k) == 0)
          return false;

        return mChildren->at(k).getValue(dest);
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
        return static_cast<const Dictionary&>(*this).get<T>(key);
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
        Dictionary node;
        if(!walkPath(key, node))
        {
          return std::make_pair(res, false);
        }
        bool success = node.getValue(res);
        return std::make_pair(res, success);
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
       *
       */
      template<typename T>
      void put(const std::string& key, const T& value)
      {
        std::vector<std::string> parts = split(key, '.');
        Dictionary* dict = this;

        for(int i = 0; i < parts.size(); i++) {
          DictionaryKey k = createKey(parts[i]);
          Children& c = dict->getChildren();
          if(c.count(k) == 0) {
            c[k] = Dictionary();
          }

          dict = &c[k];
        }

        dict->set(value);
      }

      /**
       * Set Dictionary value.
       *
       * @param value to set
       */
      template<typename T>
      void set(const T& value)
      {
        mValue = typename TranslatorBetween<std::string, T>::type().from(value);
        //mPolicy = detail::AccessPolicyFactory<T>::getPolicy();
        //mPolicy->copyFromValue(&value, &mValue);
      }

      /**
       * Get begin iterator.
       */
      iterator begin()
      {
        return mChildren->begin();
      }

      /**
       * Get end iterator.
       */
      iterator end()
      {
        return mChildren->end();
      }

      /**
       * @copydoc Dictionary::begin()
       */
      constIterator begin() const
      {
        return mChildren->begin();
      }

      /**
       * @copydoc Dictionary::end()
       */
      constIterator end() const
      {
        return mChildren->end();
      }

      /**
       * Size of dictionary.
       */
      int size() const
      {
        return mChildren->size();
      }

      /**
       * Count of key occurences in Dictionary childrens.
       */
      int count(const std::string &key) const
      {
        return mChildren->count(key);
      }

      /**
       * Check is Dictionary is and array.
       */
      bool isArray() const
      {
        return mIsArray;
      }

      /**
       * Switch Dictionary mode.
       * Changing mode erases data.
       *
       * @param value flag
       *
       */
      void setArray(bool value)
      {
        // clear children if type changed
        if(mIsArray != value)
        {
          mChildren->clear();
        }
        mIsArray = value;
      }

      /**
       * @returns true if Dictionary value is empty and Dictionary has no children
       */
      bool empty() const
      {
        return mChildren->empty();
      }

      /**
       * Push element to the last index.
       * Calling this method switches Dictionary mode to array.
       *
       * @param value to push
       */
      template<class T>
      void push(const T& value)
      {
        if(!mIsArray) {
          setArray(true);
        }
        Children& c = getChildren();
        DictionaryKey k(c.size(), "");
        c[k] = Dictionary();
        c[k].set(value);
      }

      std::pair<iterator, bool> insert(const value_type& val);

      iterator insert(iterator position, const value_type& val);

      DictionaryKey createKey(const std::string& key) const;
      /**
       * Special handling for const char* get.
       *
       * @copydoc Dictionary::get(key, value)
       */
      std::string get(const std::string& key, const char* def) const;

    private:
      bool walkPath(const std::string& key, Dictionary& dest) const;

      Children& getChildren() {
        return *mChildren.get();
      }

      const Children& getChildren() const {
        return *mChildren.get();
      }

      std::shared_ptr<Children> mChildren;
      std::string mValue;
      bool mIsArray;
  };

  /**
   * @copydoc Dictionary::get()
   */
  template<>
  std::pair<Dictionary, bool> Dictionary::get<Dictionary>(const std::string& key);

  /**
   * @copydoc Dictionary::get()
   */
  template<>
  std::pair<Dictionary, bool> Dictionary::get<Dictionary>(const std::string& key) const;

  /**
   * @copydoc Dictionary::push()
   */
  template<>
  void Dictionary::push<Dictionary>(const Dictionary& value);

  /**
   * @copydoc Dictionary::put()
   */
  template<>
  void Dictionary::put<Dictionary>(const std::string& key, const Dictionary& child);

  /**
   * @copydoc Dictionary::set()
   */
  template<>
  void Dictionary::set<std::string>(const std::string& value);

  /**
   * @copydoc Dictionary::getValue()
   */
  template<>
  bool Dictionary::getValue<std::string>(std::string& dest) const;

  /**
   * Parse json from ifstream.
   *
   * @param is input file stream to read from
   *
   * @returns pair Dictionary, success
   */
  std::pair<Dictionary, bool> parseJson(std::ifstream& is);

  /**
   * Parse json from string.
   *
   * @param s string to parse from
   *
   * @returns pair Dictionary, success
   */
  std::pair<Dictionary, bool> parseJson(const std::string& s);

  /**
   * Read json into Dictionary.
   *
   * @param s string to read
   * @param dest ref to write to
   *
   * @returns success flag
   */
  bool parseJson(const std::string& s, Dictionary& dest);

  /**
   * Read json from the file path.
   * This method will open and close file itself.
   *
   * @param path file path
   * @param dest ref to write to
   *
   * @return success flag
   */
  bool readJson(const std::string& path, Dictionary& dest);

  /**
   * Dump Dictionary to json string.
   *
   * @param node Dictionary to dump
   *
   * @returns json string. Empty, if failed
   */
  std::string dumpJson(const Dictionary& dict);

  /**
   * Dump Dictionary to the ofstream in json format.
   *
   * @param os ofstream to dump to
   * @param dict Dictionary to Dump
   *
   * @returns success flag
   */
  bool dumpJson(std::ofstream& os, const Dictionary& dict);

  /**
   * Write Dictionary to the file located on path in json format.
   *
   * @param path file path
   * @param dict Dictionary to write
   *
   * @returns success flag
   */
  bool writeJson(const std::string& path, const Dictionary& dict);

  /**
   * Parse msgpack from ifstream.
   *
   * @param is input file stream to read from
   *
   * @returns pair Dictionary, success
   */
  std::pair<Dictionary, bool> parseMsgPack(std::ifstream& is);

  /**
   * Parse msgpack from string.
   *
   * @param s string to parse from
   *
   * @returns pair Dictionary, success
   */
  std::pair<Dictionary, bool> parseMsgPack(const std::string& s);

  /**
   * Read msgpack into Dictionary.
   *
   * @param s string to read
   * @param dest ref to write to
   *
   * @returns success flag
   */
  bool parseMsgPack(const std::string& s, Dictionary& dest);

  /**
   * Read msgpack from the file path.
   * This method will open and close file itself.
   *
   * @param path file path
   * @param dest ref to write to
   *
   * @return success flag
   */
  bool readMsgPack(const std::string& path, Dictionary& dest);

  /**
   * Dump Dictionary to msgpack string.
   *
   * @param node Dictionary to dump
   *
   * @returns msgpack string. Empty, if failed
   */
  std::string dumpMsgPack(const Dictionary& dict);

  /**
   * Dump Dictionary to the ofstream in msgpack format.
   *
   * @param os ofstream to dump to
   * @param dict Dictionary to Dump
   *
   * @returns success flag
   */
  bool dumpMsgPack(std::ofstream& os, const Dictionary& dict);

  /**
   * Write Dictionary to the file located on path in msgpack format.
   *
   * @param path file path
   * @param dict Dictionary to write
   *
   * @returns success flag
   */
  bool writeMsgPack(const std::string& path, const Dictionary& dict);

  /**
   * internal method
   */
  template<typename T>
  inline void putValue(Dictionary& dst, const T& value, const std::string& key = "")
  {
    if(key.empty())
    {
      dst.set(value);
    }
    else
    {
      dst.put(key, value);
    }
  }

  /**
   * Get Dictionary which is union of two dicts
   *
   * @param first Dictionary
   * @param second Dictionary
   *
   * @returns Dictionary
   */
  Dictionary getUnionDict(const Dictionary& first, const Dictionary& second);

  /**
   * Merge one dict into another
   *
   * @param mergeTo Dictionary to merge into
   * @param child Dictionary to merge
   */
  void unionDict(Dictionary& mergeTo, const Dictionary& child);
}

#endif
