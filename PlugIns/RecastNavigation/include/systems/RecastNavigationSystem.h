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

#ifndef _RecastNavigationSystem_H_
#define _RecastNavigationSystem_H_

#include <string>
#include "components/RecastNavigationComponent.h"

#include "EventSubscriber.h"
#include "ComponentStorage.h"

#include "RecastWrapper.h"
#include "TileCache.h"

namespace Gsage
{
  class Entity;
  class Engine;
  class RecastNavigationComponent;
  class SelectEvent;

  class RecastNavigationSystem : public ComponentStorage<RecastNavigationComponent>, public EventSubscriber<RecastNavigationSystem>
  {
    public:
      // System class identifier
      static const std::string ID;
      RecastNavigationSystem();
      virtual ~RecastNavigationSystem();

      /**
       * Initialize recast movement system
       * @param settings DataProxy with recast system settings
       */
      bool initialize(const DataProxy& settings);
      /**
       * Builds component instance
       * @param component RecastNavigationComponent instance to build
       * @param data Object with all settings
       */
      bool fillComponentData(RecastNavigationComponent* component, const DataProxy& data);
      /**
       * Update movement component
       * @param component Movement component pointer
       * @param entity Entity that owns the component
       * @param time elapsed time
       */
      void updateComponent(RecastNavigationComponent* component, Entity* entity, const double& time);
      /**
       * Rebuilds navigation mesh
       * @param renderer Render system to get all entities from
       *
       * @return true if succeed
       */
      bool rebuild(DataProxy options);
      /**
       * Configures recast movement system
       */
      void configUpdated();

      /**
       * Returns default options for recast navmesh builder
       * @return DataProxy
       */
      const DataProxy& getDefaultOptions() const;

      /**
       * @copydoc TileCache::findNearestPointOnNavmesh
       */
      std::tuple<Gsage::Vector3, bool> findNearestPointOnNavmesh(const Gsage::Vector3& p);

      /**
       * Find path using navmesh
       *
       * @param start Start point
       * @param end End point
       *
       * @return path if succeed
       */
      Path3DPtr findPath(const Gsage::Vector3& start, const Gsage::Vector3& end);

      /**
       * Get navigation mesh data, for visualization purposes
       *
       * @return points vector
       */
      std::vector<Gsage::Vector3> getNavMeshRawPoints() const;

    private:
      RecastWrapper mRecast;
      TileCachePtr mTileCache;

      int mAgentCounter;
  };
}
#endif
