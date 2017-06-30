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

#include "AnimationScheduler.h"
#include "components/RenderComponent.h"
#include "ogre/SceneNodeWrapper.h"
#include "ogre/EntityWrapper.h"

#include <OgreSceneManager.h>
#include <OgreEntity.h>

#include "Logger.h"

namespace Gsage {
  // --------------------------------------------------------------------------------
  // Animation
  // --------------------------------------------------------------------------------

  Animation::Animation()
    : mAnimationState(0)
    , mFadeIn(false)
    , mFadeOut(false)
    , mSpeed(1)
  {

  }

  Animation::~Animation()
  {

  }

  void Animation::initialize(Ogre::AnimationState* state)
  {
    mAnimationState = state;
    mAnimationState->setWeight(0);
  }

  void Animation::update(const float& time)
  {
    if(!isInitialized() || !mAnimationState->getEnabled())
      return;

    float len = getLength();
    mAnimationState->addTime(time * mSpeed);

    // TODO: Add ability to customize fade out speed
    if(!getLoop())
    {
      const float& delta = len * 0.1f;
      if(mSpeed > 0 && getTimePosition() > (len - delta))
        disable();
      else if(mSpeed < 0 && getTimePosition() < delta)
        disable();
    }

    if(mFadeIn)
    {
      float newWeight = mAnimationState->getWeight() + time * mFadeTime;
      mAnimationState->setWeight(Ogre::Math::Clamp<float>(newWeight, 0, 1));
      if(newWeight >= 1)
        mFadeIn = false;
    }
    else if(mFadeOut)
    {
      float newWeight = mAnimationState->getWeight() - time * mFadeTime;
      mAnimationState->setWeight(Ogre::Math::Clamp<float>(newWeight, 0, 1));
      if(newWeight <= 0)
      {
        mFadeOut = false;
        mAnimationState->setEnabled(false);
        setTimePosition(getLength());
      }
    }
  }

  void Animation::enable(const float& time)
  {
    mFadeIn = true;
    mFadeOut = false;

    mFadeTime = time;

    setEnabled(true);
  }

  void Animation::disable(const float& time)
  {
    mFadeIn = false;
    mFadeOut = true;

    mFadeTime = time;
  }

  float Animation::getTimePosition()
  {
    return mAnimationState->getTimePosition();
  }

  void Animation::setTimePosition(const float& value)
  {
    mAnimationState->setTimePosition(value);
  }

  float Animation::getLength()
  {
    return mAnimationState->getLength();
  }

  bool Animation::getEnabled()
  {
    return isInitialized() && mAnimationState->getEnabled();
  }

  void Animation::setEnabled(bool value)
  {
    if(isInitialized())
    {
      mAnimationState->setEnabled(value);
    }
  }

  bool Animation::getLoop()
  {
    return mAnimationState->getLoop();
  }

  void Animation::setLoop(bool value)
  {
    mAnimationState->setLoop(value);
  }

  bool Animation::hasEnded()
  {
    return mAnimationState->hasEnded();

  }

  const float& Animation::getSpeed()
  {
    return mSpeed;
  }

  void Animation::setSpeed(const float& value)
  {
    mSpeed = value;
  }

  bool Animation::isInitialized()
  {
    return mAnimationState != 0;
  }

  bool Animation::isEnding()
  {
    return isInitialized() && !getLoop() && mFadeOut;
  }

  bool Animation::isFadingOut()
  {
    return mFadeOut;
  }

  void Animation::rewind(const float& offset)
  {
    setTimePosition(mSpeed > 0 ? offset : getLength() - offset);
  }

  // --------------------------------------------------------------------------------
  // AnimationGroup
  // --------------------------------------------------------------------------------

  AnimationGroup::AnimationGroup(RenderComponent* c)
    : mSpeed(1)
    , mRenderComponent(c)
  {

  }

  AnimationGroup::~AnimationGroup()
  {
  }

