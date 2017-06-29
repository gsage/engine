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

#include "Dictionary.h"
#include "msgpack.hpp"
#include <json/json.h>
#include <queue>

#define _LOAD_PATH_WRAP(wrapper, func) \
  bool wrapper(const std::string& path, Dictionary& value)\
  {\
    std::ifstream is(path);\
    if(!is)\
      return false;\
    auto pair = func(is);\
    value = pair.first;\
    is.close();\
    return pair.second;\
  }

#define _DUMP_PATH_WRAP(wrapper, func) \
  bool wrapper(const std::string& path, const Dictionary& value)\
  {\
    std::ofstream os(path);\
    if(!os)\
      return false;\
    bool res = func(os, value);\
    os.close();\
    return res;\
  }

namespace msgpack {
  MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

      template<>
      struct convert<Gsage::Dictionary> {
        void create_array(msgpack::object const& o, Gsage::Dictionary& v) const {
          v = Gsage::Dictionary(true);
          for(int i = 0; i < o.via.array.size; ++i) {
            Gsage::Dictionary child;
            (*this)(o.via.array.ptr[i], child);
            v.put(std::to_string(i), child);
          }
        }

        void create_map(msgpack::object const& o, Gsage::Dictionary& v) const {
          object_kv* kv = o.via.map.ptr;
          object_kv* const kvend = o.via.map.ptr + o.via.map.size;
          for(; kv != kvend; ++kv) {
            Gsage::Dictionary child;
            (*this)(kv->val, child);
            v.put(kv->key.as<std::string>(), child);
          }
        }

        msgpack::object const& operator()(msgpack::object const& o, Gsage::Dictionary& v) const {
          switch(o.type) {
            case msgpack::type::ARRAY:
              create_array(o, v);
              break;
            case msgpack::type::MAP:
              create_map(o, v);
              break;
            case msgpack::type::BOOLEAN:
            case msgpack::type::POSITIVE_INTEGER:
            case msgpack::type::NEGATIVE_INTEGER:
            case msgpack::type::FLOAT:
            case msgpack::type::STR:
              v.set(o.as<std::string>());
              break;
            default:
              LOG(WARNING) << "Unhandled type " << o.type;
          }
          return o;
        }
      };

      template<>
      struct pack<Gsage::Dictionary> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Gsage::Dictionary const& v) const {
          // packing member variables as a map.
          if(v.size() == 0)
          {
            o.pack(v.getValueOptional<std::string>(""));
          }
          else
          {
            v.isArray() ? o.pack_array(v.size()) : o.pack_map(v.size());
            for(auto pair : v) {
              if(!v.isArray())
                o.pack(pair.first.str());
              (*this)(o, pair.second);
            }
          }
          return o;
        }
      };
    } // namespace adaptor
  } // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

namespace Gsage
{

  DictionaryKey Dictionary::createKey(const std::string& key) const
  {
    DictionaryKey k(key);
    if(mIsArray)
    {
      k = DictionaryKey(std::stoi(key), key);
    }
    return k;
  }

  template<>
  void Dictionary::put<Dictionary>(const std::string& key, const Dictionary& child)
  {
    std::vector<std::string> parts = split(key, '.');
    Dictionary* dict = this;
    DictionaryKey k;

    for(int i = 0; i < parts.size(); i++) {
      k = createKey(parts[i]);
      if(i == parts.size()-1) {
        break;
      }
      Children& c = dict->getChildren();

      if(c.count(k) == 0) {
        c[k] = Dictionary();
      }

      dict = &c[k];
    }

    dict->getChildren()[k] = child;
  }

  std::string Dictionary::get(const std::string& key, const char* def) const
  {
    auto pair = get<std::string>(key);
    if(pair.second)
    {
      return pair.first;
    }
    return def;
  }

  template<>
  std::pair<Dictionary, bool> Dictionary::get<Dictionary>(const std::string& key)
  {
    Dictionary node;
    bool success = walkPath(key, node);
    return std::make_pair(node, success);
  }

  template<>
  std::pair<Dictionary, bool> Dictionary::get<Dictionary>(const std::string& key) const
  {
    Dictionary node;
    bool success = walkPath(key, node);
    return std::make_pair(node, success);
  }

