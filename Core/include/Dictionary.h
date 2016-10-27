#ifndef _Dict_H_
#define _Dict_H_
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

#include <GsageDefinitions.h>
#include <string>
#include <json/json.h>
#include <type_traits>
#include <typeinfo>
namespace Gsage {
  class DictionaryKey;
  class Dictionary;
}
std::ostream & operator<<(std::ostream & os, Gsage::DictionaryKey const & key);
std::ostream & operator<<(std::ostream & os, Gsage::Dictionary const & dict);

#include "Logger.h"
#include <tuple>
#include <stdexcept>

#define TYPE_CASTER(name, t) \
struct name {\
  typedef t Type;\
  bool to(const std::string& src, Type& dst) const;\
  const std::string from(const Type& value) const;\
};\
template<>\
struct TranslatorBetween<std::string, t> { typedef name type; };

#define _OVERRIDE_KEY_OPERATOR(op) \
  friend bool operator op(const DictionaryKey& k1, const DictionaryKey& k2) { \
    if(k1.mIndex == -1) { \
      return k1.mId op k2.mId; \
    }\
    return k1.mIndex op k2.mIndex;\
  }\


namespace Gsage {
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
   * CastException is raised when it's impossible to get value as<something>.
   */
  class CastException: public std::exception
  {
    virtual const char* what() const throw()
    {
      return "Failed to cast value";
    }
  };

  /**
   * NoopCaster does not do any cast.
   *
   * Implements behavior when no caster is found for the requested type.
   */
  template<class T>
  struct NoopCaster
  {
    /**
     * Noop.
     */
    typedef T Type;
    bool to(const std::string& src, Type& dst)
    {
      return false;
    }
    /**
     * Noop.
     */
    const std::string from(const Type& value)
    {
      return "";
    }
  };

  /**
   * Wrapper for Casters.
   */
  template<typename F, typename T>
  struct TranslatorBetween
  {
    typedef NoopCaster<T> type;
  };

  TYPE_CASTER(DoubleCaster, double)
  TYPE_CASTER(IntCaster, int)
  TYPE_CASTER(UlongCaster, unsigned long)
  TYPE_CASTER(FloatCaster, float)
  TYPE_CASTER(BoolCaster, bool)
  TYPE_CASTER(StringCaster, std::string)

  /**
   * Const string caster.
   */
  struct CStrCaster {
    typedef const char* Type;
    bool to(const std::string& src, Type& dst) const;
    const std::string from(const Type& value) const;
  };

  /**
   * Wraps CStrCaster.
   */
  template<size_t N>
  struct TranslatorBetween<std::string, char[N]>
  {
    typedef CStrCaster type;
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

      Dictionary(bool isArray = false) : mIsArray(isArray) {}
      virtual ~Dictionary() {}

      DictionaryKey createKey(const std::string& key) const
      {
        DictionaryKey k(key);
        if(mIsArray)
        {
          k = DictionaryKey(std::stoi(key), key);
        }
        return k;
      }

      /**
       * Reads dictionary value into a reference.
       *
       * @param dest reference to read into
       * @returns true if succeed.
       */
      template<class T>
      bool getValue(T& dest) const
      {
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
        if(mChildren.count(k) == 0)
          return false;

        return mChildren.at(k).getValue(dest);
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
       * Special handling for const char* get.
       *
       * @copydoc Dictionary::get(key, value)
       */
      std::string get(const std::string& key, const char* def) const;

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
          if(dict->mChildren.count(k) == 0) {
            dict->mChildren[k] = Dictionary();
          }

          dict = &dict->mChildren[k];
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
      }

      /**
       * Get begin iterator.
       */
      iterator begin()
      {
        return mChildren.begin();
      }

      /**
       * Get end iterator.
       */
      iterator end()
      {
        return mChildren.end();
      }

      /**
       * @copydoc Dictionary::begin()
       */
      constIterator begin() const
      {
        return mChildren.begin();
      }

      /**
       * @copydoc Dictionary::end()
       */
      constIterator end() const
      {
        return mChildren.end();
      }

      /**
       * Size of dictionary.
       */
      int size() const
      {
        return mChildren.size();
      }

      /**
       * Count of key occurences in Dictionary childrens.
       */
      int count(const std::string &key) const
      {
        return mChildren.count(key);
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
          mChildren.clear();
        }
        mIsArray = value;
      }

      /**
       * @returns true if Dictionary value is empty and Dictionary has no children
       */
      bool empty() const
      {
        return mValue.empty() && mChildren.empty();
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
        DictionaryKey k(mChildren.size(), "");
        mChildren[k] = Dictionary();
        mChildren[k].set(value);
      }
    private:
      bool walkPath(const std::string& key, Dictionary& dest) const;

      Children mChildren;
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