  bool AnimationGroup::initialize(const DataProxy& dict, Ogre::SceneManager* sceneManager)
  {
    for(auto& pair : dict)
    {
      std::string fullId = pair.second.as<std::string>();

      std::vector<std::string> id = split(fullId, '.');

      std::string entityID;
      mAnimations[pair.first] = Animation();
      if(id.size() < 2) {
        LOG(TRACE) << "Added empty animation to group " << pair.first;
        continue;
      }

      SceneNodeWrapper* containerNode = mRenderComponent->getRoot();
      int i;
      for(i = 0; i < id.size() - 2; i++) {
        containerNode = containerNode->getChildOfType<SceneNodeWrapper>(id[i]);
        if(containerNode == 0) {
          break;
        }
      }

      if(containerNode == 0) {
        LOG(TRACE) << "Failed to find node with name " << id[i];
        continue;
      }

      EntityWrapper* w = containerNode->getChildOfType<EntityWrapper>(id[i]);

      Ogre::Entity* e = w->getEntity();

      // no entity was found with such id
      if(e == 0)
      {
        LOG(ERROR) << "Failed to add animation: entity with id \"" << id[0] << "\" not found in scene manager";
        return false;
      }

      // no such state
      if(!e->hasAnimationState(id[1]))
      {
        LOG(ERROR) << "Failed to add animation: animation state with id \"" << id[1] << "\" not found in entity";
        return false;
      }

      LOG(TRACE) << "Adding animation " << fullId << " to group " << pair.first;
      mAnimations[pair.first].initialize(e->getAnimationState(id[1]));
    }
    return true;
  }

  const float& AnimationGroup::getSpeed()
  {
    return mSpeed;
  }

  void AnimationGroup::setSpeed(const float& value)
  {
    for(auto& pair : mAnimations)
    {
      pair.second.setSpeed(value);
    }
    mSpeed = value;
  }

  bool AnimationGroup::hasEnded()
  {
    for(auto& pair : mAnimations)
    {
      if(!pair.second.hasEnded())
        return false;
    }
    return true;
  }

  // --------------------------------------------------------------------------------
  // AnimationController
  // --------------------------------------------------------------------------------

  AnimationController::AnimationController(Animation& animation, const float& speed, const float& offset)
    : mAnimation(animation)
    , mOffset(offset)
    , mSpeed(speed)
  {
  }

  AnimationController::AnimationController(const AnimationController& other)
    : mAnimation(other.mAnimation)
    , mOffset(other.mOffset)
    , mSpeed(other.mSpeed)
  {
  }

  AnimationController AnimationController::operator=(const AnimationController& other)
  {
    return AnimationController(other.mAnimation, other.mSpeed, other.mOffset);
  }

  AnimationController::~AnimationController() {}

  void AnimationController::start()
  {
    if(mSpeed != 0)
      mAnimation.setSpeed(mSpeed);

    mAnimation.rewind(mOffset);
    mAnimation.setLoop(false);
    mAnimation.enable();
  }

  void AnimationController::finish()
  {
    mAnimation.disable();
  }

  bool AnimationController::hasEnded()
  {
    return mAnimation.hasEnded();
  }

  // --------------------------------------------------------------------------------
  // AnimationScheduler
  // --------------------------------------------------------------------------------

  AnimationScheduler::AnimationScheduler()
    : mInitialized(false)
    , mDefaultAnimationSpeed(1)
    , mSceneManager(0)
    , mRenderComponent(0)
  {
    BIND_PROPERTY("defaultState", &mDefaultAnimation);
    BIND_PROPERTY("defaultSpeed", &mDefaultAnimationSpeed);
    BIND_ACCESSOR("states", &AnimationScheduler::setStates, &AnimationScheduler::getStates);
  }

  AnimationScheduler::~AnimationScheduler()
  {
    mRenderComponent = 0;
  }

  void AnimationScheduler::setRenderComponent(RenderComponent* c)
  {
    mRenderComponent = c;
  }

  bool AnimationScheduler::initialize(const DataProxy& dict, Ogre::SceneManager* sceneManager, RenderComponent* c)
  {
    mRenderComponent = c;
    mSceneManager = sceneManager;
    read(dict);
    playDefaultAnimation();
    return mInitialized = mAnimationGroups.size() > 0;
  }

  bool AnimationScheduler::adjustSpeed(const std::string& name, const float& speed)
  {
    if(mAnimationGroups.count(name) == 0)
    {
      LOG(ERROR) << "Failed to adjust speed for animation \"" << name << "\": no such animation found";
      return false;
    }

    mAnimationGroups[name].setSpeed(speed);
    return true;
  }

