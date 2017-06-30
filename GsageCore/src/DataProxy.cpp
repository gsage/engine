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

#include "DataProxy.h"

#include <msgpack.hpp>

namespace msgpack {
  MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

      template<>
      struct convert<Gsage::DataProxy> {
        void create_array(msgpack::object const& o, Gsage::DataProxy& v) const {
          for(int i = 0; i < o.via.array.size; ++i) {
            Gsage::DataProxy child = Gsage::DataProxy::create(v.getWrappedType());
            (*this)(o.via.array.ptr[i], child);
            v.put(i, child);
          }
        }

        void create_map(msgpack::object const& o, Gsage::DataProxy& v) const {
          object_kv* kv = o.via.map.ptr;
          object_kv* const kvend = o.via.map.ptr + o.via.map.size;
          for(; kv != kvend; ++kv) {
            Gsage::DataProxy child = Gsage::DataProxy::create(v.getWrappedType());
            (*this)(kv->val, child);
            v.put(kv->key.as<std::string>(), child);
          }
        }

        msgpack::object const& operator()(msgpack::object const& o, Gsage::DataProxy& v) const {
          switch(o.type) {
            case msgpack::type::ARRAY:
              create_array(o, v);
              break;
            case msgpack::type::MAP:
              create_map(o, v);
              break;
            case msgpack::type::BOOLEAN:
              v.set(o.as<bool>());
              break;
            case msgpack::type::POSITIVE_INTEGER:
              v.set(o.as<unsigned int>());
              break;
            case msgpack::type::NEGATIVE_INTEGER:
              v.set(o.as<int>());
              break;
            case msgpack::type::FLOAT:
              v.set(o.as<double>());
              break;
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
      struct pack<Gsage::DataProxy> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Gsage::DataProxy const& v) const {
          switch(v.getStoredType()) {
            case Gsage::DataWrapper::Int:
              o.pack(v.getValueOptional<int>(0));
              break;
            case Gsage::DataWrapper::UInt:
              o.pack(v.getValueOptional<unsigned int>(0));
              break;
            case Gsage::DataWrapper::Float:
              o.pack(v.getValueOptional<float>(0.0));
              break;
            case Gsage::DataWrapper::Double:
              o.pack(v.getValueOptional<double>(0.0));
              break;
            case Gsage::DataWrapper::String:
              o.pack(v.getValueOptional<std::string>(""));
              break;
            case Gsage::DataWrapper::Bool:
              o.pack(v.getValueOptional<bool>(false));
              break;
            default:
              break;
            case Gsage::DataWrapper::Object:
            case Gsage::DataWrapper::Array:
              int size = 0;

              // size() does not work nicely for lua table
              // so we have to count it in cycle
              for(auto pair : v) {
                size++;
              }
              v.getStoredType() == Gsage::DataWrapper::Array ? o.pack_array(size) : o.pack_map(size);
              for(auto pair : v) {
                if(v.getStoredType() == Gsage::DataWrapper::Object)
                  o.pack(pair.first);
                (*this)(o, pair.second);
              }
              break;
          }
          return o;
        }
      };
    } // namespace adaptor
  } // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

namespace Gsage {
  void NoDelete(DataWrapper *) {}

  class DecodeException : public DataProxyException
  {
    public:
      DecodeException(const std::string& detail) : DataProxyException("Failed to decode", detail) {}
  };

  class CreateException : public DataProxyException
  {
    public:
      CreateException(const std::string& detail) : DataProxyException("Failed to create data proxy", detail) {}
  };

  DataProxy::base_iterator::base_iterator(DataWrapper::iterator* iterator)
    : mWrappedIterator(std::shared_ptr<DataWrapper::iterator>(iterator))
    , mCurrent(ValuePtr(new value_type()))
  {
    update();
  }

  DataProxy::base_iterator::~base_iterator() {
  }

  void DataProxy::base_iterator::update() {
    DataWrapper::iterator::reference pair = mWrappedIterator->ref();
    mCurrent->first = pair.first;
    mCurrent->second.mDataWrapper = DataWrapperPtr(pair.second, &NoDelete);
  }

  DataProxy::iterator::iterator(DataWrapper::iterator* iterator)
    : base_iterator(iterator)
  {
  }

  DataProxy::iterator::~iterator() {
  }

