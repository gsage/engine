#ifndef _RecastWrapper_H_
#define _RecastWrapper_H_

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

#include "TileCache.h"
#include <Recast.h>
#include <ChunkyTriMesh.h>
#include "systems/RenderSystem.h"
#include "DataProxy.h"

namespace Gsage {
  /**
   * Implementation of rcContext
   */
  class RecastContext : public rcContext
  {
    virtual void doResetLog();
    virtual void doLog(const rcLogCategory category, const char* msg, const int len);
    virtual void doResetTimers();
    virtual void doStartTimer(const rcTimerLabel label);
    virtual void doStopTimer(const rcTimerLabel label);
    virtual int doGetAccumulatedTime(const rcTimerLabel label) const;
  };

  /**
   * Used to build Recast meshes
   */
  class RecastWrapper
  {
    public:
      RecastWrapper();
      virtual ~RecastWrapper();

      /**
       * Build tiled navigation mesh
       *
       * @param geom Input geom to use for tile cache build
       * @param renderer Render system to get all entities from
       *
       * @return tile cache if succeed
       */
      TileCachePtr buildTileCache(GeomPtr geom, DataProxy options);

    private:
      /**
       * Used to cache rcChunkyTriMesh
       */
      class GeomWrapper
      {
        public:
          GeomWrapper(GeomPtr geom, int trisPerChunk);
          virtual ~GeomWrapper();
          /**
           * Generate and cache chunky tri mesh
           */
          rcChunkyTriMesh* getChunkyMesh();

          /**
           * Retrieves the vertices stored within this Geom. The verts are an array of floats in which each
           * subsequent three floats are in order the x, y and z coordinates of a vert. The size of this array is
           * always a multiple of three and is exactly 3*getVertCount().
           **/
          Geom::Verts getVerts();

          /**
           * Retrieves the tris stored in this Geom.
           * A tri is defined by a sequence of three indexes which refer to an index position in the getVerts() array.
           * Similar to getVerts, the size of this array is a multitude of 3 and is exactly 3*getTriCount().
           **/
          Geom::Tris getTris();

          /**
           * The axis aligned bounding box minimum of this input Geom.
           **/
          float* getMeshBoundsMin();

          /**
           * The axis aligned bounding box maximum of this input Geom.
           **/
          float* getMeshBoundsMax();
        private:
          GeomPtr mGeom;
          rcChunkyTriMesh* mChunkyMesh;
          int mTrisPerChunk;
      };

      unsigned char* buildTileMesh(GeomWrapper& geom, DataProxy options, const int tx, const int ty, const float* bmin, const float* bmax, int& dataSize);
  };
}

#endif
