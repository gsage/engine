.. _imgui-label:

Dear ImGUI Plugin
=================

Plugin Name :code:`ImGUIPlugin`

ImGUI is used only for game development utilities.
It may be possible to use it for the game interface but it may lack some functional.

.. important::

   built-in ImGui::Image(...) is not supported.
   There is no support for images yet, but it should be possible to implement images basing on :cpp:class:`Gsage::OgreView`.

.. _imgui-lua-label:

Creating ImGUI Views in Lua
---------------------------

All ImGui views manipulation must be done using :code:`imguiInterface`.
Registering a simple view is pretty straightforward:

.. code-block:: lua

  local imguiInterface = require 'imgui.base'

  if imguiInterface:available() then

    -- simple function
    imguiInterface:addView("window", function()
      imgui.TextWrapped("Hello world")
    end, true)
  end

It is possible to use Lua class for some complicated views:

.. code-block:: lua

  local imguiInterface = require 'imgui.base'

  -- imgui engine stats view
  Stats = class(ImguiWindow, function(self, title, docked, open)
    ImguiWindow.init(self, title, docked, open)
    self.fps = 0
    self.frames = 0
    self.elapsedTime = 0
    self.monitor = ResourceMonitor.new(0.1)
    self.stats = self.monitor.stats

    self.handleTime = function(delta)
      self.frames = self.frames + 1
      self.elapsedTime = self.elapsedTime + delta
      if self.elapsedTime >= 1 then
        self.fps = self.frames
        self.frames = 0
        self.elapsedTime = 0
      end
    end
    game:addUpdateListener(self.monitor)
    time.addHandler("stats", self.handleTime, true)
  end)

  -- render stats
  function Stats:__call()
    if self:imguiBegin() then
      imgui.Text("FPS:" .. self.fps)
      imgui.Text("CPU:" .. math.floor(self.stats.lastCPU * 100))
      self:imguiEnd()
    end
  end

  if imguiInterface:available() then
    local stats = Stats("stats", true)
    imguiInterface:addView("window", stats)
  end

Note that this view inherits :code:`ImguiWindow`.
This allows this view to be configured as dockable window by setting :code:`docked` parameter to :code:`true`.

ImGui Lua Interface
^^^^^^^^^^^^^^^^^^^

ImGui interface for Lua differs from the what there is for C++.
ImGui relies on pointers to bool, float and other types, but there is no way to pass pointer to primitive types from Lua to C++.

Refer to :code:`PlugIns/ImGUI/include/ImguiLuaInterface.h` file to see the list of all available methods.

There are some additional utility classes for Ogre:

.. doxygenclass:: Gsage::OgreView

.. doxygenclass:: Gsage::Gizmo

.. doxygenclass:: Gsage::ImGuiDockspaceRenderer

Additional imgui global variables:

- :code:`imgui.render` :cpp:class:`Gsage::ImguiRenderer` instance.
- :code:`imgui.dockspace` :cpp:class:`Gsage::ImGuiDockspaceRenderer` instance.


