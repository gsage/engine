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

#include "systems/MovementSystem.h"
#include "Entity.h"
#include "Engine.h"
#include "components/RenderComponent.h"

namespace Gsage {

  const std::string MovementSystem::ID = "3dmovement";

  MovementSystem::MovementSystem()
  {
  }

  MovementSystem::~MovementSystem()
  {
  }

  void MovementSystem::updateComponent(MovementComponent* component, Entity* entity, const double& time)
  {
    if(!component->hasTarget())
    {
      return;
    }

    RenderComponent* renderComponent = mEngine->getComponent<RenderComponent>(entity);

    if(renderComponent == 0)
    {
      LOG(ERROR) << "Failed to move component: render component not present";
      component->mPath = nullptr;
      return;
    }

    if(component->reachedDestination())
    {
      renderComponent->resetAnimationState();
      return;
    }

    if(component->getSpeed() == 0)
    {
      renderComponent->resetAnimationState();
      return;
    } else {
      if(!component->getMoveAnimationState().empty())
      {
        renderComponent->setAnimationState(component->getMoveAnimationState());
        renderComponent->adjustAnimationStateSpeed(component->getMoveAnimationState(), component->getSpeed() * component->getAnimSpeedRatio());
      }
    }

    Gsage::Vector3* newPosition = component->getNextPoint(renderComponent->getPosition(), time);

    if(!newPosition) {
      return;
    }

    renderComponent->setPosition(*newPosition);

    Gsage::Quaternion* rotation = component->getRotation(renderComponent->getDirection(), time);
    if(!rotation) {
      return;
    }

    renderComponent->rotate(*rotation);
  }
}
