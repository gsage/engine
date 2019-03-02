local camera = require 'factories.camera'
local event = require 'lib.event'
local imguiInterface = require 'imgui.base'

-- ogre view
OgreView = class(ImguiWindow, function(self, textureID, title, docked, open)
  ImguiWindow.init(self, title, docked, open)
  self.viewport = imgui.createOgreView("0xFF000000")
  self.textureID = textureID
  self.camera = nil

  self.gizmo = imgui.createGizmo()
  self.targetID = nil
  self.gizmoEnabled = false
  self.reset = function(event)
    if Facade.BEFORE_RESET or (event.type == EntityEvent.REMOVE and event.id == self.targetID) then
      self:resetGizmoTarget()
    end
  end

  event:onEntity(core, EntityEvent.REMOVE, self.reset)
  event:bind(core, Facade.BEFORE_RESET, self.reset)
end)

-- create camera
function OgreView:createCamera(type, name, settings)
  self.camera = camera:create(type, name, settings)
  local texture = self.camera:renderToTexture(self.textureID, {
    autoUpdated = true,
    workspaceName = "ogreview",
    viewport = {
      renderQueueSequence = {
        ogre.RENDER_QUEUE_BACKGROUND,
        ogre.RENDER_QUEUE_SKIES_EARLY,
        ogre.RENDER_QUEUE_MAIN,
        ogre.RENDER_QUEUE_OVERLAY,
      }
    }
  })

  if texture then
    self.viewport:setTexture(texture)
  end
end

-- render ogre view
function OgreView:__call()
  imgui.PushStyleVar_2(ImGuiStyleVar_WindowPadding, 5.0, 5.0)
  local render = self:imguiBegin()
  imgui.PopStyleVar(ImGuiStyleVar_WindowPadding)

  if not self.open or not render then
    return
  end

  imguiInterface:captureMouse(not imgui.IsWindowHovered())

  local w, h = imgui.GetContentRegionAvail()
  local x, y = imgui.GetCursorScreenPos()

  if w < 0 or h < 0 then
    self:imguiEnd()
    return
  end

  self.viewport:render(w, h)
  if self.camera then
    self.gizmo:render(x, y, self.textureID)
  end

  self:imguiEnd()
end

-- set gizmo target
function OgreView:setGizmoTarget(entity)
  local render = entity.render
  if render == nil then
    log.error("Can't transform target " .. entity.id .. ": no render component")
    return
  end

  if entity.props.utility then
    return
  end

  self.targetID = entity.id
  self.gizmo:setTarget(render.root)
  self.gizmo:enable(true)
  self.gizmoEnabled = true
end

-- reset gizmo target
function OgreView:resetGizmoTarget()
  if not self.gizmoEnabled then
    return
  end
  self.gizmoEnabled = false
  self.gizmo:setTarget(nil)
  self.gizmo:enable(false)
end

function OgreView:getRenderTarget()
  if not self.camera then
    return nil
  end

  return self.camera:getRenderTarget()
end