  template<>
  void Dictionary::push<Dictionary>(const Dictionary& value)
  {
    if(!mIsArray) {
      setArray(true);
    }
    Children& c = getChildren();
    DictionaryKey k(c.size(), "");
    c[k] = value;
  }

  bool Dictionary::walkPath(const std::string& key, Dictionary& dest) const
  {
    std::vector<std::string> parts = split(key, '.');
    Dictionary node = *this;
    DictionaryKey k(key);

    int i = 0;
    for(i = 0; i < parts.size(); i++)
    {
      k = node.createKey(parts[i]);
      if(node.mChildren->count(k) == 0)
      {
        return false;
      }

      dest = node.mChildren->at(k);
      node = dest;
    }

    dest = node;
    return true;
  }

  template<>
  void Dictionary::set<std::string>(const std::string& value)
  {
    mValue = value;
  }

  template<>
  bool Dictionary::getValue<std::string>(std::string& dest) const
  {
    // try to convert any value to string
    /*if(!mValue.contains_type<std::string>()) {
      return false;
    }

    try {
      dest = static_cast<Any>(mValue).cast<std::string>();
    } catch(anyimpl::bad_any_cast) {
      return false;
    }
    return true;*/
    dest = mValue;
    return true;
  }

  std::pair<Dictionary::iterator, bool> Dictionary::insert(const Dictionary::value_type& val)
  {
    return getChildren().insert(val);
  }

  Dictionary::iterator Dictionary::insert(Dictionary::iterator position, const Dictionary::value_type& val)
  {
    return getChildren().insert(position, val);
  }

  bool DoubleCaster::to(const DoubleCaster::FromType& src, DoubleCaster::Type& dst) const
  {
    dst = std::stod(src);
    return true;
  }

  const DoubleCaster::FromType DoubleCaster::from(const DoubleCaster::Type& value) const
  {
    return std::to_string(value);
  }

  void fillDictionary(Json::Value value, Dictionary& dst);
  void convertType(Json::Value value, Dictionary& dst, const std::string& key = "");

  Dictionary createArray(Json::Value value)
  {

    int index = 0;
    Dictionary array(true);
    for( Json::ValueIterator itr = value.begin() ; itr != value.end() ; itr++ ) {
      Dictionary element;
      convertType(*itr, element);
      array.put(std::to_string(index++), element);
    }
    return array;
  }

  void convertType(Json::Value value, Dictionary& dst, const std::string& key)
  {
    switch(value.type()) {
      case Json::intValue:
        putValue(dst, value.asInt(), key);
        break;
      case Json::uintValue:
        putValue(dst, value.asUInt(), key);
        break;
      case Json::realValue:
        putValue(dst, value.asDouble(), key);
        break;
      case Json::stringValue:
        putValue(dst, value.asString(), key);
        break;
      case Json::booleanValue:
        putValue(dst, value.asBool(), key);
        break;
      case Json::arrayValue:
        putValue(dst, createArray(value), key);
        break;
      case Json::objectValue:
        if(key.empty())
        {
          fillDictionary(value, dst);
        }
        else
        {
          Dictionary child;
          fillDictionary(value, child);
          putValue(dst, child, key);
        }
        break;
      default:
        break;
    }
  }

  void fillDictionary(Json::Value value, Dictionary& dst)
  {
    if(value.size() > 0) {
      for( Json::ValueIterator itr = value.begin() ; itr != value.end() ; itr++ ) {
        convertType(*itr, dst, itr.key().asString());
      }
    } else {
      convertType(value, dst);
    }
  }

  std::pair<Dictionary, bool> parseJson(std::ifstream& is)
  {
    Dictionary res;
    Json::Value root;
    Json::Reader reader;
    bool parsed = reader.parse(is, root);
    if(!parsed)
    {
      return std::make_pair(res, false);
    }

    fillDictionary(root, res);

    return std::make_pair(res, true);
  }

  std::pair<Dictionary, bool> parseJson(const std::string& s)
  {
    Dictionary res;
    bool success = parseJson(s, res);
    return std::make_pair(res, success);
  }

  bool parseJson(const std::string& s, Dictionary& dest)
  {
    Json::Value root;
    Json::Reader reader;
    bool parsed = reader.parse(s, root);
    if(!parsed)
    {
      return false;
    }

    fillDictionary(root, dest);
    return true;
  }

