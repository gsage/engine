#ifndef _ImGuiDockspace_H_
#define _ImGuiDockspace_H_

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

#include <map>
#include <vector>
#include <memory>
#include <string>

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

namespace Gsage {

  enum DockSlotPreviewStyle
  {
    // show filled rect for dock slot preview
    DockSlotPreviewFill = 0,
    // only show outline
    DockSlotPreviewOutline = 1
  };

  struct ImGuiDockspaceStyle {
    // splitter thickness allows you to customize spacing between docks as well
    float splitterThickness;
    // default 35
    float tabbarHeight;
    // x margin before and after the text in tab
    float tabbarTextMargin;
    // padding between tabs
    float tabbarPadding;
    // all docks and tabs rounding
    float windowRounding;

    // dock BG color
    ImU32 windowBGColor;
    ImU32 tabInactiveColor;
    ImU32 tabActiveColor;
    ImU32 tabHoveredColor;
    ImU32 textColor;

    ImU32 splitterHoveredColor;
    ImU32 splitterNormalColor;

    // same colors are used for slot rectangles
    ImU32 dockSlotHoveredColor;
    ImU32 dockSlotNormalColor;

    DockSlotPreviewStyle dockSlotPreview;
  };

  class Dock;
  class ImGuiDockspace;
  class ImGuiDockspaceRenderer;

  typedef std::shared_ptr<Dock> DockPtr;

  class Dock {

    public:
      enum Layout {
        None = 0, // not a container
        Vertical = 1,
        Horizontal = 2
      };

      enum Location {
        Unspecified = 10,

        Left = 1,
        Right = Left ^ 1,
        Top = 3,
        Bottom = Top ^ 1,
        Tab = 5,
        Root = 6
      };

      Dock(const char* label, const ImGuiDockspaceStyle& mStyle);
      virtual ~Dock();

      /**
       * Called when dock is detached
       */
      void reset();

      /**
       * Activate any other tab if there is one
       * Descending order is preferable
       */
      void activateOther();

      /**
       * Check if dock is container
       * should have any children present
       */
      bool isContainer() const;

      /**
       * Check if container has both children open
       */
      bool hasBothChildren() const;

      /**
       * Check if both children are visible
       */
      bool bothChildrenVisible() const;

      /**
       * Update children in container
       *
       * Update dimensions, positions and draw splits
       */
      void updateChildren();

      /**
       * Update dock dimensions
       *
       * @param pos Position
       * @param size Size
       */
      void setDimensions(ImVec2 pos, ImVec2 size);

      /**
       * Get first dock scale ratio
       */
      float getRatio() const;
      /**
       * Set first dock scale ratio
       *
       * @param ratio from 0 to 1
       */
      void setRatio(float ratio);

      /**
       * Get position
       */
      const ImVec2& getPosition() const;

      /**
       * Get size
       */
      const ImVec2& getSize() const;

      /**
       * Check if dock is opened
       */
      bool getOpened() const;

      /**
       * Check if this tab or any next tab is open
       */
      bool anyTabOpen() const;

      /**
       * Change opened state for dock
       *
       * @param value
       */
      void setOpened(bool value);

      /**
       * Check if dock is active (tab wise)
       */
      bool getActive() const;

      /**
       * Set dock as active
       *
       * @param value
       */
      void setActive(bool value);

      /**
       * Check if the dock should be drawn
       */
      bool visible() const;

      /**
       * Is docked to anything
       */
      bool docked() const;

      /**
       * Add child to location defined in child itself
       *
       * @param child DockPtr
       */
      void addChild(DockPtr child);

      /**
       * Add child to container at location
       *
       * @param child DockPtr
       * @param location Location
       */
      void addChildAt(DockPtr child, Location location);

      /**
       * Remove child from location
       *
       * @param location Location
       */
      void removeChildAt(Location location);

      /**
       * Get child at specified location
       */
      DockPtr getChildAt(Location location);

      /**
       * Get child at specified index
       */
      DockPtr getChildAt(int index);

      // kind of internal info exposed for tests

      /**
       * Get dock layout
       */
      Dock::Layout getLayout() const;

      /**
       * Get dock location
       */
      Dock::Location getLocation() const;

      /**
       * Get dock parent
       */
      DockPtr getParent();

      /**
       * Get next tab
       */
      DockPtr getNextTab();

      /**
       * Get previous tab
       */
      DockPtr getPreviousTab();

      /**
       * Get dock label
       */
      const char* getLabel() const;

      /**
       * Get bounding rect
       */
      const ImRect& getBoundingRect() const;

      /**
       * Calculate ratio for the dock, basing on dock location
       *
       * @param container destination container
       */
      float calculateRatio(Dock::Layout layout, const ImVec2& containerSize);
    private:
      int locationToIndex(Location location) const;

      friend class ImGuiDockspace;
      friend class ImGuiDockspaceRenderer;

      char* mLabel;

      ImVec2 mPos;
      ImVec2 mSize;
      ImRect mBounds;

      DockPtr mChildren[2];
      DockPtr mNextTab;
      DockPtr mPreviousTab;
      DockPtr mParent;

      Layout mLayout;
      Location mLocation;

      // tab selected
      bool mActive;
      // window is opened
      bool mOpened;
      float mRatio;

      const ImGuiDockspaceStyle& mStyle;

      bool mResized;
      bool mDirty;
  };

