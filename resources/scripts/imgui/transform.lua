require 'lib.class'
local time = require 'lib.time'
require 'imgui.base'
local lm = require 'lib.locales'
local icons = require 'imgui.icons'

-- node transform settings
Transform = class(ImguiWindow, function(self, sceneEditor, title, docked, open)
  ImguiWindow.init(self, title, docked, open)
  self.icon = icons.rotation_3d
  self.sceneEditor = sceneEditor
  self.gizmo = sceneEditor.gizmo
end)

-- render
function Transform:__call()
  if self:imguiBegin() then
    imgui.Text(lm("transform.mode"))
    imgui.Separator()
    if imgui.RadioButton(lm("transform.op.translate"), self.gizmo.operation == imgui.gizmo.TRANSLATE) then
      self.gizmo.operation = imgui.gizmo.TRANSLATE
    end
    imgui.SameLine();

    if imgui.RadioButton(lm("transform.op.rotate"), self.gizmo.operation == imgui.gizmo.ROTATE) then
      self.gizmo.operation = imgui.gizmo.ROTATE
    end
    imgui.SameLine();

    if imgui.RadioButton(lm("transform.op.scale"), self.gizmo.operation == imgui.gizmo.SCALE) then
      self.gizmo.operation = imgui.gizmo.SCALE
    end

    if imgui.RadioButton(lm("transform.local"), self.gizmo.mode == imgui.gizmo.LOCAL) then
      self.gizmo.mode = imgui.gizmo.LOCAL
    end
    imgui.SameLine()

    if imgui.RadioButton(lm("transform.world"), self.gizmo.mode == imgui.gizmo.WORLD) then
      self.gizmo.mode = imgui.gizmo.WORLD
    end

    imgui.Text(lm("transform.dimensions"))
    imgui.Separator()
    if not self.gizmo:drawCoordinatesEditor(1, 0, 0, "%.3f", 1,
      lm("transform.position"),
      lm("transform.scale"),
      lm("transform.rotation")) then
      imgui.Text(lm("transform.no_target"))
    end

    self:imguiEnd()
  end
end
