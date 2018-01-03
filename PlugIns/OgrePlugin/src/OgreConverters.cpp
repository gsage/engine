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

#include "OgreConverters.h"

namespace Gsage {
  inline float fromHex(const unsigned int& value, const int& offset)
  {
    return (value >> offset & 0xFF) / 255.0f;
  }

  inline int toHex(const float& value, const int& offset)
  {
    return (int)(value * 0xFF) << offset;
  }

  // -----------------------------------------------------------------------------

  bool OgreDegreeCaster::to(const OgreDegreeCaster::FromType& src, OgreDegreeCaster::Type& dst) const
  {
    if(src.empty())
      return false;

    dst = Ogre::Degree((float)atof(src.c_str()));
    return true;
  }

  const OgreDegreeCaster::FromType OgreDegreeCaster::from(const OgreDegreeCaster::Type& value) const
  {
    return std::to_string(value.valueDegrees());
  }

  // -----------------------------------------------------------------------------

  bool OgreColourValueCaster::to(const OgreColourValueCaster::FromType& src, OgreColourValueCaster::Type& dst) const
  {
    if(src.empty())
      return false;
    Ogre::ColourValue res;
    unsigned int value;
    try
    {
      value = std::stoul(src, nullptr, 16);
    }
    catch(std::invalid_argument)
    {
      return false;
    }

    dst.a = fromHex(value, 24);
    dst.r = fromHex(value, 16);
    dst.g = fromHex(value, 8);
    dst.b = fromHex(value, 0);
    return true;
  }

  const OgreColourValueCaster::FromType OgreColourValueCaster::from(const OgreColourValueCaster::Type& value) const
  {
    std::stringstream stream;
    stream << "0x" <<std::hex << (toHex(value.a, 24) | toHex(value.r, 16) | toHex(value.g, 8) | toHex(value.b, 0));
    return stream.str();
  }

  // -----------------------------------------------------------------------------

  bool OgreVector3Caster::to(const OgreVector3Caster::FromType& src, OgreVector3Caster::Type& dst) const
  {
    if(src.empty())
      return false;

    dst = Ogre::Vector3::ZERO;
    std::vector<std::string> values = split(src, ',');
    if(values.size() == 0)
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
      else if(i == 2)
        stream >> dst.z;
      else
        break;

      stream.clear();
    }

    return true;
  }

  const OgreVector3Caster::FromType OgreVector3Caster::from(const OgreVector3Caster::Type& value) const
  {
    std::stringstream stream;
    stream << value.x << "," << value.y << "," << value.z;
    return stream.str();
  }

  // -----------------------------------------------------------------------------

  bool OgreQuaternionCaster::to(const OgreQuaternionCaster::FromType& src, OgreQuaternionCaster::Type& dst) const
  {
    if(src.empty())
      return false;

    std::vector<std::string> values = split(src, ',');

    if(values.size() == 0)
    {
      return false;
    }

    std::stringstream stream;

    for(unsigned int i = 0; i < values.size(); i++)
    {
      stream << values[i];
      if(i == 0)
        stream >> dst.w;
      else if(i == 1)
        stream >> dst.x;
      else if(i == 2)
        stream >> dst.y;
      else if(i == 3)
        stream >> dst.z;
      else
        break;

      stream.clear();
    }

    return true;
  }

  const OgreQuaternionCaster::FromType OgreQuaternionCaster::from(const OgreQuaternionCaster::Type& value) const
  {
    std::stringstream stream;
    stream << value.w << "," << value.x << "," << value.y << "," << value.z;
    return stream.str();
  }

  // -----------------------------------------------------------------------------

  bool OgreFloatRectCaster::to(const OgreFloatRectCaster::FromType& src, OgreFloatRectCaster::Type& dst) const
  {
    if(src.empty())
      return false;

    std::vector<std::string> values = split(src, ',');

    if(values.size() == 0)
    {
      return false;
    }

    std::stringstream stream;

    for(unsigned int i = 0; i < values.size(); i++)
    {
      stream << values[i];
      if(i == 0)
        stream >> dst.left;
      else if(i == 1)
        stream >> dst.top;
      else if(i == 2)
        stream >> dst.right;
      else if(i == 3)
        stream >> dst.bottom;
      else
        break;

      stream.clear();
    }

    return true;
  }

  const OgreFloatRectCaster::FromType OgreFloatRectCaster::from(const OgreFloatRectCaster::Type& value) const
  {
    std::stringstream stream;
    stream << value.left << "," << value.top << "," << value.right << "," << value.bottom;
    return stream.str();
  }

  // -----------------------------------------------------------------------------

  bool OgrePixelFormatCaster::to(const OgrePixelFormatCaster::FromType& src, OgrePixelFormatCaster::Type& dst) const
  {
    if(src.empty())
      return false;

    int value;
    try {
      value = std::stoi(src);
    } catch (...) {
      return false;
    }
    dst = static_cast<Ogre::PixelFormat>(value);
    return true;
  }

  const OgrePixelFormatCaster::FromType OgrePixelFormatCaster::from(const OgrePixelFormatCaster::Type& value) const
  {
    return std::to_string(value);
  }

  // -----------------------------------------------------------------------------

  bool RenderTargetTypeCaster::to(const RenderTargetTypeCaster::FromType& src, RenderTargetTypeCaster::Type& dst) const
  {
    if(src.empty())
      return false;

    if(src == "rtt") {
      dst = RenderTarget::Rtt;
    } else if(src == "window") {
      dst = RenderTarget::Window;
    } else {
      return false;
    }
    return true;
  }

  const RenderTargetTypeCaster::FromType RenderTargetTypeCaster::from(const RenderTargetTypeCaster::Type& value) const
  {
    switch(value) {
      case RenderTarget::Rtt:
        return "rtt";
      case RenderTarget::Window:
        return "window";
      default:
        return "";
    }
    return "";
  }

}
