require 'lib.class'
require 'factories.base'

Camera = class(function(self, name, settings, cameraPath)
  self.name = name
  self.cameraPath = cameraPath
  self.mouseSensivity = settings.mouseSensivity or 0.003
  self.camera = {
    type = "camera",
    clipDistance = settings.clipDistance or 0.01,
    bgColour = settings.bgColour or "0x000000",
    name = name
  }

  self.onKeyPress = function(event)
    if self.attached then
      self:handleKeyEvent(event)
    end
  end

  self.onMouseEvent = function(event)
    if self.attached then
      self:handleMouseEvent(event)
    end
  end

  self.onTime = function(time)
    self:update(time)
  end

  event:onKey(core, KeyboardEvent.KEY_DOWN, self.onKeyPress)
  event:onKey(core, KeyboardEvent.KEY_UP, self.onKeyPress)
  event:onMouse(core, MouseEvent.MOUSE_MOVE, self.onMouseEvent)
  event:onMouse(core, MouseEvent.MOUSE_UP, self.onMouseEvent)
  event:onMouse(core, MouseEvent.MOUSE_DOWN, self.onMouseEvent)
  local config = self:configure(name, settings)
  if not config then
    return false
  end

  if not data:createEntity(config) then
    return false
  end

  self.entity = entity.get(name)
  return true
end)

function Camera:configure(name, settings)
  return nil
end


function Camera:handleKeyEvent(event)
  return true
end

function Camera:handleMouseEvent(event)
  return true
end

function Camera:update(time)
end

function Camera:attach()
  if currentCamera then
    currentCamera:detach()
  end
  self.entity:render().root:getCamera(self.cameraPath):attach(core:render().viewport)
  currentCamera = self
  core:script():addUpdateListener(self.onTime)
  self.attached = true
end

function Camera:detach()
  self.attached = false
  core:script():removeUpdateListener(self.onTime)
end

-- OrbitCamera implementation
OrbitCamera = class(Camera, function(self, name, settings)
  Camera.init(self, name, settings, "pitch.flip." .. name)
  self.mousePosition = Vector3.ZERO
  self.moveCamera = false
  self.center = Vector3.ZERO
  self.uAngle = 45
  self.vAngle = 45

  self.distance = settings.distance or 10
  self.maxDistance = settings.maxDistance or 100
  self.minDistance = settings.minDistance or 1
  self.maxAngle = settings.maxAngle or 80
  self.minAngle = settings.minAngle or 10
  self.zoomStepMultiplier = settings.zoomStepMultiplier or 0.01
  self.mouseSensivity = settings.mouseSensivity or 0.005

  if type(settings.cameraOffset) == "string" then
    self.cameraOffset, _ = Vector3.parse(settings.cameraOffset or "0,0,0")
  elseif settings.cameraOffset then
    self.cameraOffset = settings.cameraOffset
  end
  self.target = settings.target
  self.targetEntity = nil
  if self.target ~= nil then
    self.targetEntity = entity.get(self.target)
  end
end)

function OrbitCamera:configure(name, settings)
  return {
    id = name,
    render = {
      root = {
        position = settings.position or Vector3.ZERO,
        children = {
          {
            type = "node",
            name = "pitch",
            children = {
              {
                type = "node",
                name = "flip",
                rotation = Quaternion.new(0, 0, 90, 1),
                children = {
                  self.camera
                }
              }
            }
          }
        }
      }
    },
    script = {
      context = settings
    }
  }
end

function OrbitCamera:handleMouseEvent(event)
  if event.relZ ~= 0 then
    self.distance = self.distance + event.relZ * self.zoomStepMultiplier
  end
  if event.type == MouseEvent.MOUSE_DOWN and event.button == MouseEvent.Right then
    self.mousePosition.x = event.x
    self.mousePosition.y = event.y
    self.mousePosition.z = event.z
    self.moveCamera = true
  elseif event.type == MouseEvent.MOUSE_UP and event.button == MouseEvent.Right then
    self.moveCamera = false
  elseif event.type == MouseEvent.MOUSE_MOVE and self.moveCamera then
    local delta = Vector3.ZERO

    delta.x = event.x - self.mousePosition.x
    delta.y = event.y - self.mousePosition.y
    if self.moveCamera then
      self.uAngle = self.uAngle - delta.y
      self.vAngle = self.vAngle + delta.x

      self.mousePosition.x = event.x
      self.mousePosition.y = event.y
      self.mousePosition.z = event.z
    end
  end
  return true
