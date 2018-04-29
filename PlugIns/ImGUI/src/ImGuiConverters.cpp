/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2018 Artem Chernyshev and contributors

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

#include "ImGuiConverters.h"

namespace Gsage {

  inline float fromHex(unsigned int value, int offset)
  {
    return ((value >> offset) & 0xFF) / 255.0f;
  }

  inline int toHex(float value, int offset)
  {
    return (int)(value * 0xFF) << offset;
  }

  // -----------------------------------------------------------------------------

  bool ImVec4TypeCaster::to(const ImVec4TypeCaster::FromType& src, ImVec4TypeCaster::Type& dst) const
  {
    if(src.empty())
      return false;
    ImVec4 res;
    unsigned long value;
    try
    {
      value = std::stoul(src, nullptr, 16);
    }
    catch(std::invalid_argument)
    {
      return false;
    }

    dst.w = fromHex(value, 24);
    dst.x = fromHex(value, 16);
    dst.y = fromHex(value, 8);
    dst.z = fromHex(value, 0);
    return true;
  }

  const ImVec4TypeCaster::FromType ImVec4TypeCaster::from(const ImVec4TypeCaster::Type& value) const
  {
    std::stringstream stream;
    stream << "0x" <<std::hex << (toHex(value.w, 24) | toHex(value.x, 16) | toHex(value.y, 8) | toHex(value.z, 0));
    return stream.str();
  }

  // -----------------------------------------------------------------------------

  bool ImVec2TypeCaster::to(const ImVec2TypeCaster::FromType& src, ImVec2TypeCaster::Type& dst) const
  {
    if(src.empty())
      return false;

    std::vector<std::string> values = split(src, ',');
    if(values.size() != 2)
    {
      return false;
    }
    std::stringstream stream;

    for(unsigned int i = 0; i < values.size(); i++)
    {
      stream << values[i];
      if(i == 0)
        stream >> dst.x;
      else if(i == 1)
        stream >> dst.y;
      else
        break;

      stream.clear();
    }

    return true;
  }

  const ImVec2TypeCaster::FromType ImVec2TypeCaster::from(const ImVec2TypeCaster::Type& value) const
  {
    std::stringstream stream;
    stream << value.x << "," << value.y;
    return stream.str();
  }
}
