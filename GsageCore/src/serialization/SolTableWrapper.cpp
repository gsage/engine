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

#include "serialization/SolTableWrapper.h"

namespace Gsage {

  SolTableWrapper::SolTableWrapper(const sol::table& value)
    : DataWrapper(LUA_TABLE)
    , mObject(value)
    , mArrayMode(false)
  {
  }

  SolTableWrapper::SolTableWrapper()
    : DataWrapper(LUA_TABLE)
    , mArrayMode(false)
  {
  }

  SolTableWrapper::~SolTableWrapper() {
  };

  bool SolTableWrapper::putChild(const std::string& key, DataWrapper& value) {
    if(value.getType() != getType()) {
      return false;
    }
    mObject[key] = static_cast<SolTableWrapper&>(value).getObject();
    return true;
  }

  bool SolTableWrapper::putChild(int key, DataWrapper& value) {
    if(value.getType() != getType()) {
      return false;
    }
    mObject[correctIndex(key)] = static_cast<SolTableWrapper&>(value).getObject();
    return true;
  }

  DataWrapper* SolTableWrapper::createChildAt(const std::string& key)
  {
    // DataProxy will wrap it into shared pointer
    return new SolTableWrapper(mObject.create(key));
  }

  DataWrapper* SolTableWrapper::createChildAt(int key)
  {
    key = correctIndex(key);
    mObject[key] = mObject.create();
    sol::table t = mObject[key];
    // DataProxy will wrap it into shared pointer
    return new SolTableWrapper(t);
  }

  const DataWrapper* SolTableWrapper::getChildAt(const std::string& key) const
  {
    sol::optional<sol::object> child = mObject[key];
    if(!child) {
      return 0;
    }

    // DataProxy will wrap it into shared pointer
    return new SolTableWrapper(child.value());
  }

  const DataWrapper* SolTableWrapper::getChildAt(int key) const
  {
    sol::optional<sol::object> child = mObject[correctIndex(key)];
    if(!child) {
      return 0;
    }

    // DataProxy will wrap it into shared pointer
    return new SolTableWrapper(child.value());
  }

  DataWrapper::Type SolTableWrapper::getStoredType() const
  {
    return const_cast<SolTableWrapper*>(this)->getStoredType();
  }

  DataWrapper::Type SolTableWrapper::getStoredType()
  {
    bool isArray = false;
    switch(mObject.get_type()) {
      case sol::type::table:
        return mObject.size() != 0 ? Array : Object;
      case sol::type::boolean:
        return Bool;
      case sol::type::number:
        return Double;
      case sol::type::string:
        return String;
      case sol::type::thread:
        return Thread;
      case sol::type::function:
        return Function;
      case sol::type::userdata:
        return Userdata;
      case sol::type::lightuserdata:
        return Userdata;
      case sol::type::lua_nil:
        return Null;
      case sol::type::none:
        return Null;
    }
    return Null;
  }

  template<>
  int SolTableWrapper::correctIndex<int>(const int& index) const
  {
    int offset = 0;
    if(getStoredType() == Array || mArrayMode) {
      offset++;
    }
    return index + offset;
  }

  void SolTableWrapper::makeArray()
  {
    mArrayMode = true;
  }

  SolTableWrapper::iterator::iterator(sol::table& table, SolTableWrapper::iterator::Type iterType)
    : mTableIterator(iterType == BEGIN ? table.begin() : table.end())
    , mCurrentValue(new SolTableWrapper())
    , mTable(table)
  {
    DataWrapper::iterator::mCurrent = DataWrapper::iterator::value_type("", mCurrentValue);
    update();
  }

  SolTableWrapper::iterator::~iterator() {
    delete mCurrentValue;
  }

  void SolTableWrapper::iterator::update() {
    if(mTableIterator == mTable.end()) {
      return;
    }
    auto iter = *mTableIterator;

    if(mTable.size() > 0) {
      sol::optional<int> index = iter.first.as<int>();
      if(index) {
        mCurrent.first = std::to_string(index.value() - 1);
      } else {
        mCurrent.first = iter.first.as<std::string>();
      }
    } else {
      mCurrent.first = iter.first.as<std::string>();
    }
    mCurrentValue->mObject = iter.second.as<sol::table>();
    DataWrapper::iterator::mCurrent = DataWrapper::iterator::value_type(mCurrent.first, mCurrentValue);
  }

  void SolTableWrapper::iterator::increment()
  {
    ++mTableIterator;
    update();
  }

  bool SolTableWrapper::iterator::operator==(const SolTableWrapper::iterator::self_type& rhs)
  {
    return mTableIterator == ((const SolTableWrapper::iterator&)rhs).mTableIterator;
  }

  bool SolTableWrapper::iterator::operator!=(const SolTableWrapper::iterator::self_type& rhs)
  {
    return mTableIterator != ((const SolTableWrapper::iterator&)rhs).mTableIterator;
  }

}
