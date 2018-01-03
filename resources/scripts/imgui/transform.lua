require 'lib.class'
local time = require 'lib.time'
require 'imgui.base'

-- node transform settings
Transform = class(ImguiWindow, function(self, ogreView, title, docked, open)
  ImguiWindow.init(self, title, docked, open)
  self.ogreView = ogreView
  self.gizmo = ogreView.gizmo
end)

-- render
function Transform:__call()
  if self:imguiBegin() then
    imgui.Text("Mode")
    imgui.Separator()
    if imgui.RadioButton("Translate", self.gizmo.operation == imgui.gizmo.TRANSLATE) then
      self.gizmo.operation = imgui.gizmo.TRANSLATE
    end
    imgui.SameLine();

    if imgui.RadioButton("Rotate", self.gizmo.operation == imgui.gizmo.ROTATE) then
      self.gizmo.operation = imgui.gizmo.ROTATE
    end
    imgui.SameLine();

    if imgui.RadioButton("Scale", self.gizmo.operation == imgui.gizmo.SCALE) then
      self.gizmo.operation = imgui.gizmo.SCALE
    end

    if imgui.RadioButton("Local", self.gizmo.mode == imgui.gizmo.LOCAL) then
      self.gizmo.mode = imgui.gizmo.LOCAL
    end
    imgui.SameLine()

    if imgui.RadioButton("World", self.gizmo.mode == imgui.gizmo.WORLD) then
      self.gizmo.mode = imgui.gizmo.WORLD
    end

    imgui.Text("Dimensions")
    imgui.Separator()
    if not self.gizmo:drawCoordinatesEditor(1, 0, 0, "%.3f", 1) then
      imgui.Text("No target selected")
    end

    self:imguiEnd()
  end
end
