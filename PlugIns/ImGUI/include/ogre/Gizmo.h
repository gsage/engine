#ifndef _Gizmo_H_
#define _Gizmo_H_

/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Gsage Authors

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

#include "ImGuizmo.h"

namespace Ogre {
  class Matrix4;
}

namespace Gsage {
  class OgreRenderSystem;
  class SceneNodeWrapper;

  class Gizmo
  {
    public:
      Gizmo(OgreRenderSystem* rs);
      virtual ~Gizmo();

      /**
       * Render gizmo UI
       */
      void render();

      /**
       * Enable/disable gizmo
       */
      void enable(bool value);

      /**
       * Set target
       */
      void setTarget(SceneNodeWrapper* target);

      /**
       * Set gizmo operation
       */
      void setOperation(ImGuizmo::OPERATION value);

      /**
       * Get gizmo operation
       */
      ImGuizmo::OPERATION getOperation() const;

      /**
       * Set gizmo mode
       */
      void setMode(ImGuizmo::MODE value);

      /**
       * Get gizmo mode
       */
      ImGuizmo::MODE getMode() const;
    private:
      OgreRenderSystem* mRenderSystem;
      SceneNodeWrapper* mTarget;
      float mModelMatrix[16];

      ImGuizmo::OPERATION mOperation;
      ImGuizmo::MODE mMode;

      bool mEnabled;

      bool extractMatrix(Ogre::Matrix4, float* dest, int length);
  };
}

#endif