  DataProxy::const_iterator::const_iterator(DataWrapper::iterator* iterator)
    : base_iterator(iterator)
  {
  }

  DataProxy::const_iterator::~const_iterator() {
  }

  DataProxy DataProxy::create(const sol::table& object) {
    return DataProxy(new SolTableWrapper(object));
  }

  DataProxy DataProxy::create(const Json::Value& object) {
    return DataProxy(new JsonValueWrapper(object));
  }

  DataProxy DataProxy::create(DataWrapper::WrappedType type) {
    switch(type) {
      case DataWrapper::LUA_TABLE:
        throw CreateException("cannot use LUA_TABLE to create the DataProxy. Lua table can be wrapped or created by copying sol::object only.");
      case DataWrapper::JSON_OBJECT:
        return DataProxy(new typename TypeToWrapper<DataWrapper::JSON_OBJECT>::type(true));
      default:
        break;
    }

    return DataProxy();
  }

  DataProxy DataProxy::wrap(sol::table& object) {
    return DataProxy::create(object);
  }

  DataProxy DataProxy::wrap(Json::Value& object) {
    return DataProxy(new JsonValueWrapper(&object));
  }

  DataProxy::DataProxy(DataWrapper* dataWrapper) : mDataWrapper(DataWrapperPtr(dataWrapper)) {
  }

  DataProxy::DataProxy(DataWrapperPtr dataWrapper) : mDataWrapper(dataWrapper) {
  }

  DataProxy::DataProxy(bool allocate)
    : mDataWrapper(DataWrapperPtr(allocate ? new TypeToWrapper<DataWrapper::JSON_OBJECT>::type(true) : nullptr))
  {
  }

  DataProxy::~DataProxy()
  {
  }

  template<>
  bool DataProxy::dump(DataProxy& dest, int flags) const {
    if(mDataWrapper->getType() == dest.mDataWrapper->getType() && (flags & ForceCopy) == 0) {
      dest.mDataWrapper = mDataWrapper;
    } else {
      for(auto pair : *this) {
        if(mDataWrapper->getStoredType() == DataWrapper::Array) {
          int index = std::atoi(pair.first.c_str());
          dest.mDataWrapper->makeArray();
          copyKey(dest, index, pair.second, flags);
        } else {
          copyKey(dest, pair.first, pair.second, flags);
        }
      }
      return true;
    }
    return true;
  }

  DataWrapper::Type DataProxy::getStoredType() const
  {
    return mDataWrapper->getStoredType();
  }

  DataWrapper::WrappedType DataProxy::getWrappedType() const
  {
    return mDataWrapper->getType();
  }

  int DataProxy::size() const
  {
    return mDataWrapper->size();
  }

  int DataProxy::count(const std::string& key) const
  {
    return mDataWrapper->count(key);
  }

  template<>
  void DataProxy::putImpl<std::string, DataProxy>(DataWrapperPtr dw, const std::string& key, const DataProxy& value)
  {
    if(dw->getType() == value.mDataWrapper->getType()) {
      dw->putChild(key, *value.mDataWrapper);
      return;
    }

    DataProxy dp = DataProxy(dw->createChildAt(key));
    value.dump(dp);
  }

  template<>
  void DataProxy::putImpl<int, DataProxy>(DataWrapperPtr dw, const int& key, const DataProxy& value)
  {
    if(dw->getType() == value.mDataWrapper->getType()) {
      dw->putChild(key, *value.mDataWrapper);
      return;
    }

    DataProxy dp = DataProxy(dw->createChildAt(key));
    value.dump(dp);
  }


  template<>
  bool DataProxy::readImpl<std::string, DataProxy>(DataWrapperPtr dw, const std::string& key, DataProxy& dest) const
  {
    const DataWrapper* child = dw->getChildAt(key);
    if(!child) {
      return false;
    }

    dest = DataProxy(const_cast<DataWrapper*>(child));
    return true;
  }

  template<>
  bool DataProxy::readImpl<int, DataProxy>(DataWrapperPtr dw, const int& key, DataProxy& dest) const
  {
    const DataWrapper* child = dw->getChildAt(key);
    if(!child) {
      return false;
    }

    dest = DataProxy(const_cast<DataWrapper*>(child));
    return true;
  }

  template<>
  DataProxy DataProxy::operator[]<std::string>(const std::string& key) const
  {
    DataProxy value(getWrappedType());
    if(!read(key, value)) {
      throw KeyException(key);
    }
    return value;
  }

