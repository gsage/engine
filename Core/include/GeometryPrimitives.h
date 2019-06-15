#ifndef _GeometryPrimitives_H_
#define _GeometryPrimitives_H_

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

#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Matrix3x3.hpp"
#include "Quaternion.hpp"
#include "Converters.h"

namespace Gsage {

  struct BoundingBox {
    enum Extent {
      EXTENT_NULL,
      EXTENT_FINITE,
      EXTENT_INFINITE
    };

    BoundingBox(Extent e)
      : extent(e)
    {
    }

    BoundingBox(Vector3 min, Vector3 max)
      : min(min), max(max)
      , extent(EXTENT_FINITE)
    {
    }

    bool contains(float x, float y, float z)
    {
      return x >= min.X && y >= min.Y && z >= min.Z && x <= max.X && y <= max.Y && z <= max.Z;
    }

    Extent extent;
    Vector3 min;
    Vector3 max;
  };

  namespace Geometry {
      enum RotationAxis {
        X_AXIS,
        Y_AXIS,
        Z_AXIS,
        NONE
      };

      enum TransformSpace {
        TS_WORLD,
        TS_LOCAL
      };
  }

  TYPE_CASTER(GsageVector2Caster, Gsage::Vector2, std::string)
  TYPE_CASTER(GsageVector3Caster, Gsage::Vector3, std::string)

}

#endif
