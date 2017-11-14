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

#include "ImGuiDockspace.h"
#include "Logger.h"

#include <stdio.h>

#include "sole.hpp"

namespace Gsage {

  static ImRect getSlotRect(ImRect parentRect, Dock::Location location, bool isRoot)
  {
    ImVec2 size = parentRect.Max - parentRect.Min;
    ImVec2 center = parentRect.Min + size * 0.5f;

    if(isRoot) {
      switch (location)
      {
        case Dock::Top:
          return ImRect(ImVec2(center.x - 20, parentRect.Min.y + 10),
              ImVec2(center.x + 20, parentRect.Min.y + 30));
        case Dock::Left:
          return ImRect(ImVec2(parentRect.Min.x + 10, center.y - 20),
              ImVec2(parentRect.Min.x + 30, center.y + 20));
        case Dock::Bottom:
          return ImRect(ImVec2(center.x - 20, parentRect.Max.y - 30),
              ImVec2(center.x + 20, parentRect.Max.y - 10));
        case Dock::Right:
          return ImRect(ImVec2(parentRect.Max.x - 30, center.y - 20),
              ImVec2(parentRect.Max.x - 10, center.y + 20));
        default: return ImRect(center - ImVec2(20, 20), center + ImVec2(20, 20));
      }
    } else {
      switch (location)
      {
        default: return ImRect(center - ImVec2(20, 20), center + ImVec2(20, 20));
        case Dock::Top: return ImRect(center + ImVec2(-20, -50), center + ImVec2(20, -30));
        case Dock::Right: return ImRect(center + ImVec2(30, -20), center + ImVec2(50, 20));
        case Dock::Bottom: return ImRect(center + ImVec2(-20, +30), center + ImVec2(20, 50));
        case Dock::Left: return ImRect(center + ImVec2(-50, -20), center + ImVec2(-30, 20));
      }
    }

    return ImRect(ImVec2(0, 0), ImVec2(0, 0));
  }

  static ImRect getDockedRect(const ImRect& rect, const ImVec2& size, Dock::Location location)
  {
    switch (location)
    {
      default: return rect;
      case Dock::Top: return ImRect(rect.Min, ImVec2(rect.Max.x, rect.Min.y + size.y));
      case Dock::Right: return ImRect(ImVec2(rect.Max.x - size.x, rect.Min.y), rect.Max);
      case Dock::Bottom: return ImRect(ImVec2(rect.Min.x, rect.Max.y - size.y), rect.Max);
      case Dock::Left: return ImRect(rect.Min, ImVec2(rect.Min.x + size.x, rect.Max.y));
    }
    return rect;
  }

  static ImVec4 colorSum(const ImVec4& color, ImVec4 adjustment)
  {
    return ImVec4(color.x + adjustment.x, color.y + adjustment.y, color.z + adjustment.z, color.w + adjustment.w);
  }

  static ImVec4 invert(const ImVec4& color)
  {
    return ImVec4(-color.x, -color.y, -color.z, -color.w);
  }

  ImGuiDockspaceStyle getDefaultStyle()
  {
    ImGuiDockspaceStyle style;
    style.splitterThickness = 3.0f;
    style.tabbarHeight = 35.0f;
    style.tabbarTextMargin = 15.0f;
    style.tabbarPadding = 1.0f;
    style.windowRounding = ImGui::GetStyle().WindowRounding;

    style.windowBGColor = ImGui::GetColorU32(ImGuiCol_ChildWindowBg);

    ImVec4 col = colorSum(ImGui::ColorConvertU32ToFloat4(style.windowBGColor), ImVec4(0.05f, 0.05f, 0.05f, 0.0f));
    ImVec4 adjustment(0.1f, 0.1f, 0.1f, 0.0f);

    style.tabInactiveColor = ImGui::ColorConvertFloat4ToU32(colorSum(col, invert(adjustment)));
    style.tabHoveredColor = ImGui::ColorConvertFloat4ToU32(colorSum(col, adjustment));
    style.tabActiveColor = ImGui::ColorConvertFloat4ToU32(col);
    style.textColor = ImGui::GetColorU32(ImGuiCol_Text);

    style.splitterHoveredColor = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    style.splitterNormalColor = 0;

    style.dockSlotNormalColor = ImGui::GetColorU32(ImGuiCol_Button);
    style.dockSlotHoveredColor = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    style.dockSlotPreview = DockSlotPreviewOutline;
    return style;
  }

  Dock::Location getOppositeLocation(Dock::Location location)
  {
    return (Dock::Location)(location ^ 1);
  }

  std::string getStringLocation(Dock::Location location)
  {
    switch(location) {
      case Dock::Left:
        return "left";
      case Dock::Right:
        return "right";
      case Dock::Top:
        return "top";
      case Dock::Bottom:
        return "bottom";
      default:
        return "";
    }

    return "";
  }

