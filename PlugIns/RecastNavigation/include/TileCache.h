#ifndef _TileCache_H_
#define _TileCache_H_

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

#include <memory>
#include <string>

#include "Recast.h"
#include "DetourAlloc.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "DetourStatus.h"

#include "Path.h"

#define MAX_PATHPOLY      256 // max number of polygons in a path
#define MAX_PATHVERT      512 // most verts in a path

namespace Gsage {

  class TileCache
  {
    public:
      TileCache();
      virtual ~TileCache();

      /**
       * Init tile cache
       *
       * @param params Nav mesh parameters
       * @param config Additional parameters
       */
      dtStatus init(const dtNavMeshParams* params, DataProxy config);

      /**
       * Load tile cache data from file
       *
       * @param path Data path
       *
       * @return true if succeed
       */
      bool load(const std::string& path);

      /**
       * Dump tile cache data to file
       *
       * @param path Data path
       *
       * @return true if succeed
       */
      bool dump(const std::string& path);

      /**
       * Gets the tile at the specified grid location.
       *  @param[in]	x		The tile's x-location. (x, y, layer)
       *  @param[in]	y		The tile's y-location. (x, y, layer)
       *  @param[in]	layer	The tile's layer. (x, y, layer)
       * @return The tile, or null if the tile does not exist.
       */
      dtTileRef getTileRefAt(int x, int y, int layer);

      /**
       * Merge another tile cache into existing
       *
       * @param tileCache tile cache to merge
       */
      void merge(TileCache* tileCache);

      /**
       * Adds a tile to the navigation mesh.
       *  @param[in]		data		Data for the new tile mesh. (See: #dtCreateNavMeshData)
       *  @param[in]		dataSize	Data size of the new tile mesh.
       *  @param[in]		flags		Tile flags. (See: #dtTileFlags)
       *  @param[in]		lastRef		The desired reference for the tile. (When reloading a tile.) [opt] [Default: 0]
       *  @param[out]	result		The tile reference. (If the tile was succesfully added.) [opt]
       * @return The status flags for the operation
       */
      dtStatus addTile(unsigned char* data, int dataSize, int flags, dtTileRef lastRef, dtTileRef* result);

      /**
       * Removes the specified tile from the navigation mesh.
       *
       * @param[in]		ref			The reference of the tile to remove.
       * @param[out]	data		Data associated with deleted tile.
       * @param[out]	dataSize	Size of the data associated with deleted tile.
       *
       * @return The status flags for the operation.
       */
      dtStatus removeTile(dtTileRef ref, unsigned char** data, int* dataSize);

      /**
       * Find path using tile cache
       *
       * @param start Start point
       * @param end End point
       */
      Path3DPtr findPath(const Gsage::Vector3& start, const Gsage::Vector3& end);

      /**
       * Find path using tile cache
       *
       * @param start Start point
       * @param end End point
       */
      Path3DPtr findPath(const float* start, const float* end);

      /**
       * Find nearest point on navmesh
       *
       * @param point Source point
       * @param dest Nearest point
       */
      bool findNearestPointOnNavmesh(const Gsage::Vector3& point, Gsage::Vector3& dest);

      /**
       * Get poly height at point
       *
       * @param pos Position
       *
       * @return height
       */
      float getHeight(const Gsage::Vector3& pos);

      // utility methods

      /**
       * Get recast navmesh points, used for visualizations
       *
       * @return list of Gsage::Vector3 points
       */
      std::vector<Gsage::Vector3> getPoints() const;
    private:
      dtNavMesh* mNavMesh;
      dtNavMeshQuery* mNavQuery;
      dtQueryFilter* mFilter;
      float mExtents[3];
      float* mNormals;
  };

  typedef std::shared_ptr<TileCache> TileCachePtr;
}

#endif
