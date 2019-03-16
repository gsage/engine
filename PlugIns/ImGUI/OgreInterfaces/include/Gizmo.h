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

#include "Definitions.h"
#if OGRE_VERSION >= 0x020100
#include "v2/ImGuizmo.h"
#else
#include "v1/ImGuizmo.h"
#endif
#include <string>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include "EventSubscriber.h"

namespace Ogre {
  class Matrix4;
}

namespace Gsage {
  class OgreRenderSystem;
  class SceneNodeWrapper;

  /**
   * Transformation Gizmo, supports move, scale and rotate operations.
   *
   * \verbatim embed:rst:leading-asterisk
   *
   * Lua usage example:
   *
   * .. code-block:: lua
   *
   *  -- create gizmo
   *  gizmo = imgui.createGizmo()
   *
   *  -- setting target
   *  local render = eal:getEntity("test").render
   *  if render == nil then
   *    exit(1)
   *  end
   *  gizmo:addTarget(render.root)
   *
   *  -- enable
   *  gizmo:enable(true)
   *
   *  -- render on each ImGUI cycle
   *  gizmo:render(0, 0, 320, 240)
   *
   *  -- changing mode
   *  gizmo.mode = imgui.gizmo.WORLD
   *
   *  -- changing operation
   *  gizmo.operation = imgui.gizmo.ROTATE
   *
   *  -- draw coordinates editor for this gizmo
   *  -- it is separate to make it possible to draw it in the separate window
   *  gizmo:drawCoordinatesEditor(1, 0, 0, "%.3f", 1,
      "position",
      "scale",
      "rotation")
   * \endverbatim
   */
  class Gizmo : public EventSubscriber<Gizmo>
  {
    public:
      Gizmo(OgreRenderSystem* rs);
      virtual ~Gizmo();

      /**
       * Render gizmo UI
       */
      void render(float x, float y, const std::string& rttName);

      /**
       * Enable/disable gizmo
       */
      void enable(bool value);

      /**
       * Set target
       */
      void addTarget(SceneNodeWrapper* target);

      /**
       * Remove target
       */
      void removeTarget(const SceneNodeWrapper* target);

      /**
       * Reset all targets in Gizmo
       */
      void resetTargets();

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

      /**
       * Draw coordinates editor
       */
      bool drawCoordinatesEditor(float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const std::string& display_format = "%.3f", float power = 1.0f, const std::string& posLabel = "position", const std::string& scaleLabel = "scale", const std::string& rotationLabel = "rotation");

    private:

      const Ogre::Vector3& getPosition();

      const Ogre::Vector3& getScale();

      const Ogre::Quaternion& getOrientation();

      bool onTargetDestroyed(EventDispatcher* sender, const Event& event);

      void updateTargetNode(bool reset = true);

      void rotate(float delta[3]);

      void setScale(float scale[3]);

      OgreRenderSystem* mRenderSystem;
      typedef std::vector<SceneNodeWrapper*> Targets;
      Targets mTargets;
      typedef std::map<const SceneNodeWrapper*, Ogre::Vector3> Positions;
      Positions mPositions;
      Positions mInitialPositions;
      Positions mInitialScales;

      float mModelMatrix[16];

      Ogre::Vector3 mPosition;
      Ogre::Vector3 mScale;
      Ogre::Quaternion mOrientation;

      ImGuizmo::OPERATION mOperation;
      ImGuizmo::MODE mMode;

      bool mEnabled;
      bool mUsing;
      bool extractMatrix(Ogre::Matrix4, float* dest, int length);
  };
}

#endif
