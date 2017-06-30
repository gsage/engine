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

#include "serialization/JsonValueWrapper.h"

namespace Gsage {

  JsonValueWrapper::JsonValueWrapper(const Json::Value& value)
    : DataWrapper(JSON_OBJECT)
    , mSelfAllocatedObject(true)
    , mObject(new Json::Value(value))
  {
  }

  JsonValueWrapper::JsonValueWrapper(Json::Value* value)
    : DataWrapper(JSON_OBJECT)
    , mSelfAllocatedObject(false)
    , mObject(value)
  {
  }

  JsonValueWrapper::JsonValueWrapper(bool allocate)
    : DataWrapper(JSON_OBJECT)
    , mObject(allocate ? new Json::Value() : 0)
    , mSelfAllocatedObject(allocate)
  {
  }

  JsonValueWrapper::~JsonValueWrapper()
  {
    if(mSelfAllocatedObject and mObject != 0) {
      delete mObject;
    }
  }

  template<>
  Json::ValueType JsonValueWrapper::getJsonType<int>() const {
    return Json::intValue;
  }

  template<>
  Json::ValueType JsonValueWrapper::getJsonType<unsigned int>() const {
    return Json::uintValue;
  }

  template<>
  Json::ValueType JsonValueWrapper::getJsonType<double>() const {
    return Json::realValue;
 }

  template<>
  Json::ValueType JsonValueWrapper::getJsonType<float>() const {
    return Json::realValue;
 }

  template<>
  Json::ValueType JsonValueWrapper::getJsonType<std::string>() const {
    return Json::stringValue;
  }

  template<>
  Json::ValueType JsonValueWrapper::getJsonType<bool>() const {
    return Json::booleanValue;
  }

  template<>
  bool JsonValueWrapper::readValue<int>(const Json::Value& v, int& dest) const
  {
    dest = v.asInt();
    return true;
  }

  template<>
  bool JsonValueWrapper::readValue<unsigned int>(const Json::Value& v, unsigned int& dest) const
  {
    dest = v.asUInt();
    return true;
  }

  template<>
  bool JsonValueWrapper::readValue<double>(const Json::Value& v, double& dest) const
  {
    dest = v.asDouble();
    return true;
  }

  template<>
  bool JsonValueWrapper::readValue<float>(const Json::Value& v, float& dest) const
  {
    dest = v.asFloat();
    return true;
  }

  template<>
  bool JsonValueWrapper::readValue<std::string>(const Json::Value& v, std::string& dest) const
  {
    dest = v.asString();
    return true;
  }

  template<>
  bool JsonValueWrapper::readValue<bool>(const Json::Value& v, bool& dest) const
  {
    dest = v.asBool();
    return true;
  }

  void JsonValueWrapper::put(const std::string& key, const std::string& value)
  {
    getObject()[key] = value;
  }

  void JsonValueWrapper::put(int key, const std::string& value)
  {
    getObject()[key] = value;
  }

  bool JsonValueWrapper::putChild(const std::string& key, DataWrapper& value)
  {
    if(value.getType() != getType()) {
      return false;
    }
    getObject()[key] = static_cast<JsonValueWrapper&>(value).getObject();
    return true;
  }

  bool JsonValueWrapper::putChild(int key, DataWrapper& value)
  {
    if(value.getType() != getType()) {
      return false;
    }
    getObject()[key] = static_cast<JsonValueWrapper&>(value).getObject();
    return true;
  }

  DataWrapper* JsonValueWrapper::createChildAt(const std::string& key)
  {
    getObject()[key] = Json::Value();
    // DataProxy will wrap it into shared pointer
    Json::Value& value = getObject()[key];
    return new JsonValueWrapper(&value);
  }

  DataWrapper* JsonValueWrapper::createChildAt(int key)
  {
    getObject()[key] = Json::Value();
    // DataProxy will wrap it into shared pointer
    Json::Value& value = getObject()[key];
    return new JsonValueWrapper(&value);
  }

  const DataWrapper* JsonValueWrapper::getChildAt(const std::string& key) const
  {
    if(getObject()[key].isNull()) {
      return 0;
    }
    const Json::Value& value = getObject()[key];
    return new JsonValueWrapper(value);
  }

