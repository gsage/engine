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
#include "Logger.h"

namespace Gsage {

  TileCache::TileCache()
    : mNavMesh(nullptr)
    , mNavQuery(nullptr)
    , mFilter(nullptr)
  {
  }

  TileCache::~TileCache()
  {
    if(mNavMesh != nullptr) {
      dtFreeNavMesh(mNavMesh);
    }
    if(mNavQuery != nullptr) {
      dtFreeNavMeshQuery(mNavQuery);
    }
	  mNavMesh = nullptr;
  }

  dtStatus TileCache::init(const dtNavMeshParams* params, DataProxy config)
  {
    if(mNavMesh != nullptr) {
      return DT_FAILURE;
    }

    mNavMesh = dtAllocNavMesh();
    if(!mNavMesh) {
      LOG(ERROR) << "Out of memory: mNavMesh";
      return DT_FAILURE;
    }

    dtStatus s = mNavMesh->init(params);
    if(dtStatusFailed(s)) {
      return s;
    }

    mNavQuery = dtAllocNavMeshQuery();
    if(!mNavQuery) {
      LOG(ERROR) << "Out of memory: mNavQuery";
    }

    mFilter = new dtQueryFilter();
    mFilter->setIncludeFlags(0xFFFF);    // Include all
    mFilter->setExcludeFlags(0);         // Exclude none
    // Area flags for polys to consider in search, and their cost
    //
    // TODO: allow configuring that
    mFilter->setAreaCost(RC_WALKABLE_AREA, 1.0f);

    // TODO: configure it properly
    // Set default size of box around points to look for nav polygons
    mExtents[0] = 100.0f; mExtents[1] = 100.0f; mExtents[2] = 100.0f;

    return mNavQuery->init(mNavMesh, config.get("navMeshQuery.maxNodes", 2048));
  }

  bool TileCache::load(const std::string& filepath)
  {
    // TODO
    return false;
  }

  bool TileCache::dump(const std::string& filepath)
  {
    // TODO
    return false;
  }

  void TileCache::merge(TileCache* tileCache)
  {
    // TODO
  }

  dtTileRef TileCache::getTileRefAt(int x, int y, int layer)
  {
    if(mNavMesh == nullptr) {
      return 0;
    }

    return mNavMesh->getTileRefAt(x, y, layer);
  }

  dtStatus TileCache::addTile(unsigned char* data, int dataSize, int flags, dtTileRef lastRef, dtTileRef* result)
  {
    if(mNavMesh == nullptr) {
      return DT_FAILURE;
    }

    return mNavMesh->addTile(data, dataSize, flags, lastRef, result);
  }

  dtStatus TileCache::removeTile(dtTileRef ref, unsigned char** data, int* dataSize)
  {
    if(mNavMesh == nullptr) {
      return DT_FAILURE;
    }

    return mNavMesh->removeTile(ref, data, dataSize);
  }

  Path3DPtr TileCache::findPath(const Gsage::Vector3& start, const Gsage::Vector3& end)
  {
    float s[3] = {(float)start.X, (float)start.Y, (float)start.Z};
    float e[3] = {(float)end.X, (float)end.Y, (float)end.Z};
    return findPath(s, e);
  }

  Path3DPtr TileCache::findPath(const float* start, const float* end)
  {
    dtStatus status;
    dtPolyRef startPoly;
    float startNearest[3];
    dtPolyRef endPoly;
    float endNearest[3];
    dtPolyRef polyPath[MAX_PATHPOLY];
    int nVertCount = 0;
    float straightPath[MAX_PATHVERT * 3];
    int nPathCount = 0;

    // find the start polygon
    status = mNavQuery->findNearestPoly(start, mExtents, mFilter, &startPoly, startNearest);
    if(dtStatusFailed(status)) {
      return nullptr;
    }

    // find the end polygon
    status = mNavQuery->findNearestPoly(end, mExtents, mFilter, &endPoly, endNearest) ;
    if(dtStatusFailed(status)) {
      return nullptr;
    }

    status = mNavQuery->findPath(startPoly, endPoly, startNearest, endNearest, mFilter, polyPath, &nPathCount, MAX_PATHPOLY);
    if(dtStatusFailed(status) || nPathCount == 0) {
      return nullptr;
    }

    status = mNavQuery->findStraightPath(startNearest, endNearest, polyPath, nPathCount, straightPath, NULL, NULL, &nVertCount, MAX_PATHVERT);
    if(dtStatusFailed(status) || nVertCount == 0) {
      return nullptr;
    }

    dtPolyRef currentPoly = startPoly;
    float* currentStart = startNearest;
    Path3D::Vector points;
    // At this point we have our path.  Copy it to the path store
    for(int i = 1; i < nVertCount; i++) {
      int index = i * 3;
      float point[3] = {straightPath[index], straightPath[index + 1], straightPath[index + 2]};

      if(i > 0) {
        points.emplace_back(point[0], point[1], point[2]);
      }
    }
    return std::make_shared<Path3D>(points);
  }

  bool TileCache::findNearestPointOnNavmesh(const Gsage::Vector3& point, Gsage::Vector3& dest)
  {
    dtPolyRef result;
    float resultPoint[3];

    float p[3] = {(float)point.X, (float)point.Y, (float)point.Z};
    dtStatus status = mNavQuery->findNearestPoly(p, mExtents, mFilter, &result, resultPoint);
    if(dtStatusFailed(status)) {
      return false;
    }

    dest.X = resultPoint[0];
    dest.Y = resultPoint[1];
    dest.Z = resultPoint[2];

    return true;
  }

  std::vector<Gsage::Vector3> TileCache::getPoints() const {
    std::vector<Gsage::Vector3> res;
    if(!mNavMesh) {
      return res;
    }
    for(int i = 0; i < mNavMesh->getMaxTiles(); ++i) {
      const dtMeshTile* tile = const_cast<const dtNavMesh*>(mNavMesh)->getTile(i);
      if (!tile->header) 
        continue;

      for (int i = 0; i < tile->header->polyCount; ++i)
      {
        const dtPoly* p = &tile->polys[i];
        if (p->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)	// Skip off-mesh links.
          continue;
        const dtPolyDetail* pd = &tile->detailMeshes[i];

        for (int j = 0; j < pd->triCount; ++j)
        {
          const unsigned char* t = &tile->detailTris[(pd->triBase+j) * 4];
          for (int k = 0; k < 3; ++k)
          {
            float* coordinates = nullptr;
            if (t[k] < p->vertCount)
              coordinates = &tile->verts[p->verts[t[k]]*3];
            else
              coordinates = &tile->detailVerts[(pd->vertBase+t[k]-p->vertCount)*3];
            res.emplace_back(coordinates[0], coordinates[1], coordinates[2]);
          }
        }
      }
    }
    return res;
  }

  float TileCache::getHeight(const Gsage::Vector3& position)
  {
    dtStatus status;
    dtPolyRef poly;
    float nearest[3] = {0.0f};
    float pos[3] = {(float)position.X, (float)position.Y, (float)position.Z};

    status = mNavQuery->findNearestPoly(pos, mExtents, mFilter, &poly, nearest);
    float height = -1;
    if(dtStatusFailed(status)) {
      return -1;
    }

    status = mNavQuery->getPolyHeight(poly, nearest, &height);
    if(dtStatusFailed(status)) {
      return -1;
    }
    return height;
  }

}
