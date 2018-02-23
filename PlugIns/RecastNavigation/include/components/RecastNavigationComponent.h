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

#ifndef _RecastNavigationComponent_H_
#define _RecastNavigationComponent_H_

#include "GeometryPrimitives.h"
#include "Component.h"

namespace Gsage {

  class RecastNavigationSystem;

  /**
   * Recast movement component
   */

  class RecastNavigationComponent : public EntityComponent
  {
    public:
      static const std::string SYSTEM;

      RecastNavigationComponent();
      virtual ~RecastNavigationComponent();

      /**
       * Set entity destination
       *
       * @param x X coordinate
       * @param y Y coordinate
       * @param z Z coordinate
       */
      void setTarget(float x, float y, float z);
      /**
       * Set entity destination
       *
       * @param position Vector3 position of target
       */
      void setTarget(const Gsage::Vector3& position);

      /**
       * Get final destination the path
       */
      const Gsage::Vector3& getTarget();

      /**
       * Check if target is actually set
       */
      bool hasTarget();

      /**
       * Reset target currently set target
       */
      void resetTarget();
    private:
      friend class RecastNavigationSystem;

      // TODO: maybe we'll need this for Detour
      int mAgentId;

      bool mAlign;
      bool mAligned;
      bool mHasTarget;

      Gsage::Vector3 mTarget;
  };
}

#endif
