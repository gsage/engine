#ifndef _Path_H_
#define _Path_H_

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
#include "Converters.h"
#include "Serializable.h"
#include <vector>

namespace Gsage {
  /**
   * Represents Path iterator
   */
  template<class C>
  class Path : Serializable<Path<C>>
  {
    public:
      typedef std::vector<C> Vector;

      Path() : mCurrent(-1)
      {
      };

      Path(Vector points)
        : mPoints(points)
        , mCurrent(-1)
      {
      };

      virtual ~Path() {};

      /**
       * Iterate to the next point
       *
       * @return true if there are any points left
       */
      bool next()
      {
        mCurrent++;
        return mCurrent < mPoints.size();
      }

      /**
       * Reset iterator
       */
      void reset()
      {
        mCurrent = -1;
      }

      /**
       * Get current point
       *
       * @return current element of points vector
       */
      C* current()
      {
        if(mCurrent == -1) {
          next();
        }

        if(mCurrent < mPoints.size()) {
          return &mPoints[mCurrent];
        }

        return nullptr;
      }

      C* end()
      {
        if(mPoints.size() == 0) {
          return nullptr;
        }

        return &mPoints[mPoints.size() - 1];
      }

      /**
       * Updates underlying points vector
       *
       * @param points New points vector
       */
      void update(Vector points) {
        mPoints = points;
        reset();
      }

      virtual bool dump(DataProxy& dest)
      {
        for(auto point : mPoints) {
          dest.push(point);
        }
        return true;
      }

      virtual bool read(const DataProxy& src)
      {
        mPoints.clear();
        for(auto pair : src) {
          auto point = pair.second.getValue<C>();
          if(!point.second) {
            LOG(WARNING) << "Skipped malfomed point in the path " << pair.second.as<std::string>();
            continue;
          }

          mPoints.push_back(point.first);
        }

        reset();
        return true;
      }
    private:
      Vector mPoints;
      int mCurrent;
  };

  /**
   * 3D Path
   */
  typedef Path<Gsage::Vector2> Path2D;

  /**
   * 3D Path
   */
  typedef Path<Gsage::Vector3> Path3D;

  typedef std::shared_ptr<Path3D> Path3DPtr;
}

#endif
