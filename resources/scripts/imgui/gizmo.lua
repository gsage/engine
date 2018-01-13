require 'lib.class'
local event = require 'lib.event'

-- imguizmo wrapper
Gizmo = class(function(self)
  self.gizmo = imgui.createGizmo()
  self.targetID = nil
  self.reset = function(event)
    if Facade.BEFORE_RESET or (event.type == EntityEvent.REMOVE and event.id == self.targetID) then
      self:resetTarget()
    end
  end

  event:onEntity(core, EntityEvent.REMOVE, self.reset)
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

  local width, height = imgui.GetWindowSize()
  self.gizmo:render(0, 0, width, height)
end