  Dock::Dock(const char* label, const ImGuiDockspaceStyle& style)
    : mLayout(Dock::Layout::None)
    , mLabel(ImStrdup(label))
    , mRatio(0.5f)
    , mStyle(style)
  {
    reset();
  }

  Dock::~Dock()
  {
  }

  void Dock::reset()
  {
    mParent = nullptr;
    mNextTab = nullptr;
    mPreviousTab = nullptr;
    mLocation = Dock::Unspecified;
    mActive = true;
    mOpened = true;
    mResized = false;
    mChildren[0] = mChildren[1] = nullptr;
  }

  void Dock::activateOther()
  {
    DockPtr tmp = mPreviousTab;
    while(tmp) {
      if(tmp->getOpened()) {
        tmp->setActive(true);
        return;
      }
      tmp = tmp->mPreviousTab;
    }

    tmp = mNextTab;
    while(tmp) {
      if(tmp->getOpened()) {
        tmp->setActive(true);
        return;
      }
      tmp = tmp->mNextTab;
    }
  }

  bool Dock::isContainer() const {
    return mChildren[0] != nullptr || mChildren[1] != nullptr;
  }

  bool Dock::hasBothChildren() const {
    return mChildren[0] != nullptr && mChildren[1] != nullptr;
  }

  bool Dock::bothChildrenVisible() const {
    return hasBothChildren() && mChildren[0]->visible() && mChildren[1]->visible();
  }

  void Dock::updateChildren() {
    DockPtr tmp = mNextTab;

    if(mPreviousTab == nullptr) {
      while(tmp) {
        if(tmp->getActive()) {
          tmp->setDimensions(mPos, mSize);
          break;
        }
        tmp = tmp->getNextTab();
      }
    }

    if(!hasBothChildren()) {
      return;
    }

    int inactive = -1;

    for(int i = 0; i < 2; i++) {
      if(!mChildren[i]->anyTabOpen()) {
        if(inactive != -1) {
          // deactivate whole container as both childs are inactive
          return;
        }
        inactive = i;
      } else {
        mOpened = true;
      }
    }

    // draw only one half
    if(inactive != -1) {
      DockPtr child = mChildren[(inactive + 1) % 2];
      child->setDimensions(mPos, mSize);
      return;
    }

    ImVec2 pos0 = mPos;
    ImVec2 pos1 = mPos;
    ImVec2 size0 = mSize;
    ImVec2 size1 = mSize;

    switch(mLayout) {
      case Horizontal:
        size0.x = round(mSize.x * mRatio);
        size1.x = mSize.x - size0.x;

        pos1.x = mPos.x + size0.x;

        size0.x -= mStyle.splitterThickness;
        break;
      case Vertical:
        size0.y = round(mSize.y * mRatio);
        size1.y = mSize.y - size0.y;

        pos1.y = mPos.y + size0.y;

        size0.y -= mStyle.splitterThickness;
        break;
      default:
        LOG(INFO) << "Incorrect layout defined for container " << mLayout << " " << mLabel;
        break;
    }

    mChildren[0]->setDimensions(pos0, size0);
    mChildren[1]->setDimensions(pos1, size1);
  }

  void Dock::setDimensions(ImVec2 pos, ImVec2 size)
  {
    mPos = pos;
    mSize = size;

    mBounds = ImRect(mPos, mPos + mSize);

    updateChildren();
  }

  const ImVec2& Dock::getPosition() const
  {
    return mPos;
  }

  const ImVec2& Dock::getSize() const
  {
    return mSize;
  }

  bool Dock::getOpened() const
  {
    return mOpened;
  }

  bool Dock::anyTabOpen() const
  {
    if(mOpened) {
      return true;
    }

    DockPtr tmp = mNextTab;
    while(tmp) {
      if(tmp->getOpened()) {
        return true;
      }
      tmp = tmp->getNextTab();
    }

    return false;
  }

  void Dock::setOpened(bool value)
  {
    if(mOpened == value) {
      return;
    }

    mOpened = value;
    if(!value && mActive) {
      activateOther();
    }

    if(mParent) {
      Dock::Location location = getOppositeLocation(mLocation);
      if(mParent->getChildAt(location)->getOpened() == false) {
        mParent->setOpened(value);
      } else {
        mParent->updateChildren();
      }
    }
  }

  bool Dock::getActive() const
  {
    return mActive;
  }

  void Dock::setActive(bool value)
  {
    mActive = value;
    if(value == false) {
      return;
    }
    DockPtr tmp = mPreviousTab;
    while(tmp) {
      tmp->setActive(false);
      tmp = tmp->getPreviousTab();
    }

    tmp = mNextTab;
    while(tmp) {
      tmp->setActive(false);
      tmp = tmp->getNextTab();
    }
  }

