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

#ifndef _AnimationScheduler_H_
#define _AnimationScheduler_H_

#include <queue>

#include "OgreConverters.h"
#include "Serializable.h"

#define DEFAULT_FADE_SPEED 5.0f
#define LOOP -1

namespace Ogre
{
  class SceneManager;
  class Entity;
  class AnimationState;
}

namespace Gsage {
  class AnimationScheduler;
  class AnimationGroup;
  class RenderComponent;

  /**
   * Class that wraps ogre animation state, adds speed definition
   */
  class Animation
  {
    public:
      Animation();
      virtual ~Animation();
      /**
       *
       */
      void initialize(Ogre::AnimationState* state);
      /**
       * Calls Ogre::AnimationState addTime with adjusted speed
       *
       * @param time Time in seconds
       */
      void update(const float& time);
      /**
       * Enables animation
       *
       * @param time Time to fade in
       */
      void enable(const float& time = DEFAULT_FADE_SPEED);
      /**
       * Disables animation
       *
       * @param time Time to fade out
       */
      void disable(const float& time = DEFAULT_FADE_SPEED);
      /**
       * Gets current time position of anim
       */
      float getTimePosition();
      /**
       * Sets current time position of anim
       * 
       * @param value Time in seconds
       */
      void setTimePosition(const float& value);
      /**
       * Gets animation length
       */
      float getLength();
      /**
       * Gets enabled
       */
      bool getEnabled();
      /**
       * Change enabled animation state immediately
       *
       * @param value Enable anim
       */
      void setEnabled(bool value);
      /**
       * Gets loop enabled
       */
      bool getLoop();
      /**
       * Sets loop value
       *
       * @param value
       */
      void setLoop(bool value);
      /**
       * Gets animation finished (if not looping)
       */
      bool hasEnded();
      /**
       * Gets animation speed
       */
      const float& getSpeed();
      /**
       * Sets animation speed
       */
      void setSpeed(const float& speed);
      /**
       * Animation has anim state
       */
      bool isInitialized();
      /**
       * Animation is finite and fading out
       */
      bool isEnding();
      /**
       * Animation is disabled but is still fading
       */
      bool isFadingOut();
      /**
       * Rewind animation to the beginning, depends on direction
       * @param offset If speed >= 0: 0 + offset, else: length - offset
       */
      void rewind(const float& offset = 0);
    private:
      friend class AnimationScheduler;
      Ogre::AnimationState* mAnimationState;
      bool mFadeIn;
      bool mFadeOut;
      float mFadeTime;
      float mSpeed;
  };

  /**
   * Container for several animations
   */
  class AnimationGroup
  {
    public:
      AnimationGroup() : mRenderComponent(0) {};
      AnimationGroup(RenderComponent* c);
      virtual ~AnimationGroup();
      /**
       * Initialize animation groups
       */
      bool initialize(const DataProxy& dict, Ogre::SceneManager* sceneManager);
      /**
       * Gets animations speed
       */
      const float& getSpeed();
      /**
       * Sets animations speed
       *
       * @param value 1 is normal speed
       */
      void setSpeed(const float& value);
      /**
       * Checks that all animations ended
       */
      bool hasEnded();
    private:
      friend class AnimationScheduler;

      typedef std::map<std::string, Animation> Animations;
      Animations mAnimations;

      RenderComponent* mRenderComponent;

      float mSpeed;
  };

  /**
   * Animation controller
   */
  class AnimationController
  {
    public:
      AnimationController(Animation& animation, const float& speed = 0, const float& offset = 0);
      AnimationController(const AnimationController& other);
      virtual ~AnimationController();

      /**
       * Starts animation with predefined parameters
       */
      void start();
      /**
       * Stops animation
       */
      void finish();
      /**
       * Check that wrapped animation ended
       */
      bool hasEnded();

      AnimationController operator=(const AnimationController& other);
      bool operator ==(Animation& anim) { return &anim == &mAnimation; }
      bool operator !=(Animation& anim) { return &anim != &mAnimation; }
    private:
      Animation& mAnimation;
      float mOffset;
      float mSpeed;
  };

  /**
   * Class that reads all animation information, configures states, then manages all animations playback
   */
  class AnimationScheduler : public Serializable<AnimationScheduler>
  {
    public:
      AnimationScheduler();
      virtual ~AnimationScheduler();

      void setRenderComponent(RenderComponent* c);
      /**
       * Read dict and configure groups, read anim states from entities
       *
       * @param dict DataProxy to get settings from
       * @param sceneManager Ogre::SceneManager to get entities from
       * @param renderComponent target render component
       */
      bool initialize(const DataProxy& dict, Ogre::SceneManager* sceneManager, RenderComponent* renderComponent);

      /**
       * Adjust animation speed
       *
       * @param name Animation group name
       * @param speed Animation speed, 1 -- is normal speed
       * @returns true if animation group exists
       */
      bool adjustSpeed(const std::string& name, const float& speed);
      /**
       * Play animations from animations group
       *
       * @param name Animation group name
       * @param times Repeat times, -1 means loop
       * @param speed Animation speed, 1 -- is normal speed
       * @param offset Animation start offset
       * @param reset Starts animation immediately
       *
       * @returns true if animation group exists
       */
      bool play(const std::string& name, const int& times = LOOP, const float& speed = 0, const float& offset = 0, bool reset = false);
      /**
       * Reset animation state
       */
      void resetState();
      /**
       * Update animations
       *
       * @param time Time delta in seconds
       */
      void update(const float& time);

    private:
      /**
       * Plays default animation, if present
       */
      void playDefaultAnimation();
      /**
       * Queue animation
       * @param animation Animation group to play
       * @param speed Animation speed
       * @param offset Animation offset
       * @param reset interrupt current animation
       */
      bool queueAnimation(AnimationGroup& animation, const float& speed, const float& offset, bool reset);
      /**
       * Check that animation is in queue
       * @param group Animation group, queue name
       * @param anim Animation instance
       */
      inline bool isQueued(const std::string& group, Animation& anim) 
      {
        if(mAnimationQueues.count(group) == 0)
          return false;

        return !mAnimationQueues[group].empty() && mAnimationQueues[group].front() == anim;
      }
      /**
       * Set animation states
       * @param dict DataProxy with settings
       */
      void setStates(const DataProxy& dict);
      /**
       * Get animation states serialized
       */
      const DataProxy& getStates() const;

      Ogre::SceneManager* mSceneManager;

      typedef std::map<std::string, Animation*> Animations;
      typedef std::map<std::string, AnimationGroup> AnimationGroups;
      typedef std::queue<AnimationController> AnimationQueue;
      typedef std::map<std::string, AnimationQueue> AnimationQueues;

      AnimationQueues mAnimationQueues;
      AnimationGroups mAnimationGroups;
      Animations mAnimations;

      float mDefaultAnimationSpeed;
      std::string mDefaultAnimation;
      std::string mCurrentAnimation;

      DataProxy mAnimationStatesDict;

      RenderComponent* mRenderComponent;

      bool mInitialized;
  };
}

#endif