  bool AnimationScheduler::play(const std::string& name, const int& times, const float& speed, const float& offset, bool reset)
  {
    if(times == 0)
      return false;

    if(mAnimationGroups.count(name) == 0)
    {
      LOG(ERROR) << "Failed to play animation \"" << name << "\": no such animation found";
      return false;
    }

    AnimationGroup& group = mAnimationGroups[name];
    float animationOffset = offset;

    if(times > 0)
    {
      for(int i = 0; i < times; i++)
      {
        if(!queueAnimation(group, speed, offset, reset && i == 0))
          return false;
      }
      return true;
    }

    mCurrentAnimation = name;
    for(auto& pair : group.mAnimations)
    {
      if(mAnimations.count(pair.first) != 0)
      {
        if(mAnimations[pair.first] == &pair.second && mAnimations[pair.first]->getEnabled() && !mAnimations[pair.first]->isFadingOut())
        {
          // sync animations
          animationOffset = pair.second.getTimePosition();
          continue;
        }
        mAnimations[pair.first]->disable();
      }

      if(pair.second.isInitialized())
      {
        pair.second.setTimePosition(animationOffset);
        if(speed != 0)
          pair.second.setSpeed(speed);
        if(mAnimationQueues.count(pair.first) == 0 || mAnimationQueues[pair.first].empty())
          pair.second.enable();
        pair.second.setLoop(true);
      }
      mAnimations[pair.first] = &pair.second;
    }

    return true;
  }

  bool AnimationScheduler::queueAnimation(AnimationGroup& group, const float& speed, const float& offset, bool reset = false)
  {
    for(auto& pair : group.mAnimations)
    {
      if(!pair.second.isInitialized())
        return false;

      if(mAnimationQueues.count(pair.first) == 0)
        mAnimationQueues[pair.first] = AnimationQueue();

      AnimationController c = AnimationController(pair.second, speed, offset);
      if(reset)
      {
        AnimationQueue empty;
        mAnimationQueues[pair.first].swap(empty);
      }

      if(mAnimationQueues[pair.first].size() == 0)
      {
        if(mAnimations.count(pair.first) != 0)
        {
          // TODO: add dependency on animation speed
          mAnimations[pair.first]->disable();
        }
        c.start();
      }
      mAnimationQueues[pair.first].push(c);
    }
    return true;
  }

  void AnimationScheduler::resetState()
  {
    for(auto& pair : mAnimations)
    {
      pair.second->disable();
    }
    mAnimations.clear();
    playDefaultAnimation();
  }

  void AnimationScheduler::update(const float& time)
  {
    for(auto& pair : mAnimationGroups)
    {
      for(auto& anim : pair.second.mAnimations)
      {

        AnimationQueue& queue = mAnimationQueues[anim.first];
        if(!anim.second.getEnabled() && !isQueued(anim.first, anim.second))
          continue;

        anim.second.update(time);
        if(anim.second.isEnding() || anim.second.hasEnded())
        {
          if(!isQueued(anim.first, anim.second))
            continue;

          // ended limited animation, remove from queue
          queue.pop();
          if(!queue.empty())
          {
            queue.front().start();
          }
          else if(!mCurrentAnimation.empty())
          {
            play(mCurrentAnimation);
          }
        }
      }
    }
  }

  void AnimationScheduler::setStates(const DataProxy& dict)
  {
    mAnimationStatesDict = dict;
    mAnimations.clear();
    mAnimationQueues.clear();
    // get all entities anim states
    for(auto& pair : dict)
    {
      AnimationGroup group(mRenderComponent);
      if(!group.initialize(pair.second, mSceneManager))
      {
        LOG(ERROR) << "Failed to initialize animation group \"" << pair.first << "\", skipped";
        continue;
      }

      mAnimationGroups[pair.first] = group;
      LOG(TRACE) << "Initialized animation group \"" << pair.first << "\"";
    }
  }

  const DataProxy& AnimationScheduler::getStates() const
  {
    return mAnimationStatesDict;
  }

  void AnimationScheduler::playDefaultAnimation()
  {
    if(!mDefaultAnimation.empty())
    {
      play(mDefaultAnimation, LOOP, mDefaultAnimationSpeed);
    }
  }
}