  bool Dock::visible() const
  {
    bool visible = mActive && mOpened;
    DockPtr tmp = mNextTab;
    while(tmp) {
      visible |= tmp->visible();
      if(visible) {
        break;
      }
      tmp = tmp->getNextTab();
    }
    return visible;
  }

  bool Dock::docked() const
  {
    return mParent || mPreviousTab || mNextTab;
  }

  void Dock::addChild(DockPtr child)
  {
    addChildAt(child, child->mLocation);
  }

  void Dock::addChildAt(DockPtr child, Location location)
  {
    mChildren[locationToIndex(location)] = child;
  }

  void Dock::removeChildAt(Location location)
  {
    int index = locationToIndex(location);
    mChildren[index]->mParent = nullptr;
    mChildren[index] = nullptr;
  }

  DockPtr Dock::getChildAt(Location location)
  {
    return getChildAt(locationToIndex(location));
  }

  DockPtr Dock::getChildAt(int index)
  {
    return mChildren[index];
  }

  int Dock::locationToIndex(Dock::Location location) const
  {
    return location == Location::Left || location == Location::Top ? 0 : 1;
  }

  Dock::Layout Dock::getLayout() const
  {
    return mLayout;
  }

  Dock::Location Dock::getLocation() const
  {
    return mLocation;
  }

  DockPtr Dock::getParent()
  {
    return mParent;
  }

  DockPtr Dock::getNextTab()
  {
    return mNextTab;
  }

  DockPtr Dock::getPreviousTab()
  {
    return mPreviousTab;
  }

  const char* Dock::getLabel() const
  {
    return mLabel;
  }

  const ImRect& Dock::getBoundingRect() const
  {
    return mBounds;
  }

  float Dock::calculateRatio(Dock::Layout layout, const ImVec2& containerSize)
  {
    ImVec2 size = mSize;
    if(locationToIndex(mLocation) == 1) {
      size = containerSize - mSize;
    }

    float ratio = 0.5f;

    switch(layout) {
      case Dock::Horizontal:
        ratio = size.x / containerSize.x;
        break;
      case Dock::Vertical:
        ratio = size.y / containerSize.y;
        break;
      default:
        break;
    }

    return ratio;
  }

  float Dock::getRatio() const
  {
    return mRatio;
  }

  void Dock::setRatio(float value)
  {
    if(!isContainer()) {
      return;
    }

    mRatio = ImMin(value, 0.95);
    mRatio = ImMax(mRatio, 0.05);
    updateChildren();
  }

  ImGuiDockspace::ImGuiDockspace(const ImGuiDockspaceStyle& style)
    : mStyle(style)
    , mRootDock(nullptr)
  {
    mLocationToLayout[Dock::Left] = mLocationToLayout[Dock::Right] = Dock::Horizontal;
    mLocationToLayout[Dock::Top] = mLocationToLayout[Dock::Bottom] = Dock::Vertical;
    mLocationToLayout[Dock::Tab] = Dock::None;
  }

  static std::string getDockLabel(DockPtr dock)
  {
    if(dock == nullptr) {
      return "";
    }

    return dock->getLabel();
  }

  ImGuiDockspaceState::Dockstate::Dockstate(DockPtr dock)
  {
    for(int i = 0; i < 2; ++i) {
      children[i] = getDockLabel(dock->getChildAt(i));
    }

    next = getDockLabel(dock->getNextTab());
    prev = getDockLabel(dock->getPreviousTab());
    parent = getDockLabel(dock->getParent());
    layout = dock->getLayout();
    location = dock->getLocation();

    active = dock->getActive();
    opened = dock->getOpened();
    ratio = dock->getRatio();
  }

  ImGuiDockspace::ImGuiDockspace()
  {
    mStyle.splitterThickness = 2.0f;
    mLocationToLayout[Dock::Left] = mLocationToLayout[Dock::Right] = Dock::Horizontal;
    mLocationToLayout[Dock::Top] = mLocationToLayout[Dock::Bottom] = Dock::Vertical;
    mLocationToLayout[Dock::Tab] = Dock::None;
  }

  ImGuiDockspace::~ImGuiDockspace()
  {
  }

  ImGuiDockspaceState ImGuiDockspace::getState()
  {
    ImGuiDockspaceState s;

    for(auto pair : mDocks) {
      s.docks[pair.first] = ImGuiDockspaceState::Dockstate(pair.second);
    }

    return s;
  }