  const DataWrapper* JsonValueWrapper::getChildAt(int key) const
  {
    if(getObject().size() <= key) {
      return 0;
    }
    const Json::Value& value = getObject()[key];
    return new JsonValueWrapper(value);
  }

  DataWrapper::Type JsonValueWrapper::getStoredType()
  {
    Type res = Null;
    switch(getObject().type()) {
      case Json::booleanValue:
        res = Type::Bool;
        break;
      case Json::intValue:
        res = Type::Int;
        break;
      case Json::uintValue:
        res = Type::UInt;
        break;
      case Json::realValue:
        res = Type::Double;
        break;
      case Json::stringValue:
        res = Type::String;
        break;
      case Json::objectValue:
        res = Type::Object;
        break;
      case Json::arrayValue:
        res = Type::Array;
        break;
      default:
        res = Type::Object;
    }
    return res;
  }

  std::string JsonValueWrapper::toString() const
  {
    Json::FastWriter writer;
    return writer.write(getObject());
  }

  bool JsonValueWrapper::fromString(const std::string& s)
  {
    Json::Reader reader;
    return reader.parse(s, getObject());
  }

#define _PRIMITIVE_TYPE_PUT(t) template<> void JsonValueWrapper::put<t>(const std::string& key, const t& value) { getObject()[key] = value; }\
                               template<> void JsonValueWrapper::put<t>(int key, const t& value) { getObject()[key] = value; }\
                               template<> void JsonValueWrapper::set<t>(const t& value) { Json::Value wrap(value); getObject().swap(wrap); }

  _PRIMITIVE_TYPE_PUT(int)
  _PRIMITIVE_TYPE_PUT(unsigned int)
  _PRIMITIVE_TYPE_PUT(double)
  _PRIMITIVE_TYPE_PUT(float)
  _PRIMITIVE_TYPE_PUT(bool)

#undef _PRIMITIVE_TYPE_PUT

  void JsonValueWrapper::set(const std::string& value)
  {
    Json::Value wrap(value); getObject().swap(wrap);
  }

  bool JsonValueWrapper::readString(const Json::Value& value, std::string& dest) const
  {
    if(value.isNull()) {
      return false;
    }

    if(Json::stringValue != value.type()) {
      switch(value.type()) {
        case Json::booleanValue:
          dest = typename TranslatorBetween<std::string, bool>::type().from(value.asBool());
          break;
        case Json::intValue:
          dest = typename TranslatorBetween<std::string, int>::type().from(value.asInt());
          break;
        case Json::uintValue:
          dest = typename TranslatorBetween<std::string, unsigned int>::type().from(value.asUInt());
          break;
        case Json::realValue:
          dest = typename TranslatorBetween<std::string, double>::type().from(value.asDouble());
          break;
        default:
          return false;
      }
    } else {
      dest = value.asString();
    }
    return true;
  }

  JsonValueWrapper::iterator::iterator(Json::Value::iterator wrappedIterator, Json::Value& object)
    : mIterator(wrappedIterator)
    , mObject(object)
    , mCurrentValue(new JsonValueWrapper())
  {
    DataWrapper::iterator::mCurrent = DataWrapper::iterator::value_type("", mCurrentValue);
    update();
  }

  JsonValueWrapper::iterator::~iterator() {
    delete mCurrentValue;
  }

  void JsonValueWrapper::iterator::update() {
    if(mIterator == mObject.end()) {
      return;
    }
    mCurrent.first = mIterator.key().asString();
    mCurrentValue->mObject = &(*mIterator);
  }

  void JsonValueWrapper::iterator::increment()
  {
    mIterator++;
    update();
  }

  bool JsonValueWrapper::iterator::operator==(const JsonValueWrapper::iterator::self_type& rhs) {
    auto iter = ((const JsonValueWrapper::iterator&)rhs).mIterator;
    return mIterator == ((const JsonValueWrapper::iterator&)rhs).mIterator;
  }

  bool JsonValueWrapper::iterator::operator!=(const JsonValueWrapper::iterator::self_type& rhs)
  {
    return mIterator != ((const JsonValueWrapper::iterator&)rhs).mIterator;
  }
}
