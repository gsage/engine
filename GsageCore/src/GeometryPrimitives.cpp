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

#include "GeometryPrimitives.h"

namespace Gsage {

  bool GsageVector2Caster::to(const GsageVector2Caster::FromType& src, GsageVector2Caster::Type& dst) const
  {
    if(src.empty())
      return false;

    dst = Gsage::Vector2::Zero();
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
        stream >> dst.X;
      else if(i == 1)
        stream >> dst.Y;
      else
        break;

      stream.clear();
    }

    return true;
  }

  const GsageVector2Caster::FromType GsageVector2Caster::from(const GsageVector2Caster::Type& value) const
  {
    std::stringstream stream;
    stream << value.X << "," << value.Y;
    return stream.str();
  }

  // -----------------------------------------------------------------------------

  bool GsageVector3Caster::to(const GsageVector3Caster::FromType& src, GsageVector3Caster::Type& dst) const
  {
    if(src.empty())
      return false;

    dst = Gsage::Vector3::Zero();
    std::vector<std::string> values = split(src, ',');
    if(values.size() != 3)
    {
      return false;
    }

    std::stringstream stream;

    for(unsigned int i = 0; i < values.size(); i++)
    {
      stream << values[i];
      if(i == 0)
        stream >> dst.X;
      else if(i == 1)
        stream >> dst.Y;
      else if(i == 2)
        stream >> dst.Z;
      else
        break;

      stream.clear();
    }

    return true;
  }

  const GsageVector3Caster::FromType GsageVector3Caster::from(const GsageVector3Caster::Type& value) const
  {
    std::stringstream stream;
    stream << value.X << "," << value.Y << "," << value.Z;
    return stream.str();
  }

  // -----------------------------------------------------------------------------

}