  void ImGuiDockspace::setState(ImGuiDockspaceState state)
  {
    mDocks.clear();
    mRootDock = nullptr;
    // Firstly create all docks
    for(auto pair : state.docks) {
      DockPtr dock = getDock(pair.first);
      if(!dock) {
        dock = createDock(pair.first.c_str());
      }
    }

    for(auto pair : state.docks) {
      DockPtr dock = getDock(pair.first);
      if(!dock) {
        LOG(ERROR) << "Dock " << pair.first << " was not created";
        continue;
      }

      dock->mParent = getDock(pair.second.parent);
      dock->mChildren[0] = getDock(pair.second.children[0]);
      dock->mChildren[1] = getDock(pair.second.children[1]);
      dock->mNextTab = getDock(pair.second.next);
      dock->mPreviousTab = getDock(pair.second.prev);

      dock->mActive = pair.second.active;
      dock->mLocation = pair.second.location;
      dock->mLayout = pair.second.layout;
      dock->mRatio = pair.second.ratio;
      dock->mOpened = pair.second.opened;

      if(dock->getLocation() == Dock::Root) {
        mRootDock = dock;
      }
    }
  }

  void ImGuiDockspace::setDimensions(ImVec2 pos, ImVec2 size)
  {
    if(mRootDock == nullptr) {
      return;
    }

    mRootDock->setDimensions(pos, size);
  }

  void ImGuiDockspace::updateLayout() {
    if(mRootDock == nullptr) {
      return;
    }

    mRootDock->updateChildren();
  }

  bool ImGuiDockspace::dockTo(const char* label, DockPtr child, Dock::Location location)
  {
    DockPtr dock;
    if(label == nullptr) {
      if(mRootDock == nullptr) {
        mRootDock = child;
        child->mLocation = Dock::Root;
        return true;
      } else {
        dock = mRootDock;
      }
    } else if(mDocks.count(label) > 0) {
      dock = mDocks[label];
    }

    if(dock == nullptr) {
      return false;
    }

    if(child->docked()) {
      LOG(ERROR) << "Can't dock " << child->mLabel << " to " << dock->mLabel << ", child should be undocked first";
      return false;
    }

    if(dock == child) {
      LOG(ERROR) << "Can't dock dock to itself";
      return false;
    }

    if(mLocationToLayout.count(location) == 0) {
      LOG(ERROR) << "Can't dock to unsupported location " << location;
      return false;
    }

    Dock::Layout layout = mLocationToLayout[location];

    if(location == Dock::Tab) {
      if(dock->isContainer()) {
        LOG(ERROR) << "Can't add tab to container";
        return false;
      }

      while(dock->mNextTab) {
        dock = dock->mNextTab;
      }
      dock->mNextTab = child;

      child->mLocation = location;
      child->mPreviousTab = dock;
      child->mParent = dock->mParent;

      if(dock) {
        child->mActive = false;
      }
    } else {
      DockPtr previousTab = dock;
      while(true) {
        if(!previousTab->mPreviousTab) {
          break;
        }

        previousTab = previousTab->mPreviousTab;
      }

      if(previousTab != nullptr && dock != previousTab) {
        dock = previousTab;
      }

      std::stringstream ss;
      DockPtr parent = dock->mParent;

      ss << " " << sole::uuid4();

      DockPtr container = createDock(ss.str().c_str());
      container->mLayout = layout;

      ImVec2 parentSize;
      if(parent) {
        container->mLocation = dock->mLocation;
        addChild(parent, container);
        parentSize = dock->getSize();
      } else if(mRootDock) {
        parentSize = mRootDock->getSize();
      }

      child->mLocation = location;

      // opposite location is simply inverse integer
      dock->mLocation = getOppositeLocation(location);

      float ratio = child->calculateRatio(container->getLayout(), parentSize);

      addChild(container, dock);
      addChild(container, child);

      container->setRatio(ratio);

      if(dock == mRootDock) {
        mRootDock = container;
        mRootDock->mLocation = Dock::Root;
      }
    }

    return true;
  }

  DockPtr ImGuiDockspace::getRootDock()
  {
    return mRootDock;
  }

  DockPtr ImGuiDockspace::getDock(const char* label)
  {
    if(mDocks.count(label) == 0) {
      return nullptr;
    }

    return mDocks[label];
  }

  DockPtr ImGuiDockspace::getDock(const std::string& label)
  {
    if(label.empty()) {
      return nullptr;
    }
    return getDock(label.c_str());
  }

  DockPtr ImGuiDockspace::createDock(const char* label, bool opened, bool active)
  {
    if(mDocks.count(label) != 0) {
      return mDocks[label];
    }

    DockPtr dock = DockPtr(new Dock(label, mStyle));
    dock->mOpened = opened;
    dock->mActive = active;
    mDocks[label] = dock;
    return dock;
  }

  bool ImGuiDockspace::undock(DockPtr dock)
  {
    return undock(dock->mLabel);
  }