  DataProxy DataProxy::operator[] (const char* key) const
  {
    return (*this)[std::string(key)];
  }

  template<>
  bool DataProxy::read<DataProxy>(DataProxy& dest) const
  {
    dest = *this;
    return true;
  }

  DataProxy DataProxy::createChild(const std::string& key)
  {
    std::vector<std::string> parts = split(key, '.');
    std::string lastPart = parts[parts.size() - 1];
    parts.pop_back();

    DataWrapperPtr wrapper = parts.size() > 0 ? traverseWrite(parts) : mDataWrapper;
    return DataProxy(mDataWrapper->createChildAt(key));
  }

  std::string DataProxy::toString() const
  {
    return mDataWrapper->toString();
  }

  bool DataProxy::fromString(const std::string& s)
  {
    return mDataWrapper->fromString(s);
  }

  std::string DataProxy::get(const std::string& key, const char* def) const
  {
    auto pair = get<std::string>(key);
    if(pair.second)
    {
      return pair.first;
    }
    return def;
  }

  DataProxy::DataWrapperPtr DataProxy::traverseSearch(const std::vector<std::string>& parts) const
  {
    DataWrapperPtr res = this->mDataWrapper;

    for(auto& part : parts) {
      DataWrapperPtr tmp;
      tmp = DataWrapperPtr(res->getChildAt(part));
      if(!tmp) {
        break;
      }

      res = tmp;
    }

    return res;
  }

  DataProxy::DataWrapperPtr DataProxy::traverseWrite(const std::vector<std::string>& parts)
  {
    DataWrapperPtr res = this->mDataWrapper;

    for(auto& part : parts) {
      res = DataWrapperPtr(res->createChildAt(part));
    }

    return res;
  }

  bool dump(const DataProxy& value, const std::string& path, DataWrapper::WrappedType type)
  {
    std::ofstream os(path);
    if(!os)
      return false;

    os << dumps(value, type);
    os.close();
    return true;
  }

  std::string dumps(const DataProxy& value, DataWrapper::WrappedType type)
  {
    std::stringstream ss;
    if(type == DataWrapper::MSGPACK_OBJECT) {
      msgpack::pack(ss, value);
    } else {
      DataProxy dp = DataProxy::create(type);
      value.dump(dp);
      ss << dp.toString();
    }
    return ss.str();
  }

  std::tuple<DataProxy, bool> load(const std::string& path, DataWrapper::WrappedType type)
  {
    std::ifstream stream(path);
    if(!stream)
      return std::make_tuple(DataProxy(), false);

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
      return std::make_tuple(DataProxy(), false);
    }

    DataProxy res;
    try {
      res = loads(str, type);
    } catch(DecodeException e) {
      LOG(INFO) << e.what();
      success = false;
    }

    return std::make_tuple(res, success);
  }

  bool loads(DataProxy& dest, const std::string& value, DataWrapper::WrappedType type)
  {
    bool success = true;
    try {
      dest = loads(value, type);
    } catch(DecodeException e) {
      LOG(INFO) << e.what();
      success = false;
    }
    return success;
  }

  DataProxy loads(const std::string& s, DataWrapper::WrappedType type)
  {
    DataProxy res = DataProxy::create(type);
    if(type == DataWrapper::MSGPACK_OBJECT)
    {
      msgpack::object_handle oh =
              msgpack::unpack(s.data(), s.size());
      msgpack::object obj = oh.get();
      try {
        obj.convert(res);
      } catch (const std::bad_cast& e) {
        throw DecodeException(e.what());
      }
    } else {
      if(!res.fromString(s)) {
        std::stringstream ss;
        ss << " failed to create object of type " << type << " from string " << s;
        throw DecodeException(ss.str());
      }
    }

    return res;
  }

  DataProxy merge(const DataProxy& first, const DataProxy& second)
  {
    DataProxy base = first;
    mergeInto(base, second);
    return base;
  }

  void mergeInto(DataProxy& base, const DataProxy& child)
  {
    child.dump(base, DataProxy::Merge | DataProxy::ForceCopy);
  }
}

std::ostream & operator<<(std::ostream & os, Gsage::DataProxy const & dict){
  os << dumps(dict, Gsage::DataWrapper::JSON_OBJECT);
  return os;
}
