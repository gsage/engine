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

#include "serialization/DataWrapper.h"

namespace Gsage {

  DataWrapper::DataWrapper(WrappedType type)
    : mType(type)
  {
  }

  DataWrapper::~DataWrapper()
  {
  }

  DataWrapper::WrappedType DataWrapper::getType() const
  {
    return mType;
  }

  DataWrapper* DataWrapper::getChildAt(const std::string& key)
  {
    return const_cast<DataWrapper*>(const_cast<const DataWrapper*>(this)->getChildAt(key));
  }

  DataWrapper* DataWrapper::getChildAt(int key)
  {
    return const_cast<DataWrapper*>(const_cast<const DataWrapper*>(this)->getChildAt(key));
  }

  void DataWrapper::swap(const DataWrapper* other)
  {
    *this = *other;
  }

  std::string DataWrapper::toString() const
  {
    LOG(WARNING) << "To string is not supported for the type id: " << mType;
    return "";
  }

  bool DataWrapper::fromString(const std::string& s)
  {
    LOG(WARNING) << "From string is not supported for the type id: " << mType;
    return false;
  }

  void DataWrapper::makeArray()
  {
    // no-op by default
  }
}