  bool ImGuiDockspace::undock(const char* label)
  {
    if(mDocks.count(label) == 0) {
      return false;
    }

    DockPtr dock = mDocks[label];

    dock->activateOther();

    bool firstTab = dock->mNextTab && !dock->mPreviousTab;

    if(firstTab) {
      dock->mNextTab->mLocation = dock->mLocation;
    }

    // relink double linked lists
    if(dock->mPreviousTab) {
      dock->mPreviousTab->mNextTab = dock->mNextTab;
    }

    if(dock->mNextTab) {
      dock->mNextTab->mPreviousTab = dock->mPreviousTab;
    }

    // cleanup parent container as it won't have two childs
    DockPtr parent = dock->mParent;
    if(parent && dock->mLocation != Dock::Tab) {
      if(firstTab) {
        parent->addChild(dock->mNextTab);
        if(dock == mRootDock) {
          mRootDock = dock->mNextTab;
        }
      } else {
        DockPtr sibling = parent->getChildAt(getOppositeLocation(dock->mLocation));

        sibling->mParent = parent->mParent;
        sibling->mLocation = parent->mLocation;

        // add second child to the same position in the parent container
        if(parent->mParent) {
          parent->mParent->addChild(sibling);
        }

        if(parent == mRootDock) {
          mRootDock = sibling;
          mRootDock->mLocation = Dock::Root;
        }

        mDocks.erase(parent->mLabel);
      }
    } else if(dock == mRootDock) {
      mRootDock = dock->mNextTab;
    }

    dock->reset();

    return true;
  }

  void ImGuiDockspace::addChild(DockPtr container, DockPtr child)
  {
    container->addChild(child);
    child->mParent = container;
  }

  void ImGuiDockspace::reset()
  {
    mDocks.clear();
  }

  DockPtr ImGuiDockspace::getDockAt(const ImVec2& position)
  {
    for(auto iter = mDocks.rbegin(); iter != mDocks.rend(); ++iter) {
      DockPtr dock = iter->second;
      if(dock->isContainer()) {
        break;
      }

      if(dock->getLocation() != Dock::Unspecified &&  dock->getLocation() != Dock::Tab && dock->visible() && dock->getBoundingRect().Contains(position)) {
        return dock;
      }
    }

    return nullptr;
  }

  ImGuiDockspaceRenderer::ImGuiDockspaceRenderer()
    : mPopClipRect(false)
    , mStateWasUpdated(false)
  {
    mStyle = getDefaultStyle();
    mDockspace = ImGuiDockspace(mStyle);
  }

  ImGuiDockspaceRenderer::ImGuiDockspaceRenderer(const ImGuiDockspaceStyle& style)
    : mDockspace(style)
    , mPopClipRect(false)
    , mStateWasUpdated(false)
  {
    mStyle = style;
  }

  ImGuiDockspaceRenderer::~ImGuiDockspaceRenderer()
  {
    mDockspace.reset();
  }

  void ImGuiDockspaceRenderer::beginWorkspace(ImVec2 pos, ImVec2 size)
  {
    if(pos.x != mPos.x || pos.y != mPos.y || size.y != mSize.y || size.x != mSize.y) {
      mPos = pos;
      mSize = size;

      mDockspace.setDimensions(mPos, mSize);
    }

    dockWindows();
  }

