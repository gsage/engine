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

#include "Converters.h"

namespace Gsage {
  // -----------------------------------------------------------------------------

  bool FloatCaster::to(const FloatCaster::FromType& src, FloatCaster::Type& dst) const
  {
    dst = std::stof(src);
    return true;
  }

  const FloatCaster::FromType FloatCaster::from(const FloatCaster::Type& value) const
  {
    return std::to_string(value);
  }

  // -----------------------------------------------------------------------------

  bool IntCaster::to(const IntCaster::FromType& src, IntCaster::Type& dst) const
  {
    dst = std::stoi(src);
    return true;
  }

  const IntCaster::FromType IntCaster::from(const IntCaster::Type& value) const
  {
    return std::to_string(value);
  }

  // -----------------------------------------------------------------------------

  bool UIntCaster::to(const UIntCaster::FromType& src, UIntCaster::Type& dst) const
  {
    dst = static_cast<unsigned int>(std::stoul(src));
    return true;
  }

  const UIntCaster::FromType UIntCaster::from(const UIntCaster::Type& value) const
  {
    return std::to_string(value);
  }

  // -----------------------------------------------------------------------------

  bool UlongCaster::to(const UlongCaster::FromType& src, UlongCaster::Type& dst) const
  {
    dst = std::stoul(src);
    return true;
  }

  const UlongCaster::FromType UlongCaster::from(const UlongCaster::Type& value) const
  {
    return std::to_string(value);
  }

  // -----------------------------------------------------------------------------

  bool BoolCaster::to(const BoolCaster::FromType& src, BoolCaster::Type& dst) const
  {
    dst = src == "true";
    return true;
  }

  const BoolCaster::FromType BoolCaster::from(const BoolCaster::Type& value) const
  {
    return value ? "true" : "false";
  }

  // -----------------------------------------------------------------------------

  bool StringCaster::to(const StringCaster::FromType& src, StringCaster::Type& dst) const
  {
    dst = src;
    return true;
  }

  const StringCaster::FromType StringCaster::from(const StringCaster::Type& value) const
  {
    return value;
  }

  // -----------------------------------------------------------------------------

  bool CStrCaster::to(const CStrCaster::FromType& src, CStrCaster::Type& dst) const
  {
    dst = src.c_str();
    return true;
  }

  const CStrCaster::FromType CStrCaster::from(const CStrCaster::Type& value) const
  {
    return value;
  }

  // -----------------------------------------------------------------------------
}
