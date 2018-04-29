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

#include "RecastWrapper.h"
#include "Logger.h"
#include <vector>
#include "DetourNavMeshBuilder.h"

namespace Gsage {

  inline unsigned int nextPow2(unsigned int v)
  {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
  }

  inline unsigned int ilog2(unsigned int v)
  {
    unsigned int r;
    unsigned int shift;
    r = (v > 0xffff) << 4; v >>= r;
    shift = (v > 0xff) << 3; v >>= shift; r |= shift;
    shift = (v > 0xf) << 2; v >>= shift; r |= shift;
    shift = (v > 0x3) << 1; v >>= shift; r |= shift;
    r |= (v >> 1);
    return r;
  }

  void RecastContext::doResetLog()
  {
  }

  void RecastContext::doLog(const rcLogCategory category, const char* msg, const int len)
  {
    std::stringstream ss;
    for(int i = 0; i < len; ++i) {
      ss << msg[i];
    }

    switch(category) {
      case RC_LOG_ERROR:
        LOG(ERROR) << ss.str();
        break;
      default:
        LOG(DEBUG) << ss.str();
        break;
    }
  }

  void RecastContext::doResetTimers()
  {
  }

  void RecastContext::doStartTimer(const rcTimerLabel label)
  {
  }

  void RecastContext::doStopTimer(const rcTimerLabel label)
  {
  }

  int RecastContext::doGetAccumulatedTime(const rcTimerLabel label) const
  {
    return 0;
  }

  RecastWrapper::RecastWrapper()
  {
  }

  RecastWrapper::~RecastWrapper()
  {
  }

  TileCachePtr RecastWrapper::buildTileCache(GeomPtr geom, DataProxy config)
  {
    const float* bmin = geom->getMeshBoundsMin();
    const float* bmax = geom->getMeshBoundsMax();

    int gw = 0, gh = 0;
    float cellSize;
    int tileSize, maxTiles, maxPolys;

    config.read("cellSize", cellSize);
    config.read("tileSize", tileSize);

    rcCalcGridSize(bmin, bmax, cellSize, &gw, &gh);

    const int ts = tileSize;
    const int tw = (gw + ts-1) / ts;
    const int th = (gh + ts-1) / ts;
    const float tcs = tileSize * cellSize;

    // Max tiles and max polys affect how the tile IDs are caculated.
		// There are 22 bits available for identifying a tile and a polygon.
		int tileBits = rcMin((int)ilog2(nextPow2(tw * th)), 14);
		if (tileBits > 14) {
      tileBits = 14;
    }
		int polyBits = 22 - tileBits;
		maxTiles = 1 << tileBits;
		maxPolys = 1 << polyBits;

    float lastBmin[3] = {0.0f};
    float lastBmax[3] = {0.0f};

    int trisPerChunk = 256;

    config.read("trisPerChunk", trisPerChunk);

    GeomWrapper geomWrapper(std::move(geom), trisPerChunk);

    TileCachePtr res = std::make_shared<TileCache>();
    dtNavMeshParams params;
    memset(&params, 0, sizeof(params));
    params.tileWidth = tileSize * cellSize;
    params.tileHeight = tileSize * cellSize;
    params.maxTiles = maxTiles;
    params.maxPolys = maxPolys;

    res->init(&params, config);

    for (int y = 0; y < th; ++y)
    {
      for (int x = 0; x < tw; ++x)
      {
        lastBmin[0] = bmin[0] + x * tcs;
        lastBmin[1] = bmin[1];
        lastBmin[2] = bmin[2] + y * tcs;

        lastBmax[0] = bmin[0] + (x + 1) * tcs;
        lastBmax[1] = bmax[1];
        lastBmax[2] = bmin[2] + (y + 1) * tcs;

        int dataSize = 0;
        unsigned char* data = buildTileMesh(geomWrapper, config, x, y, lastBmin, lastBmax, dataSize);

        // Remove any previous data (navmesh owns and deletes the data).
				res->removeTile(res->getTileRefAt(x, y, 0), 0, 0);
        if (data == nullptr) {
          continue;
        }

				// Let the navmesh own the data.
				dtStatus status = res->addTile(data, dataSize, DT_TILE_FREE_DATA, 0, 0);
				if (dtStatusFailed(status))
					dtFree(data);
      }
    }

    return res;
  }

