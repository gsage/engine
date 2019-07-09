/*
-----------------------------------------------------------------------------
This file is a part of Gsage engine

Copyright (c) 2014-2017 Artem Chernyshev

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

#include "Gizmo.h"
#include "ogre/SceneNodeWrapper.h"
#include "systems/OgreRenderSystem.h"
#include "ogre/OgreObjectManager.h"

#include "RenderTarget.h"

#include <imgui.h>
#include "ogre/OgreObjectManager.h"

namespace Gsage {

  Gizmo::Gizmo(OgreRenderSystem* rs)
    : mRenderSystem(rs)
    , mEnabled(false)
    , mOperation(ImGuizmo::TRANSLATE)
    , mMode(ImGuizmo::LOCAL)
    , mUsing(false)
  {
    addEventListener(rs->getObjectManager(), OgreObjectManagerEvent::OBJECT_DESTROYED, &Gizmo::onTargetDestroyed);
  }

  Gizmo::~Gizmo()
  {
  }

  void Gizmo::render(float x, float y, const std::string& rtName)
  {
    if(mTargets.size() == 0 || !mEnabled) {
      return;
    }

    RenderTargetPtr renderTarget = mRenderSystem->getRenderTarget(rtName);
    if(renderTarget == nullptr) {
      return;
    }

    if(ImGuizmo::IsOver()) {
      ImGui::GetIO().WantCaptureMouse = ImGui::IsMouseDown(0);
    }

    Ogre::Camera* cam = renderTarget->getCamera();
    Ogre::Matrix4 view = cam->getViewMatrix();
    Ogre::Matrix4 projection = cam->getProjectionMatrix();

    float v[16] = {0.0f};
    float p[16] = {0.0f};

    if(!extractMatrix(view, v, 16))
      return;

    if(!extractMatrix(projection, p, 16))
      return;

    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    Ogre::Matrix4 model;

    bool positionsDirty = false;

    for(auto target : mTargets) {
      Ogre::Vector3 pos = target->getPositionWithoutOffset();
      if(!contains(mPositions, const_cast<const SceneNodeWrapper*>(target)) || mPositions[target] != pos) {
        positionsDirty = true;
      }

      mPositions[target] = pos;
    }

    if(positionsDirty) {
      updateTargetNode();
    }

    Ogre::Vector3 position = getPosition();
    Ogre::Vector3 scale = getScale();
    Ogre::Quaternion orientation = getOrientation();

    model.makeTransform(position, scale, orientation);

    if(!extractMatrix(model, &mModelMatrix[0], 16))
      return;

    ImGui::PushStyleColor(ImGuiCol_Border, 0);
    ImGuizmo::BeginFrame();
    ImGuizmo::Enable(mEnabled);

    ImGuizmo::SetRect(x, y, (float)renderTarget->getWidth(), (float)renderTarget->getHeight());
    float delta[16] = {0.f};

    ImGuizmo::Manipulate(&v[0], &p[0], mOperation, mMode, &mModelMatrix[0], &delta[0]);

    if(mUsing != ImGuizmo::IsUsing()) {
      updateTargetNode(false);
    }

    mUsing = ImGuizmo::IsUsing();

    if (!mUsing) {
      ImGui::PopStyleColor();
      return;
    }

    ImGuizmo::DecomposeMatrixToComponents(mModelMatrix, matrixTranslation, matrixRotation, matrixScale);
    float deltaTransformation[3], deltaScale[3], deltaRotation[3];
    ImGuizmo::DecomposeMatrixToComponents(delta, deltaTransformation, deltaRotation, deltaScale);
    if(mOperation == ImGuizmo::TRANSLATE) {
      Ogre::Vector3 pos = Ogre::Vector3(&matrixTranslation[0]);
      if(!pos.isNaN()) {
        Ogre::Vector3 delta = pos - position;
        for(auto target : mTargets) {
          target->setPositionWithoutOffset(target->getPositionWithoutOffset() + delta);
          mPositions[target] = pos;
        }
        mPosition = pos;
      }
    }
    else if(mOperation == ImGuizmo::SCALE)
    {
      setScale(deltaScale);
    }
    else if(mOperation == ImGuizmo::ROTATE)
    {
      rotate(deltaRotation);
    }
    ImGuizmo::EndFrame();
    ImGui::PopStyleColor();
  }

  void Gizmo::enable(bool value)
  {
    mEnabled = value;
  }

  void Gizmo::addTarget(SceneNodeWrapper* target)
  {
    mTargets.push_back(target);
    mEnabled = true;
  }

  void Gizmo::removeTarget(const SceneNodeWrapper* target)
  {
    if(contains(mPositions, target)) {
      mPositions.erase(target);
    }

    auto it = std::find(mTargets.begin(), mTargets.end(), target);
    if(it != mTargets.end()) {
      mTargets.erase(it);
    }

    if(mTargets.size() == 0) {
      mEnabled = false;
    }
    updateTargetNode();
  }

  void Gizmo::resetTargets()
  {
    mPositions.clear();
    mInitialPositions.clear();
    mInitialScales.clear();
    mTargets.clear();
    updateTargetNode();
  }

  void Gizmo::setOperation(ImGuizmo::OPERATION value)
  {
    mOperation = value;
  }

  ImGuizmo::OPERATION Gizmo::getOperation() const
  {
    return mOperation;
  }

  void Gizmo::setMode(ImGuizmo::MODE value)
  {
    mMode = value;
  }

  ImGuizmo::MODE Gizmo::getMode() const
  {
    return mMode;
  }

  bool Gizmo::drawCoordinatesEditor(float v_speed, float v_min, float v_max, const std::string& display_format, float power, const std::string& posLabel, const std::string& scaleLabel, const std::string& rotationLabel)
  {
    if(mTargets.size() == 0 || !mEnabled) {
      return false;
    }

    Ogre::SceneNode* node = mTargets[0]->getNode();
    if(!node) {
      return false;
    }

    Ogre::Vector3 position = node->_getDerivedPosition();
    Ogre::Vector3 scale = node->_getDerivedScale();
    Ogre::Quaternion orientation = node->_getDerivedOrientation();

    float pitch = orientation.getPitch().valueDegrees();
    float yaw = orientation.getYaw().valueDegrees();
    float roll = orientation.getRoll().valueDegrees();
    float rotation[3] = {pitch, yaw, roll};

    if(ImGui::DragFloat3(posLabel.c_str(), position.ptr(), v_speed, v_min, v_max, display_format.c_str(), power)) {
      node->setPosition(position);
    }

    if(ImGui::DragFloat3(rotationLabel.c_str(), rotation, v_speed, v_min, v_max, display_format.c_str(), power)) {
      float deltaPitch = rotation[0] - pitch;
      float deltaYaw = rotation[1] - yaw;
      float deltaRoll = rotation[2] - roll;

      if(deltaPitch) {
        node->rotate(Ogre::Vector3::UNIT_X, Ogre::Degree(deltaPitch), Ogre::SceneNode::TS_WORLD);
      }

      if(deltaYaw) {
        node->rotate(Ogre::Vector3::UNIT_Y, Ogre::Degree(deltaYaw), Ogre::SceneNode::TS_WORLD);
      }

      if(deltaRoll) {
        node->rotate(Ogre::Vector3::UNIT_Z, Ogre::Degree(deltaRoll), Ogre::SceneNode::TS_WORLD);
      }
    }
    if(ImGui::DragFloat3(scaleLabel.c_str(), scale.ptr(), v_speed, v_min, v_max, display_format.c_str(), power)) {
      node->setScale(scale);
    }

    return true;
  }

  bool Gizmo::extractMatrix(Ogre::Matrix4 matrix, float* dest, int length)
  {
    if(length != 16) {
      return false;
    }

    float res[16] = {0.0f};
    int k = 0;
    for(int i = 0; i < 4; i++) {
      for(int j = 0; j < 4; j++) {
        res[k++] = (float)matrix[j][i];
      }
    }
    memcpy(dest, res, sizeof(float)*16);
    return true;
  }

  const Ogre::Vector3& Gizmo::getPosition()
  {
    if(mTargets.size() == 1) {
      mPosition = mTargets[0]->getPositionWithoutOffset();
    }
    return mPosition;
  }

  const Ogre::Vector3& Gizmo::getScale()
  {
    if(mTargets.size() == 1) {
      mScale = mTargets[0]->getScale();
    }
    return mScale;
  }

  const Ogre::Quaternion& Gizmo::getOrientation()
  {
    if(mTargets.size() > 0) {
      mOrientation = mTargets[0]->getOrientation();
    }
    return mOrientation;
  }

  bool Gizmo::onTargetDestroyed(EventDispatcher* sender, const Event& event)
  {
    const OgreObjectManagerEvent& e = static_cast<const OgreObjectManagerEvent&>(event);
    if(e.getObject()->getType() != "node") {
      return true;
    }

    const SceneNodeWrapper* node = static_cast<const SceneNodeWrapper*>(e.getObject());
    removeTarget(node);

    return true;
  }

  void Gizmo::updateTargetNode(bool reset)
  {
    mInitialPositions.clear();
    mInitialScales.clear();
    if(mTargets.size() == 1) {
      SceneNodeWrapper* target = mTargets[0];
      mInitialPositions[target] = target->getPositionWithoutOffset();
      mInitialScales[target] = target->getScale();
      return;
    }

    if(reset) {
      mPosition = Ogre::Vector3::ZERO;
      mScale = Ogre::Vector3(1, 1, 1);
    }

    bool hasPosition = false;

    // find median for center position
    for(SceneNodeWrapper* target : mTargets) {
      Ogre::Vector3 pos = target->getPositionWithoutOffset();
      if(!hasPosition) {
        mPosition = pos;
        hasPosition = true;
      } else {
        mPosition += pos;
      }
      mInitialPositions[target] = pos;
      mInitialScales[target] = target->getScale();
    }
    if(mTargets.size() > 0) {
      mPosition /= (Ogre::Real)mTargets.size();
    }
  }

  void Gizmo::rotate(float deltaRotation[3])
  {
    Ogre::Degree rx(deltaRotation[0]);
    Ogre::Degree ry(deltaRotation[1]);
    Ogre::Degree rz(deltaRotation[2]);
    Ogre::Quaternion x(rx, Ogre::Vector3::UNIT_X);
    Ogre::Quaternion y(ry, Ogre::Vector3::UNIT_Y);
    Ogre::Quaternion z(rz, Ogre::Vector3::UNIT_Z);
    x.normalise();
    y.normalise();
    z.normalise();

    Ogre::Quaternion delta = mOrientation;

    mOrientation = x * mOrientation;
    mOrientation = y * mOrientation;
    mOrientation = z * mOrientation;

    delta = mOrientation * delta.Inverse();

    if(mTargets.size() == 1) {
      SceneNodeWrapper* target = mTargets[0];
      target->setOrientation(mOrientation);
    } else {
      for(auto target : mTargets) {
        Ogre::Vector3 newPosition(mPosition + delta * (target->getPositionWithoutOffset() - mPosition));
        target->rotate(delta, Ogre::SceneNode::TS_WORLD);
        target->setPositionWithoutOffset(newPosition);
        mPositions[target] = newPosition;
      }
    }
  }

  void Gizmo::setScale(float scale[3]) {
    Ogre::Vector3 delta(scale);
    delta -= Ogre::Vector3::UNIT_SCALE;

    Ogre::Real multiplier = 1.0f;

    if(delta.x < 0 || delta.y < 0 || delta.z < 0) {
      multiplier = -multiplier;
    }

    bool uniform = delta.x == delta.y && delta.x == delta.z;

    for(auto target : mTargets) {
      Ogre::Quaternion orientation = mOrientation * target->getOrientation().Inverse();
      Ogre::Vector3 d = Ogre::Vector3::ZERO;
      if(uniform) {
        d = delta;
      } else {
        d = orientation * delta;
        for(size_t i = 0; i < 3; i++) {
          d[i] = Ogre::Math::Abs(d[i]);
        }
        d *= multiplier;
      }

      target->setScale(mInitialScales[target] + d * mInitialScales[target]);

      if(mTargets.size() > 1) {
        Ogre::Vector3 pos = mInitialPositions[target] - mPosition;

        d = !uniform ? mOrientation.Inverse() * delta * pos : delta * pos;
        Ogre::Vector3 newPosition = mInitialPositions[target] + d;
        target->setPositionWithoutOffset(newPosition);
        mPositions[target] = newPosition;
      }
    }
  }
}
