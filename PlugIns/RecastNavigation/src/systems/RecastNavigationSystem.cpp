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

#include "systems/RecastNavigationSystem.h"
#include "Entity.h"
#include "Engine.h"
#include "RecastEvent.h"

#include "components/RenderComponent.h"
#include "components/MovementComponent.h"
#include "systems/RenderSystem.h"
#include "RecastMovementSurface.h"

#include "EngineEvent.h"

namespace Gsage {

  const std::string RecastNavigationSystem::ID = "recast";

  RecastNavigationSystem::RecastNavigationSystem() :
    mAgentCounter(0),
    mTileCache(nullptr)
  {
    mSystemInfo.put("type", RecastNavigationSystem::ID);
  }

  RecastNavigationSystem::~RecastNavigationSystem()
  {
  }

  bool RecastNavigationSystem::initialize(const DataProxy& settings)
  {
    RenderSystem* renderSystem = (RenderSystem*)mEngine->getSystem("render");
    if(renderSystem == 0)
    {
      LOG(ERROR) << "Failed to initialize RecastNavigationSystem, RenderSystem not present in engine";
      return false;
    }

    EngineSystem::initialize(settings);
    return true;
  }

  void RecastNavigationSystem::configUpdated()
  {
    if(mConfig.count("cache") != 0)
      mTileCache->load(mConfig.get<std::string>("cache").first);
    //else
      //rebuild(DataProxy::create(DataWrapper::JSON_OBJECT));
    return EngineSystem::configUpdated();
  }

  bool RecastNavigationSystem::fillComponentData(RecastNavigationComponent* c, const DataProxy& data)
  {
    c->mAgentId = mAgentCounter++;
    return true;
  }

  void RecastNavigationSystem::updateComponent(RecastNavigationComponent* component, Entity* entity, const double& time)
  {
    if(mTileCache == nullptr) {
      return;
    }

    MovementComponent* movementComponent = mEngine->getComponent<MovementComponent>(entity);
    RenderComponent* renderComponent = mEngine->getComponent<RenderComponent>(entity);
    if(!movementComponent || !renderComponent) {
      return;
    }

    Gsage::Vector3 currentPosition = renderComponent->getPosition();

    if(component->mAlign && !component->mAligned)
    {
      Gsage::Vector3 p;
      if(mTileCache->findNearestPointOnNavmesh(currentPosition, p)) {
        renderComponent->setPosition(p);
      }
      component->mAligned = true;
    }

    if(component->hasTarget())
    {
      Path3DPtr path = mTileCache->findPath(currentPosition, component->getTarget());
      mEngine->fireEvent(RecastEvent(RecastEvent::NAVIGATION_START, entity->getId(), path));
      movementComponent->move(path, SurfacePtr(std::make_shared<RecastMovementSurface>(mTileCache)));
      component->resetTarget();
      return;
    }
  }

  bool RecastNavigationSystem::rebuild(DataProxy options)
  {
    EngineSystem* es = mEngine->getSystem("render");
    if (es == 0) {
      LOG(ERROR) << "Failed to rebuild tile cache, RenderSystem not present";
      return false;
    }

    // dynamic_cast is the only way here. This method shouldn't be called often anyways
    RenderSystem* renderSystem = dynamic_cast<RenderSystem*>(es);

    LOG(INFO) << "Rebuilding navigation mesh";

    GeomPtr geom = renderSystem->getGeometry(BoundingBox(BoundingBox::EXTENT_INFINITE), RenderComponent::STATIC);
    if (geom->empty()) {
      LOG(ERROR) << "Failed to rebuild tile cache: empty geom";
      return false;
    }

    const DataProxy& defOptions = getDefaultOptions();

    DataProxy config = merge(defOptions, options);

    TileCachePtr tileCache = mRecast.buildTileCache(std::move(geom), config);

    if(options.get("merge", true) && mTileCache != nullptr) {
      mTileCache->merge(tileCache.get());
    } else {
      mTileCache = tileCache;
    }

    return true;
  }

  const DataProxy& RecastNavigationSystem::getDefaultOptions() const
  {
    static DataProxy options = loads(" \
    {\
      \"cellSize\": 0.3,\
      \"cellHeight\": 0.2,\
      \"walkableSlopeAngle\": 45.0,\
      \"walkableHeight\": 2.0,\
      \"walkableClimb\": 2,\
      \"walkableRadius\": 3,\
      \"maxEdgeLen\": 12.0,\
      \"maxSimplificationError\": 1.3,\
      \"minRegionArea\": 8.0,\
      \"mergeRegionArea\": 20.0,\
      \"maxVertsPerPoly\": 6,\
      \"detailSampleDist\": 6,\
      \"detailSampleMaxError\": 1,\
      \"filterLowHangingObstacles\": true,\
      \"filterLedgeSpans\": true,\
      \"filterWalkableLowHeightSpans\": true,\
      \"tileSize\": 300,\
      \"trisPerChunk\": 256,\
      \"navMeshQuery\": {\
        \"maxNodes\": 2048\
      }\
    }", DataWrapper::JSON_OBJECT);
    return options;
  }

  std::tuple<Gsage::Vector3, bool> RecastNavigationSystem::findNearestPointOnNavmesh(const Gsage::Vector3& p)
  {
    Gsage::Vector3 res;
    if(!mTileCache) {
      return std::make_tuple(res, false);
    }
    bool success = mTileCache->findNearestPointOnNavmesh(p, res);
    return std::make_tuple(res, success);
  }

  Path3DPtr RecastNavigationSystem::findPath(const Gsage::Vector3& start, const Gsage::Vector3& end)
  {
    if(!mTileCache) {
      return nullptr;
    }

    return mTileCache->findPath(start, end);
  }

  std::vector<Gsage::Vector3> RecastNavigationSystem::getNavMeshRawPoints() const {
    if(!mTileCache) {
      return std::vector<Gsage::Vector3>();
    }

    return mTileCache->getPoints();
  }
}