  Json::Value toJsonValue(const Dictionary& node)
  {
    Json::Value res;
    if(node.size() > 0)
    {
      int index = 0;
      for(auto pair : node)
      {
        if(node.isArray())
        {
          res[index++] = toJsonValue(pair.second);
        }
        else
        {
          res[pair.first.str()] = toJsonValue(pair.second);
        }
      }
    }
    else
    {
      res = node.getValueOptional<std::string>("");
    }
    return res;
  }

  std::string dumpJson(const Dictionary& node)
  {
    Json::FastWriter writer;
    return writer.write(toJsonValue(node));
  }

  bool dumpJson(std::ofstream& os, const Dictionary& node)
  {
    if(!os)
      return false;
    os << dumpJson(node);
    return true;
  }

  std::pair<Dictionary, bool> parseMsgPack(std::ifstream& stream)
  {
    std::string str;
    stream.seekg(0, std::ios::end);
    str.reserve(stream.tellg());
    stream.seekg(0, std::ios::beg);
    bool success = true;
    try
    {
      str.assign((std::istreambuf_iterator<char>(stream)),
                  std::istreambuf_iterator<char>());
    }
    catch(...)
    {
      LOG(ERROR) << "Failed to read file";
      success = false;
    }

    stream.close();
    if(!success)
    {
      return std::make_pair(Dictionary(), false);
    }
    return parseMsgPack(str);
  }

  std::pair<Dictionary, bool> parseMsgPack(const std::string& s)
  {
    Dictionary dest;
    bool success = parseMsgPack(s, dest);
    return std::make_pair(dest, success);
  }

  bool parseMsgPack(const std::string& s, Dictionary& dest)
  {
    msgpack::object_handle oh =
            msgpack::unpack(s.data(), s.size());
    msgpack::object obj = oh.get();
    try {
      dest = obj.as<Dictionary>();
    } catch (const std::bad_cast& e) {
      return false;
    }
    return true;
  }

  std::string dumpMsgPack(const Dictionary& node)
  {
    std::stringstream ss;
    msgpack::pack(ss, node);
    return ss.str();
  }

  bool dumpMsgPack(std::ofstream& os, const Dictionary& node)
  {
    if(!os)
      return false;
    os << dumpMsgPack(node);
    return true;
  }

  _LOAD_PATH_WRAP(readJson, parseJson)
  _LOAD_PATH_WRAP(readMsgPack, parseMsgPack)
  _DUMP_PATH_WRAP(writeJson, dumpJson)
  _DUMP_PATH_WRAP(writeMsgPack, dumpMsgPack)

  void traverseRecursive(Dictionary &first, const Dictionary &second)
  {
    if(second.size() == 0)
    {
      first.set(second.getValueOptional<std::string>(""));
    }
    else
    {
      for(auto pair : second) {
        Dictionary child = first.get<Dictionary>(pair.first.str(), Dictionary(pair.second.isArray()));
        child.setArray(pair.second.isArray());
        traverseRecursive(child, pair.second);
        first.put(pair.first.str(), child);
      }
    }
  }

  Dictionary getUnionDict(const Dictionary& first, const Dictionary& second)
  {
    Dictionary parent = first;
    unionDict(parent, second);
    return parent;
  }

  void unionDict(Dictionary& first, const Dictionary& second)
  {
    traverseRecursive(first, second);
  }

  // TODO:
  /*std::pair<Dictionary, std::vector<std::string>> diff(const Dictionary& s, const Dictionary& t)
  {
    Dictionary add;
    std::vector<std::string> removeKeys;

    for(auto pair : s)
    {
      if(t.count(pair.first) == 0) {
        result.put<Dictionary>(pair.first);
      } else if(t.size() > 0 && pair.second.size() > 0) {
        result.put<Dictionary>
      }
    }
    return result;
  }*/
}

std::ostream & operator<<(std::ostream & os, Gsage::DictionaryKey const & key)
{
   os << key.str();
   return os;
}

std::ostream & operator<<(std::ostream & os, Gsage::Dictionary const & dict){
  os << dumpJson(dict);
  return os;
}