end

function OrbitCamera:update(time)
  if self.target ~= nil then
    local render = self.entity:render()
    local pitchNode = render.root:getSceneNode("pitch")
    local targetRender = nil
    if self.targetEntity then
      targetRender = self.targetEntity:render()
    end
    if targetRender then
      self.center = targetRender.position + self.cameraOffset
    end

    self.uAngle = math.max(self.uAngle, self.minAngle)
    self.uAngle = math.min(self.uAngle, self.maxAngle)
    self.distance = math.max(self.distance, self.minDistance)
    self.distance = math.min(self.distance, self.maxDistance)

    local teta = math.rad(self.uAngle)
    local phi = math.rad(self.vAngle)

    local position = Vector3.new(
      self.center.x + math.sin(teta) * math.cos(phi) * self.distance,
      self.center.y + math.cos(teta) * self.distance,
      self.center.z + math.sin(teta) * math.sin(phi) * self.distance
    )

    local delta = math.asin(math.cos(teta)) - pitchNode.orientation:getPitch().radians

    render.position = position
    render:lookAt(self.center * Vector3.new(1, 0, 1) + Vector3.new(0, position.y, 0))
    if delta ~= 0 then
      pitchNode:pitch(Radian.new(delta))
    end
  end
  return true
end

-- FreeCamera implementation
FreeCamera = class(Camera, function(self, name, settings)
  Camera.init(self, name, settings, "yaw.pitch.roll." .. name)
  self.rotate = false
  self.movementVector = Vector3.ZERO
  self.speed = settings.speed or 2
  self.yawNode = self.entity:render().root:getSceneNode("yaw")
  self.pitchNode = self.yawNode:getSceneNode("pitch")
  self.rollNode = self.pitchNode:getSceneNode("roll")
end)

function FreeCamera:configure(name, settings)
  return {
    id = name,
    render = {
      root =  {
        position = settings.position or Vector3.ZERO,
        children = {
          {
            type = "node",
            name = "yaw",
            children = {
              {
                type = "node",
                name = "pitch",
                children = {
                  {
                    type = "node",
                    name = "roll",
                    children = {
                      self.camera
                    }
                  }
                }
              }
            }
          }
        }
      }
    },
    script = {
      context = settings
    }
  }
end

function FreeCamera:handleMouseEvent(event)
  if event.type == MouseEvent.MOUSE_DOWN and event.button == MouseEvent.Right then
    self.rotate = true
  elseif event.type == MouseEvent.MOUSE_UP and event.button == MouseEvent.Right then
    self.rotate = false
  elseif event.type == MouseEvent.MOUSE_MOVE and self.rotate then
    self.yawNode:yaw(Degree.new(-event.relX * self.mouseSensivity))
    self.pitchNode:pitch(Degree.new(-event.relY * self.mouseSensivity))
  end
  return true
end

function FreeCamera:handleKeyEvent(event)
  local movementVector = self.movementVector
  if event.type == KeyboardEvent.KEY_UP then
    if event.key == Keys.KC_S and movementVector.z > 0 or event.key == Keys.KC_W and movementVector.z < 0 then
      movementVector.z = 0
    end
    if event.key == Keys.KC_A and movementVector.x < 0 or event.key == Keys.KC_D and movementVector.x > 0 then
      movementVector.x = 0
    end
  else
    if event.key == Keys.KC_S then
      movementVector.z = self.speed
    elseif event.key == Keys.KC_W then
      movementVector.z = -self.speed
    end

    if event.key == Keys.KC_A then
      movementVector.x = -self.speed
    elseif event.key == Keys.KC_D then
      movementVector.x = self.speed
    end
  end
  return true
end

function FreeCamera:update(time)
  self.entity:render().root:translate(self.yawNode.orientation * self.pitchNode.orientation * self.movementVector)
end

CameraFactory = class(EntityFactory, function(self)
  EntityFactory.init(self, "camera")
end)

function CameraFactory:createAndAttach(type, name, settings)
  local cam = self:create(type, name, settings)
  if cam then
    cam:attach()
    return true
  end
  return false
end

camera = CameraFactory()
camera:register("orbit", function(name, settings)
  return OrbitCamera(name, settings)
end)

camera:register("free", function(name, settings)
  return FreeCamera(name, settings)
end)
