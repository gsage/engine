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

#include "systems/RecastMovementSystem.h"
#include "Entity.h"
#include "Engine.h"

#include "ogre/SceneNodeWrapper.h"
#include "systems/OgreRenderSystem.h"
#include "components/RenderComponent.h"

#include "RecastInputGeom.h"
#include "EngineEvent.h"

namespace Gsage {

  const std::string RecastMovementSystem::ID = "recast";

  RecastMovementSystem::RecastMovementSystem() :
    mRecast(0),
    mCache(0),
    mPosition(0,0,0),
    mAgentCounter(0)
  {
    mSystemInfo.put("type", RecastMovementSystem::ID);
  }

  RecastMovementSystem::~RecastMovementSystem()
  {
    if(mCache)
      delete mCache;
    if(mRecast)
      delete mRecast;
  }

  bool RecastMovementSystem::initialize(const DataProxy& settings)
  {
    OgreRenderSystem* renderSystem = mEngine->getSystem<OgreRenderSystem>();
    if(renderSystem == 0)
    {
      LOG(ERROR) << "Failed to initialize RecastMovementSystem, OgreRenderSystem not present in engine";
      return false;
    }

    mRecast = new OgreRecast(renderSystem->getSceneManager(), OgreRecastConfigParams());
    mCache = new OgreDetourTileCache(mRecast);
    EngineSystem::initialize(settings);
    return true;
  }

  void RecastMovementSystem::configUpdated()
  {
    if(mConfig.count("cache") != 0)
      mCache->loadAll(mConfig.get<std::string>("cache").first);
    else
      rebuild();
    EngineSystem::configUpdated();
  }

  bool RecastMovementSystem::fillComponentData(MovementComponent* c, const DataProxy& data)
  {
    c->mAgentId = mAgentCounter++;
    return true;
  }

  void RecastMovementSystem::updateComponent(MovementComponent* component, Entity* entity, const double& time)
  {
    RenderComponent* renderComponent = mEngine->getComponent<RenderComponent>(entity);
    if(!component->mAligned)
    {
      if(renderComponent)
      {
        Ogre::Vector3 p;
        mRecast->findNearestPointOnNavmesh(renderComponent->getPosition(), p);
        renderComponent->setPosition(p);
        component->mAligned = true;
      }
    }

    if(!component->hasTarget())
    {
      if(component->mTargetReset)
      {
        if(renderComponent)
          renderComponent->resetAnimationState();
        component->mTargetReset = false;
      }
      return;
    }

    if(renderComponent == 0)
    {
      LOG(ERROR) << "Failed to move component: render component not present";
      component->resetTarget();
      return;
    }

    if(component->mSpeed == 0)
    {
      renderComponent->resetAnimationState();
      return;
    } else {
      if(!component->mMoveAnimationState.empty())
      {
        renderComponent->setAnimationState(component->mMoveAnimationState);
        renderComponent->adjustAnimationStateSpeed(component->mMoveAnimationState, component->mSpeed * component->mAnimSpeedRatio);
      }
    }

    if(!component->hasPath())
    {
      int res = mRecast->FindPath(renderComponent->getPosition(), component->getFinalTarget(), component->mAgentId, 0);
      if(res > 0)
      {
        component->setPath(mRecast->getPath(component->mAgentId));
        if(!component->mMoveAnimationState.empty())
        {
          renderComponent->setAnimationState(component->mMoveAnimationState);
          renderComponent->adjustAnimationStateSpeed(component->mMoveAnimationState, component->mSpeed * component->mAnimSpeedRatio);
        }
      }
      else
      {
        component->resetTarget();
        return;
      }
    }

    const Ogre::Vector3 flattener(1, 0, 1);

    Ogre::Vector3 currentTarget;
    mRecast->findNearestPointOnNavmesh(component->getCurrentTarget(), currentTarget);
    // Update position
    const Ogre::Vector3& currentPosition = renderComponent->getPosition();

    float speed = (component->mSpeed * time);
    Ogre::Vector3 delta = currentTarget - currentPosition;
    Ogre::Vector3 newPosition;

    if(delta.length() < speed)
      newPosition = currentTarget;
    else
      newPosition = currentPosition + delta.normalisedCopy() * speed;

    Ogre::Vector3 alignedPosition;
    mRecast->findNearestPointOnNavmesh(newPosition, alignedPosition);
    renderComponent->setPosition(alignedPosition * Ogre::Vector3::UNIT_Y + newPosition * flattener);
    component->setLastMovementDistance((alignedPosition - currentPosition).length());

    // Rotate object
    const Ogre::Vector3& direction = renderComponent->getDirection() * flattener;
    delta *= flattener;
    renderComponent->rotate(direction.getRotationTo(delta, Ogre::Vector3::UNIT_Y));

    if(newPosition == currentTarget)
    {
      if(!component->nextPoint())
      {
        component->resetTarget(); // reached destination
      }
    }
  }

  bool RecastMovementSystem::setControlledEntity(const std::string& id)
  {
    RenderComponent* render = mEngine->getComponent<RenderComponent>(id);
    MovementComponent* movement = mEngine->getComponent<MovementComponent>(id);
    bool isSet = false;

    if(render && movement)
    {
      isSet = true;
      mControlledEntity = id;
    }
    return isSet;
  }

  void RecastMovementSystem::resetControlledEntity()
  {
    mControlledEntity.clear();
  }

  void RecastMovementSystem::rebuild()
  {
    OgreRenderSystem* renderSystem = mEngine->getSystem<OgreRenderSystem>();
    if(renderSystem == 0)
    {
      LOG(ERROR) << "Failed to rebuild tile cache, OgreRenderSystem not present";
      return;
    }

    LOG(INFO) << "Rebuilding navigation mesh";
    InputGeom geom(renderSystem->getEntities(SceneNodeWrapper::STATIC));
    mCache->TileCacheBuild(&geom);
  }

  void RecastMovementSystem::showNavMesh(bool value)
  {
    if(value)
      mCache->drawNavMesh();
  }
}