  struct ImGuiDockspaceState
  {
    struct Dockstate
    {
      Dockstate() {};
      Dockstate(DockPtr dock);
      virtual ~Dockstate() {}

      float ratio;

      std::string children[2];
      std::string next;
      std::string prev;
      std::string parent;

      Dock::Layout layout;
      Dock::Location location;

      bool active;
      bool opened;
    };

    typedef std::map<std::string, Dockstate> Docks;

    Docks docks;
  };

  /**
   * Imgui dockspace
   */
  class ImGuiDockspace
  {
    public:
      ImGuiDockspace();
      ImGuiDockspace(const ImGuiDockspaceStyle& style);
      virtual ~ImGuiDockspace();

      /**
       * Get dockspace state
       *
       * @return current ImGuiDockspaceState
       */
      ImGuiDockspaceState getState();

      /**
       * Set dockspace state
       *
       * @param state ImGuiDockspaceState
       */
      void setState(ImGuiDockspaceState state);

      /**
       * Update dockspace position and dimensions
       *
       * @param pos Position
       * @param size Size
       */
      void setDimensions(ImVec2 pos, ImVec2 size);

      /**
       * Update layout
       */
      void updateLayout();

      /**
       * Dock window
       *
       * @param label parent dock label
       * @param location location to dock to
       */
      bool dockTo(const char* label, DockPtr dock, Dock::Location location);

      /**
       * Undock dock from any kind of container
       *
       * @param dock DockPtr
       */
      bool undock(DockPtr dock);

      /**
       * Undock dock from any kind of container
       *
       * @param label dock label
       */
      bool undock(const char* label);

      /**
       * Get root dock
       */
      DockPtr getRootDock();

      /**
       * Get window state calculated by updateLayout call
       *
       * @param label identifies window uniquely
       */
      DockPtr getDock(const char* label);

      /**
       * @copydock ImGuiDockspace::getDock
       */
      DockPtr getDock(const std::string& label);

      /**
       * Create new dock
       *
       * @param label dock label
       */
      DockPtr createDock(const char* label, bool opened = true, bool active = true);

      /**
       * Add child dock to container
       *
       * @param container
       * @param child
       */
      void addChild(DockPtr container, DockPtr child);

      /**
       * Reset dockspace, remove all docks
       */
      void reset();

      /**
       * Get dock under mouse position
       *
       * @param ImVec2 position
       */
      DockPtr getDockAt(const ImVec2& position);
    private:

      friend class ImGuiDockspaceRenderer;

      ImGuiDockspaceStyle mStyle;

      typedef std::map<std::string, DockPtr> Docks;

      Docks mDocks;

      typedef std::map<Dock::Location, Dock::Layout> LocationToLayout;

      LocationToLayout mLocationToLayout;

      DockPtr mRootDock;
  };

  /**
   * Dockspace for ImGUI.
   *
   * \verbatim embed:rst:leading-asterisk
   * Provides alternate methods for :code:`Begin` and :code:`End`.
   * Docked view should be surrounded by :code:`BeginDock` and :code:`EndDock` methods.
   *
   * .. code-block:: lua
   *
   *    local flags = 0
   *    local active, open = imgui.BeginDockOpen("test", true, flags)
   *    if active and open then
   *      -- render some view here
   *      imgui.TextWrapped("Hello World!")
   *    end
   *    imgui.EndDock()
   *
   * Saving and loading dockspace state:
   *
   * .. code-block:: lua
   *
   *    -- save dock state (will get lua table)
   *    local savedState = imgui.GetDockState()
   *
   *    ...
   *
   *    -- restore dock state
   *    imgui.SetDockState(savedState)
   * \endverbatim
   */
  class ImGuiDockspaceRenderer
  {
    public:
      ImGuiDockspaceRenderer();
      ImGuiDockspaceRenderer(const ImGuiDockspaceStyle& style);
      virtual ~ImGuiDockspaceRenderer();

      /**
       * Must be called before rendering all windows
       */
      void beginWorkspace(ImVec2 pos, ImVec2 size);

      /**
       * Begin rendering
       *
       * @param label Window label
       * @param opened Pointer to bool
       * @param windowFlags additional window flags
       */
      bool begin(const char* label, bool* opened, ImGuiWindowFlags windowFlags);

      /**
       * End rendering
       */
      void end();

      /**
       * Must be called after rendering all windows
       */
      void endWorkspace();

      /**
       * Get workspace state
       */
      ImGuiDockspaceState getState();

      /**
       * Set workspace state
       *
       * @param state
       */
      void setState(const ImGuiDockspaceState& state);
    private:
      bool tabbar(DockPtr dock, bool closeButton);

      void title(DockPtr dock);

      void dockWindows();

      void splitters();

      void handleDragging();

      bool dockSlots(DockPtr destDock, const ImRect& rect, const std::vector<Dock::Location>& locations);

      enum EndCommand {
        End_None = 0,
        End_Child = 1,
        End_Window = 2
      };

      ImGuiDockspace mDockspace;
      ImGuiDockspaceStyle mStyle;

      EndCommand mEndCommand;

      ImVec2 mPos;
      ImVec2 mSize;

      bool mPopClipRect;

      typedef std::map<std::string, bool> Windows;
      Windows mDockableWindows;

      DockPtr mDraggedDock;

      bool mStateWasUpdated;
  };
}

#endif
