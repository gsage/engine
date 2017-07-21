require 'lib.class'
local event = require 'lib.event'

-- imguizmo wrapper
Gizmo = class(function(self)
  self.gizmo = imgui.createGizmo()
  self.targetID = nil
  self.reset = function(event)
    if Facade.BEFORE_RESET or (event.type == EntityEvent.DELETE and event.id == self.targetID) then
      self:resetTarget()
    end
  end

  event:onEntity(core, EntityEvent.DELETE, self.reset)
  event:bind(core, Facade.BEFORE_RESET, self.reset)
end)

-- set gizmo target
function Gizmo:setTarget(entity)
  local render = entity.render
  if render == nil then
    log.error("Can't transform target " .. entity.id .. ": no render component")
    return
  end
  self.targetID = entity.id
  self.gizmo:setTarget(render.root)
  self.gizmo:enable(true)
  self.enabled = true
end

-- reset gizmo target
function Gizmo:resetTarget()
  if not self.enabled then
    return
  end
  self.enabled = false
  self.gizmo:setTarget(nil)
  self.gizmo:enable(false)
end

function Gizmo:__call()
  if not self.enabled then
    return
  end
  -- this window will be replaced with the editor panel
  imgui.Begin("Edit Entity", true, ImGuiWindowFlags_NoResize + ImGuiWindowFlags_NoTitleBar)
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
  imgui.End()
  self.gizmo:render()
end
