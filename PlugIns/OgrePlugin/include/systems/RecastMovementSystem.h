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

#ifndef _RecastMovementSystem_H_
#define _RecastMovementSystem_H_

#include "components/MovementComponent.h"

#include "EventSubscriber.h"
#include "ComponentStorage.h"

#include "OgreDetourTileCache.h"
#include "OgreRecast.h"

namespace Ogre
{
  class SceneManager;
}

namespace Gsage
{
  class Entity;
  class Engine;
  class OgreRenderSystem;
  class RenderComponent;
  class MovementComponent;
  class SelectEvent;

  class RecastMovementSystem : public ComponentStorage<MovementComponent>, public EventSubscriber<RecastMovementSystem>
  {
    public:
      // System class identifier
      static const std::string ID;
      RecastMovementSystem();
      virtual ~RecastMovementSystem();

      /**
       * Initialize recast movement system
       * @param settings DataProxy with recast system settings
       */
      bool initialize(const DataProxy& settings);
      /**
       * Builds component instance
       * @param component MovementComponent instance to build
       * @param data Object with all settings
       */
      bool fillComponentData(MovementComponent* component, const DataProxy& data);
      /**
       * Update movement component
       * @param component Movement component pointer
       * @param entity Entity that owns the component
       * @param time elapsed time
       */
      void updateComponent(MovementComponent* component, Entity* entity, const double& time);
      /**
       * Rebuilds navigation mesh
       * @param renderer Render system to get all entities from
       */
      void rebuild();
      /**
       * Change controlled entity id
       * @param id Entity id, entity should have render and movement components
       */
      bool setControlledEntity(const std::string& id);
      /**
       * Resets controlled entity id
       */
      void resetControlledEntity();
      /**
       * Configures recast movement system
       */
      void configUpdated();

      /**
       * Enables/disables debug navigation mesh view
       * @param value Show or not to show
       */
      void showNavMesh(bool value);
    private:
      OgreRecast* mRecast;
      OgreDetourTileCache* mCache;

      Ogre::Vector3 mPosition;
      int mAgentCounter;
      std::string mControlledEntity;
  };
}
#endif