  bool ImGuiDockspaceRenderer::begin(const char* label, bool* opened, ImGuiWindowFlags windowFlags)
  {
    if(mPopClipRect) {
      ImGui::PopClipRect();
      mPopClipRect = false;
    }

    mDockableWindows[label] = true;

    mEndCommand = End_None;

    bool open = opened ? *opened : true;
    DockPtr dock = mDockspace.getDock(label);

    bool drawWindow = false;

    if(dock) {
      if(mStateWasUpdated || dock->mDirty) {
        if(opened) {
          *opened = dock->getOpened();
          dock->mDirty = false;
        }
      } else {
        dock->setOpened(open);
      }

      if(dock->getLocation() == Dock::Unspecified) {
        drawWindow = dock->getOpened();
      }
    } else {
      drawWindow = open;
    }

    if(drawWindow) {
      if(dock && dock == mDraggedDock) {
        ImVec2 size = dock->getSize();
        ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;

        dock->mPos += mouseDelta;
        ImGui::SetNextWindowPos(dock->getPosition());
        ImGui::SetNextWindowSize(dock->getSize());
      }

      bool ret = ImGui::Begin(label,
          opened,
          ImGuiWindowFlags_NoCollapse | windowFlags);

      mEndCommand = End_Window;
      return ret;
    }

    if(!dock->getActive()) {
      ImVec2 zero(0.0f, 0.0f);
      ImGui::PushClipRect(zero, zero, true);
      mPopClipRect = true;
      return false;
    }

    if(!dock->getOpened()) {
      return false;
    }

    mEndCommand = End_Child;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
      ImGuiWindowFlags_AlwaysUseWindowPadding |
      windowFlags;

    ImGui::SetCursorScreenPos(dock->getPosition() + ImVec2(0, mStyle.tabbarHeight));
    ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImGui::GetColorU32(ImVec4(0.0, 0.0, 0.0, 0.0)));
    return ImGui::BeginChild(label, dock->getSize() - ImVec2(0, mStyle.tabbarHeight), false, flags);
  }

  void ImGuiDockspaceRenderer::end()
  {
    switch(mEndCommand) {
      case End_Window:
        ImGui::End();
        break;
      case End_Child:
        ImGui::EndChild();
        ImGui::PopStyleColor();
        break;
      default:
        break;
    }
  }

  void ImGuiDockspaceRenderer::endWorkspace()
  {
    // end rendering workspace
    handleDragging();
    splitters();
    mStateWasUpdated = false;
  }


  ImGuiDockspaceState ImGuiDockspaceRenderer::getState()
  {
    return mDockspace.getState();
  }

  void ImGuiDockspaceRenderer::setState(const ImGuiDockspaceState& state)
  {
    mDockspace.setState(state);
    mStateWasUpdated = true;
  }

  void ImGuiDockspaceRenderer::title(DockPtr dock)
  {
    // if it's tab, it won't have it's own title
    // first tab will draw the whole title bar instead
    if(dock->getLocation() == Dock::Tab) {
      return;
    }

    ImVec2 size(dock->getSize().x, mStyle.tabbarHeight);
    ImVec2 screenPos = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(dock->getPosition());

    char tmp[20];
    ImFormatString(tmp, IM_ARRAYSIZE(tmp), "tabs%d", ImHash(dock->getLabel(), 0));
    if (ImGui::BeginChild(tmp, size, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
      ImDrawList* drawList = ImGui::GetWindowDrawList();
      DockPtr dockTab = dock;

      int count = 0;
      bool first = true;

      while (dockTab) {
        if(!dockTab->getOpened()) {
          dockTab = dockTab->getNextTab();
          continue;
        }
        dockTab = dockTab->getNextTab();
        count++;
      }

      DockPtr selection = nullptr;

      dockTab = dock;
      bool fullWidth = count == 1;
      while(dockTab) {
        if(!dockTab->getOpened()) {
          dockTab = dockTab->getNextTab();
          continue;
        }
        const char* textEnd = ImGui::FindRenderedTextEnd(dockTab->getLabel());
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float textSizeX = ImGui::CalcTextSize(dockTab->getLabel(), textEnd).x;

        float sizeX = fullWidth ? dock->getSize().x : textSizeX + mStyle.tabbarTextMargin * 4;

        ImVec2 size(sizeX, mStyle.tabbarHeight);

        float leftCurveRadius = first ? mStyle.windowRounding : 0.0f;
        DockPtr next = dockTab->getNextTab();
        float rightCurveRadius = next && next->anyTabOpen() ? 0.0f : mStyle.windowRounding;
        ImVec2 tabButtonSize = ImVec2(size.x - 30, size.y);

        if (ImGui::InvisibleButton(dockTab->getLabel(), tabButtonSize))
        {
          selection = dockTab;
        }

        if (ImGui::IsItemActive() && ImGui::IsMouseDragging())
        {
          mDockspace.undock(dockTab);
          mDraggedDock = dockTab;
          ImGuiWindow* window = ImGui::FindWindowByName(dockTab->getLabel());

          ImVec2 mousePos = ImGui::GetIO().MousePos;

          if(window) {
            ImVec2 windowSize = window->Size;
            mDraggedDock->setDimensions(ImVec2(mousePos.x - windowSize.x / 2, mousePos.y - window->TitleBarHeight() / 2), windowSize);
          }
        }

        bool hovered = ImGui::IsItemHovered();

        drawList->PathClear();
        drawList->PathLineTo(pos + ImVec2(0, size.y));
        drawList->PathLineTo(pos + ImVec2(0, leftCurveRadius));
        if(leftCurveRadius > 0) {
          drawList->PathBezierCurveTo(pos + ImVec2(0, leftCurveRadius),
              pos,
              pos + ImVec2(leftCurveRadius, 0),
              10);
        }
        drawList->PathLineTo(pos + ImVec2(size.x - rightCurveRadius, 0));
        if(rightCurveRadius > 0) {
          drawList->PathBezierCurveTo(pos + ImVec2(size.x - rightCurveRadius, 0),
              pos + ImVec2(size.x, 0),
              pos + ImVec2(size.x, rightCurveRadius),
              10);
        }
        drawList->PathLineTo(pos + ImVec2(size.x, size.y));
        drawList->PathFillConvex(
            hovered ? mStyle.tabHoveredColor : (dockTab->getActive() ? mStyle.tabActiveColor : mStyle.tabInactiveColor));

        ImVec2 textPos;

        textPos.x = pos.x + (fullWidth ? size.x : tabButtonSize.x)/2 - textSizeX/2;
        textPos.y = pos.y + size.y/2 - ImGui::GetTextLineHeight()/2;
        drawList->AddText(textPos, mStyle.textColor, dockTab->getLabel(), textEnd);

        ImGui::SetCursorScreenPos(pos + ImVec2(tabButtonSize.x, 0));
        std::stringstream ss;
        ss << dockTab->getLabel() << ".close";
        if(ImGui::InvisibleButton(ss.str().c_str(), ImVec2(size.x - tabButtonSize.x, size.y))) {
          dockTab->setOpened(false);
          dockTab->mDirty = true;
        }

        ImU32 color = mStyle.textColor;
        if (ImGui::IsItemHovered()) {
          color = mStyle.tabHoveredColor;
        }
        ImVec2 center = (ImGui::GetItemRectMin() + ImGui::GetItemRectMax()) * 0.5f;
        ImVec2 crossSize = ImVec2(4.0f, 4.0f);

        center.y += 1;

        drawList->PathClear();
        drawList->AddLine(
            center + ImVec2(-crossSize.x, -crossSize.y), center + ImVec2(crossSize.x, crossSize.y), color, 2);
        drawList->AddLine(
            center + ImVec2(crossSize.x, -crossSize.y), center + ImVec2(-crossSize.x, crossSize.y), color, 2);

        ImGui::SetCursorScreenPos(ImVec2(pos.x + size.x + mStyle.tabbarPadding, pos.y));
        dockTab = dockTab->getNextTab();
        first = false;
      }

      if(selection != nullptr){
        selection->setActive(true);
      }
      ImGui::SetCursorScreenPos(screenPos);
      ImGui::EndChild();
    }
  }

  void ImGuiDockspaceRenderer::dockWindows()
  {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    for(auto iter = mDockspace.mDocks.rbegin(); iter != mDockspace.mDocks.rend(); ++iter) {
      DockPtr dock = iter->second;
      if(dock->isContainer()) {
        break;
      }

      if(dock->getLocation() == Dock::Unspecified)
      {
        continue;
      }

      title(dock);

      if(!dock->visible()) {
        continue;
      }

      ImVec2 pos = dock->getPosition() + ImVec2(0, mStyle.tabbarHeight);
      ImVec2 size = dock->getSize() - ImVec2(0, mStyle.tabbarHeight);
      float curveRadius = mStyle.windowRounding;

      drawList->PathClear();
      drawList->PathLineTo(pos);
      drawList->PathLineTo(pos + ImVec2(size.x, 0));
      drawList->PathLineTo(pos + ImVec2(size.x, size.y - curveRadius));

      if(curveRadius > 0) {
        drawList->PathBezierCurveTo(pos + ImVec2(size.x, size.y - curveRadius),
            pos + size,
            pos + ImVec2(size.x - curveRadius, size.y),
            10);
      }
      drawList->PathLineTo(pos + ImVec2(curveRadius, size.y));

      if(curveRadius > 0) {
        drawList->PathBezierCurveTo(pos + ImVec2(curveRadius, size.y),
            pos + ImVec2(0, size.y),
            pos + ImVec2(0, size.y - curveRadius),
            10);
      }
      drawList->PathFillConvex(mStyle.windowBGColor);
    }
  }

  void ImGuiDockspaceRenderer::splitters()
  {
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    ImDrawList* canvas = ImGui::GetWindowDrawList();
    int index = 200;

    for(auto pair : mDockspace.mDocks) {
      DockPtr dock = pair.second;

      if(!dock->bothChildrenVisible()) {
        continue;
      }

      ImGui::PushID(index++);

      ImVec2 pos = dock->getPosition();
      ImVec2 size;
      float dratio = 0.0f;

      DockPtr child = dock->getChildAt(0);
      ImGuiMouseCursor cursor;

      ImGuiIO& io = ImGui::GetIO();

      switch(dock->getLayout()) {
        case Dock::Horizontal:
          if(dock->mResized) {
            dratio = io.MouseDelta.x / dock->getSize().x;
          }

          pos.x += child->getSize().x;
          size = ImVec2(mStyle.splitterThickness, dock->getSize().y);
          cursor = ImGuiMouseCursor_ResizeEW;
          break;
        case Dock::Vertical:
          if(dock->mResized) {
            dratio = io.MouseDelta.y / dock->getSize().y;
          }

          pos.y += child->getSize().y;
          size = ImVec2(dock->getSize().x, mStyle.splitterThickness);
          cursor = ImGuiMouseCursor_ResizeNS;
          break;
        default:
          LOG(INFO) << "Can't draw splitters for layout " << dock->getLayout();
          return;
      }

      if(dratio != 0) {
        dock->setRatio(dock->getRatio() + dratio);
      }

      ImVec2 screenPos = ImGui::GetCursorScreenPos();
      ImGui::SetCursorScreenPos(pos);
      ImGui::InvisibleButton("split", size);

      if (ImGui::IsItemHovered() && !ImGui::IsMouseDragging()) {
        dock->mResized = ImGui::IsMouseDown(0);
      }

      if(!ImGui::IsMouseDown(0)) {
        dock->mResized = false;
      }

      bool over = ImGui::IsItemHovered() || dock->mResized;

      if(over) {
        ImGui::SetMouseCursor(cursor);
      }

      canvas->AddRectFilled(
          ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), over ? mStyle.splitterHoveredColor : mStyle.splitterNormalColor);
      ImGui::PopID();
      ImGui::SetCursorScreenPos(screenPos);
    }
  }

  void ImGuiDockspaceRenderer::handleDragging()
  {
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    if(ctx->MovingWindow && !mDockableWindows[ctx->MovingWindow->Name]) {
      return;
    }

    if(ctx->MovingWindow && !mDraggedDock) {
      DockPtr dock = mDockspace.getDock(ctx->MovingWindow->Name);
      if(dock == nullptr) {
        dock = mDockspace.createDock(ctx->MovingWindow->Name);
      }

      dock->setDimensions(ctx->MovingWindow->PosFloat, ctx->MovingWindow->Size);
      mDraggedDock = dock;
    }

    if(mDraggedDock) {
      DockPtr destDock = mDockspace.getDockAt(ImGui::GetIO().MousePos);

      ImRect bounds = destDock ? destDock->getBoundingRect() : ImRect(mPos, mPos + mSize);

      std::vector<Dock::Location> locations;
      locations.reserve(5);

      if(!destDock || !destDock->isContainer()) {
        locations.push_back(Dock::Tab);
      }

      if(destDock != nullptr) {
        locations.push_back(Dock::Left);
        locations.push_back(Dock::Right);
        locations.push_back(Dock::Top);
        locations.push_back(Dock::Bottom);
      }

      if(dockSlots(destDock, bounds, locations)) {
        return;
      }

      if(mDockspace.mRootDock && destDock != mDockspace.mRootDock) {
        locations.clear();
        locations.push_back(Dock::Left);
        locations.push_back(Dock::Right);
        locations.push_back(Dock::Top);
        locations.push_back(Dock::Bottom);
        dockSlots(mDockspace.mRootDock, mDockspace.mRootDock->getBoundingRect(), locations);
      }
    }

    if (!ImGui::IsMouseDown(0)) {
      mDraggedDock = nullptr;
    }
  }

  bool ImGuiDockspaceRenderer::dockSlots(DockPtr destDock, const ImRect& rect, const std::vector<Dock::Location>& locations)
  {
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    ImDrawList* canvas = &ctx->OverlayDrawList;

    ImVec2 mousePos = ImGui::GetIO().MousePos;

    for (auto location : locations)
    {
      ImRect r = getSlotRect(rect, location, destDock && destDock == mDockspace.mRootDock);
      bool hovered = r.Contains(mousePos);
      canvas->AddRectFilled(r.Min, r.Max, hovered ? mStyle.dockSlotHoveredColor : mStyle.dockSlotNormalColor);

      if (!hovered)
        continue;

      ImVec2 dockSize = mDraggedDock->getSize();
      ImVec2 halfSize = rect.GetSize() * 0.5f;
      dockSize.x = ImMin(halfSize.x, dockSize.x);
      dockSize.y = ImMin(halfSize.y, dockSize.y);
      ImRect dockedRect = getDockedRect(rect, dockSize, location);

      if (!ImGui::IsMouseDown(0))
      {
        mDraggedDock->setDimensions(dockedRect.Min, dockedRect.GetSize());
        mDockspace.dockTo(destDock ? destDock->getLabel() : nullptr, mDraggedDock, location);
        mDraggedDock = nullptr;
        return true;
      }

      switch(mStyle.dockSlotPreview) {
        case DockSlotPreviewFill:
          canvas->AddRectFilled(dockedRect.Min, dockedRect.Max, mStyle.dockSlotHoveredColor);
          break;
        case DockSlotPreviewOutline:
          float thickness = 5.0f;
          ImVec2 tl = dockedRect.Min + ImVec2(thickness / 2, thickness / 2);
          ImVec2 tr = ImVec2(dockedRect.Max.x - thickness / 2, tl.y);
          ImVec2 br = ImVec2(tr.x, dockedRect.Max.y - thickness / 2);
          ImVec2 bl = ImVec2(tl.x, br.y);

          canvas->PathClear();
          canvas->PathLineTo(tl);
          canvas->PathLineTo(tr);
          canvas->PathLineTo(br);
          canvas->PathLineTo(bl);
          canvas->PathStroke(mStyle.dockSlotHoveredColor, true, 5.0f);
          break;
      }
    }
    return false;
  }
}