  unsigned char* RecastWrapper::buildTileMesh(GeomWrapper& geom, DataProxy config, const int tx, const int ty, const float* bmin, const float* bmax, int& dataSize)
  {
    rcConfig cfg;
    memset(&cfg, 0, sizeof(cfg));

    rcContext* ctx = new RecastContext();

    //
    // Step 1. Initialize build config.
    //

    config.read("cellSize", cfg.cs);
    config.read("cellHeight", cfg.ch);
    config.read("walkableSlopeAngle", cfg.walkableSlopeAngle);
    config.read("walkableHeight", cfg.walkableHeight);
    config.read("walkableClimb", cfg.walkableClimb);
    config.read("walkableRadius", cfg.walkableRadius);
    config.read("maxEdgeLen", cfg.maxEdgeLen);
    config.read("maxSimplificationError", cfg.maxSimplificationError);
    config.read("minRegionArea", cfg.minRegionArea);
    config.read("mergeRegionArea", cfg.mergeRegionArea);
    config.read("maxVertsPerPoly", cfg.maxVertsPerPoly);
    config.read("tileSize", cfg.tileSize);
    config.read("detailSampleDist", cfg.detailSampleDist);
    config.read("detailSampleMaxError", cfg.detailSampleMaxError);

    cfg.borderSize = cfg.walkableRadius + 3;

    if(bmin != nullptr) {
      rcVcopy(cfg.bmin, bmin);
    } else {
      rcVcopy(cfg.bmin, geom.getMeshBoundsMin());
    }

    if(bmax != nullptr) {
      rcVcopy(cfg.bmax, bmax);
    } else {
  	  rcVcopy(cfg.bmax, geom.getMeshBoundsMax());
    }

    cfg.width = cfg.tileSize + cfg.borderSize * 2;
	  cfg.height = cfg.tileSize + cfg.borderSize * 2;

    cfg.bmin[0] -= cfg.borderSize * cfg.cs;
    cfg.bmin[2] -= cfg.borderSize * cfg.cs;
    cfg.bmax[0] += cfg.borderSize * cfg.cs;
    cfg.bmax[2] += cfg.borderSize * cfg.cs;

    float* verts;
    int* tris;
    int nverts, ntris;
    int tileTriCount = 0;

    std::tie(verts, nverts) = geom.getVerts();
    std::tie(tris, ntris) = geom.getTris();

    // Recast always multiplies current index by 3 so it should have ntris 3 times smaller
    ntris /= 3;

    LOG(INFO) << "Building navigation:";
    LOG(INFO) << "\tDimensions: " << cfg.width << " x " << cfg.height << " cells";
    LOG(INFO) << "\tConfig: " << dumps(config, DataWrapper::JSON_OBJECT);
    LOG(INFO) << "\tGeom: " << nverts/1000.0 << "K verts, " << ntris/1000.0 << "K tris";


    rcChunkyTriMesh* chunkyMesh = geom.getChunkyMesh();

    //
    // Step 2. Rasterize input polygon soup.
    //

    rcHeightfield* solid = rcAllocHeightfield();
    if(!solid) {
      LOG(ERROR) << "Out of memory 'solid'";
      return nullptr;
    }

    if (!rcCreateHeightfield(ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch))
    {
      LOG(ERROR) << "Could not create solid heightfield";
      return nullptr;
    }

    // Allocate array that can hold triangle flags.
    // If you have multiple meshes you need to process, allocate
    // and array which can hold the max number of triangles you need to process.
    unsigned char* triareas = new unsigned char[chunkyMesh->maxTrisPerChunk];
    if (!triareas)
    {
      LOG(ERROR) << "Out of memory 'triareas'" << chunkyMesh->maxTrisPerChunk;
      return nullptr;
    }

    float tbmin[2], tbmax[2];
    tbmin[0] = cfg.bmin[0];
    tbmin[1] = cfg.bmin[2];
    tbmax[0] = cfg.bmax[0];
    tbmax[1] = cfg.bmax[2];

    int cid[512];// TODO: Make grow when returning too many items.
    const int ncid = rcGetChunksOverlappingRect(chunkyMesh, tbmin, tbmax, cid, 512);
    if (!ncid)
      return 0;

    tileTriCount = 0;

    for (int i = 0; i < ncid; ++i)
    {
      const rcChunkyTriMeshNode& node = chunkyMesh->nodes[cid[i]];
      const int* ctris = &chunkyMesh->tris[node.i * 3];
      const int nctris = node.n;

      tileTriCount += nctris;

      memset(triareas, 0, nctris*sizeof(unsigned char));
      rcMarkWalkableTriangles(ctx, cfg.walkableSlopeAngle,
          verts, nverts, ctris, nctris, triareas);

      if (!rcRasterizeTriangles(ctx, verts, nverts, ctris, triareas, nctris, *solid, cfg.walkableClimb))
      {
        LOG(ERROR) << "Failed to rasterize triangles";
        return nullptr;
      }
    }

    delete [] triareas;

    //
    // Step 3. Filter walkables surfaces.
    //

    // Once all geometry is rasterized, we do initial pass of filtering to
    // remove unwanted overhangs caused by the conservative rasterization
    // as well as filter spans where the character cannot possibly stand.
    if (config.get("filterLowHangingObstacles", true))
      rcFilterLowHangingWalkableObstacles(ctx, cfg.walkableClimb, *solid);
    if (config.get("filterLedgeSpans", true))
      rcFilterLedgeSpans(ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
    if (config.get("filterWalkableLowHeightSpans", true))
      rcFilterWalkableLowHeightSpans(ctx, cfg.walkableHeight, *solid);

    //
    // Step 4. Partition walkable surface to simple regions.
    //

    // Compact the heightfield so that it is faster to handle from now on.
    // This will result more cache coherent data as well as the neighbours
    // between walkable cells will be calculated.
    rcCompactHeightfield* chf = rcAllocCompactHeightfield();
    if (!chf)
    {
      LOG(ERROR) << "Out of memory 'compact heightfield'";
      return nullptr;
    }

    if (!rcBuildCompactHeightfield(ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf))
    {
      LOG(ERROR) << "Could not build compact data";
      return nullptr;
    }

    rcFreeHeightField(solid);

    // Erode the walkable area by agent radius.
    if (!rcErodeWalkableArea(ctx, cfg.walkableRadius, *chf))
    {
      LOG(ERROR) << "Could not erode walkable area";
      return nullptr;
    }

    // TODO:
    // (Optional) Mark areas.
    //const ConvexVolume* vols = m_geom->getConvexVolumes();
    //for (int i  = 0; i < m_geom->getConvexVolumeCount(); ++i)
    //  rcMarkConvexPolyArea(m_ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);

    // Prepare for region partitioning, by calculating distance field along the walkable surface.
    if (!rcBuildDistanceField(ctx, *chf))
    {
      LOG(ERROR) << "Could not build distance field";
      return nullptr;
    }

    // Partition the walkable surface into simple regions without holes.
    if (!rcBuildRegions(ctx, *chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea))
    {
      LOG(ERROR) << "Could not build watershed regions";
      return nullptr;
    }

    //
    // Step 5. Trace and simplify region contours.
    //

    // Create contours.
    rcContourSet* cset = rcAllocContourSet();
    if (!cset)
    {
      LOG(ERROR) << " Out of memory 'cset'";
      return nullptr;
    }

    if (!rcBuildContours(ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset))
    {
      LOG(ERROR) << "Could not create contours";
      return nullptr;
    }

    // Build polygon navmesh from the contours.
    rcPolyMesh* pmesh = rcAllocPolyMesh();
    if (!pmesh)
    {
      LOG(ERROR) << "Out of memory 'pmesh'.";
      return nullptr;
    }
    if (!rcBuildPolyMesh(ctx, *cset, cfg.maxVertsPerPoly, *pmesh))
    {
      LOG(ERROR) << "Could not triangulate contours.";
      return nullptr;
    }

    //
    // Step 6. Create detail mesh which allows to access approximate height on each polygon.
    //

    rcPolyMeshDetail* dmesh = rcAllocPolyMeshDetail();
    if (!dmesh)
    {
      LOG(ERROR) << "Out of memory 'pmdtl'.";
      return nullptr;
    }

    if (!rcBuildPolyMeshDetail(ctx, *pmesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh))
    {
      LOG(ERROR) << "Could not build detail mesh.";
      return nullptr;
    }

    rcFreeCompactHeightfield(chf);
    chf = 0;
    rcFreeContourSet(cset);
    cset = 0;

    unsigned char* navData = 0;
    int navDataSize = 0;
    float tileMemUsage = 0.0f;

    if (cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
    {
      if (pmesh->nverts >= 0xffff)
      {
        // The vertex indices are ushorts, and cannot point to more than 0xffff vertices.
        LOG(ERROR) << "Too many vertices per tile " << pmesh->nverts << " (max: " << 0xffff << ")";
        return nullptr;
      }

      // Update poly flags from areas.
      // TODO:
      for (int i = 0; i < pmesh->npolys; ++i)
      {
        if (pmesh->areas[i] == RC_WALKABLE_AREA) {
          pmesh->flags[i] = 1;
        }

      }

      dtNavMeshCreateParams params;
      memset(&params, 0, sizeof(params));
      params.verts = pmesh->verts;
      params.vertCount = pmesh->nverts;
      params.polys = pmesh->polys;
      params.polyAreas = pmesh->areas;
      params.polyFlags = pmesh->flags;
      params.polyCount = pmesh->npolys;
      params.nvp = pmesh->nvp;
      params.detailMeshes = dmesh->meshes;
      params.detailVerts = dmesh->verts;
      params.detailVertsCount = dmesh->nverts;
      params.detailTris = dmesh->tris;
      params.detailTriCount = dmesh->ntris;
      /*
      params.offMeshConVerts = geom->getOffMeshConnectionVerts();
      params.offMeshConRad = geom->getOffMeshConnectionRads();
      params.offMeshConDir = geom->getOffMeshConnectionDirs();
      params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
      params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
      params.offMeshConUserID = m_geom->getOffMeshConnectionId();
      params.offMeshConCount = m_geom->getOffMeshConnectionCount();
      */
      config.read("agentHeight", params.walkableHeight);
      config.read("agentMaxClimb", params.walkableClimb);
      params.tileX = tx;
      params.tileY = ty;
      params.tileLayer = 0;
      rcVcopy(params.bmin, pmesh->bmin);
      rcVcopy(params.bmax, pmesh->bmax);
      params.cs = cfg.cs;
      params.ch = cfg.ch;
      params.buildBvTree = true;

      if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
      {
        LOG(ERROR) << "Could not build Detour navmesh.";
        return nullptr;
      }
    }
    tileMemUsage = navDataSize/1024.0f;

    LOG(INFO) << "Nav mesh mem usage: " << tileMemUsage;

    dataSize = navDataSize;

    delete ctx;
    return navData;
  }

  RecastWrapper::GeomWrapper::GeomWrapper(GeomPtr geom, int trisPerChunk)
    : mChunkyMesh(nullptr)
    , mGeom(std::move(geom))
    , mTrisPerChunk(trisPerChunk)
  {
  }

  RecastWrapper::GeomWrapper::~GeomWrapper()
  {
    if(mChunkyMesh != nullptr) {
      delete mChunkyMesh;
    }
  }

  rcChunkyTriMesh* RecastWrapper::GeomWrapper::getChunkyMesh()
  {
    if(mChunkyMesh == nullptr) {
      mChunkyMesh = new rcChunkyTriMesh();
      if (!mChunkyMesh)
      {
        LOG(ERROR) << "buildTiledNavigation: Out of memory 'chunkyMesh'.";
        mChunkyMesh = nullptr;
        return nullptr;
      }

      float* verts;
      int* tris;
      int nverts, ntris;
      std::tie(verts, nverts) = mGeom->getVerts();
      std::tie(tris, ntris) = mGeom->getTris();

      if (!rcCreateChunkyTriMesh(verts, tris, ntris / 3, mTrisPerChunk, mChunkyMesh))
      {
        LOG(ERROR) << "buildTiledNavigation: Failed to build chunky mesh.";
        mChunkyMesh = nullptr;
        return nullptr;
      }
    }

    return mChunkyMesh;
  }

  Geom::Verts RecastWrapper::GeomWrapper::getVerts()
  {
    return mGeom->getVerts();
  }

  Geom::Tris RecastWrapper::GeomWrapper::getTris()
  {
    return mGeom->getTris();
  }

  float* RecastWrapper::GeomWrapper::getMeshBoundsMin()
  {
    return mGeom->getMeshBoundsMin();
  }

  float* RecastWrapper::GeomWrapper::getMeshBoundsMax()
  {
    return mGeom->getMeshBoundsMax();
  }
}
