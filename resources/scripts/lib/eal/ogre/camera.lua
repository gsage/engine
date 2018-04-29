local time = require 'lib.time'
local event = require 'lib.event'

currentCamera = currentCamera or nil

local defaultMouseSensivity = 0.4

-- base camera decorator
-- attach requires cameraPath to be set
-- mixin can add update method
local function decorate(cls)
  cls.onCreate(function(self)
    self.mouseSensivity = self.props.mouseSensivity or defaultMouseSensivity
    self.handlerId = self.id .. ".camera.update"
    self.renderTargetName = nil
  end)

  cls.onDestroy(function(self)
    self:detach()
  end)

  -- attach camera to viewport
  function cls:attach(targetName)
    if not self.props.cameraPath then
      -- TODO: search for cameras in render
      log.error("Can't attach camera: must have \"cameraPath\" defined in properties")
      return false
    end
    if currentCamera then
      currentCamera:detach()
    end
    local cam = self.render.root:getCamera(self.props.cameraPath)
    local renderTarget = core:render().mainRenderTarget

    if targetName then
      renderTarget = core:render():getRenderTarget(targetName)
      if not renderTarget then
        log.error("Can't attach camera to render target \"" .. targetName .. "\": not such render target was found")
        return false
      end
    end

    self.renderTargetName = renderTarget.name
    cam:attach(renderTarget)
    currentCamera = self
    if self.update then
      self.onTime = function(time)
        self:update(time)
      end

      time.addHandler(self.handlerId, self.onTime)
    end
    return true
  end

  -- render camera to texture
  function cls:renderToTexture(textureID, parameters)
    local renderTarget = core:render():getRenderTarget(textureID)
    if not renderTarget then
      renderTarget = core:render():createRenderTarget(textureID, RenderTarget.Rtt, parameters or {});
    end
    local cam = self.render.root:getCamera(self.props.cameraPath)
    cam:attach(renderTarget)
    self.renderTargetName = textureID
    if self.update then
      self.onTime = function(time)
        self:update(time)
      end

      time.addHandler(self.handlerId, self.onTime)
    end
  end

  -- detach camera
  function cls:detach()
    if self.renderTargetName then
      local renderTarget = core:render():getRenderTarget(self.renderTargetName)
      if renderTarget then
        renderTarget:setCamera(nil)
      end
    end

    currentCamera = nil
    self.renderTargetName = nil
    if self.update then
      time.removeHandler(self.handlerId)
    end
  end

  function cls:shouldHandleMouseEvent(event)
    return event.type == MouseEvent.MOUSE_UP or self.renderTargetName == event.dispatcher
  end

  return cls
end

-- free camera mixin
local function freeCamera(cls)
  cls.onCreate(function(self)
    if not self.render then
      error("Free camera mixin setup failed: no render component")
    end

    local function getNode(id)
      local node = self:getSceneNode(id)
      if not node then
        error("Free camera entity format is incorrect: expected to have \"" .. id .. "\" node")
      end
      return node
    end

    self.props.cameraPath = "yaw.pitch.roll." .. self.entity.id
    self.rotate = false
    self.movementVector = Vector3.ZERO
    self.mouseSensivity = self.props.mouseSensivity or defaultMouseSensivity
    self.speed = self.props.speed or 5
    self.yawNode = getNode("yaw")
    self.pitchNode = getNode("yaw.pitch")
    self.rollNode = getNode("yaw.pitch.roll")

    self.onKeyPress = function(event)
      -- TODO: handle keypress events for different targets
      if not self.renderTargetName then
        return true
      end

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

    self.onMouseEvent = function(event)
      if not self:shouldHandleMouseEvent(event) then
        return true
      end

      if event.type == MouseEvent.MOUSE_DOWN and event.button == Mouse.Right then
        self.rotate = true
      elseif event.type == MouseEvent.MOUSE_UP and event.button == Mouse.Right then
        self.rotate = false
      elseif event.type == MouseEvent.MOUSE_MOVE and self.rotate then
        self.yawNode:yaw(Radian.new(Degree.new(-event.relX * self.mouseSensivity)), OgreNode.TS_LOCAL)
        self.pitchNode:pitch(Radian.new(Degree.new(-event.relY * self.mouseSensivity)), OgreNode.TS_LOCAL)
      end
      return true
    end
    event:onKeyboard(core, KeyboardEvent.KEY_DOWN, self.onKeyPress)
    event:onKeyboard(core, KeyboardEvent.KEY_UP, self.onKeyPress)
    event:onMouse(core, MouseEvent.MOUSE_MOVE, self.onMouseEvent)
    event:onMouse(core, MouseEvent.MOUSE_UP, self.onMouseEvent)
    event:onMouse(core, MouseEvent.MOUSE_DOWN, self.onMouseEvent)

    self.unbindEvents = function()
      event:unbind(core, KeyboardEvent.KEY_DOWN, self.onKeyPress)
      event:unbind(core, KeyboardEvent.KEY_UP, self.onKeyPress)
      event:unbind(core, MouseEvent.MOUSE_MOVE, self.onMouseEvent)
      event:unbind(core, MouseEvent.MOUSE_UP, self.onMouseEvent)
      event:unbind(core, MouseEvent.MOUSE_DOWN, self.onMouseEvent)
    end
  end)

  cls.onDestroy(function(self)
    self.unbindEvents()
  end)

  function cls:update(time)
    self:translate(self.yawNode.orientation * self.pitchNode.orientation * self.movementVector * time * 10)
  end
  return cls
end

-- orbit camera mixin
local function orbitCamera(cls)
  cls.onCreate(function(self)
    if not self.render then
      error("Orbit camera mixin setup failed: no render component")
    end

    local function getNode(id)
      local node = self:getSceneNode(id)
      if not node then
        error("Orbit camera entity format is incorrect: expected to have \"" .. id .. "\" node")
      end
      return node
    end

    self.props.cameraPath = "pitch.flip." .. self.entity.id
    self.mousePosition = Vector3.ZERO
    self.moveCamera = false
    self.center = Vector3.ZERO
    self.uAngle = 45
    self.vAngle = 45

    self.distance = self.props.distance or 10
    self.maxDistance = self.props.maxDistance or 100
    self.minDistance = self.props.minDistance or 1
    self.maxAngle = self.props.maxAngle or 80
    self.minAngle = self.props.minAngle or 10
    self.zoomStepMultiplier = self.props.zoomStepMultiplier or 0.1
    self.mouseSensivity = self.props.mouseSensivity or 0.5
    if type(self.props.cameraOffset) == "string" then
      self.cameraOffset, _ = Vector3.parse(self.props.cameraOffset or "0,0,0")
    elseif self.props.cameraOffset then
      self.cameraOffset = self.props.cameraOffset
    else
      self.cameraOffset = Vector3.ZERO
    end

    self.onTargetCreate = function(event)
      if self.props and event.id == self.props.target then
        self:setTarget(event.id)
      end
    end

    self.pitchNode = getNode("pitch")
    self:onMouseChange()

    self.positionUpdater = function(event)
      if self.renderTargetName then
        self:follow(self.target.render)
      end
    end

    if self.props.target ~= nil then
      self:setTarget(self.props.target)
    end
    local targetCreateId = event:onEntity(core, EntityEvent.CREATE, self.onTargetCreate)

    self.onMouseEvent = function(event)
      if not self:shouldHandleMouseEvent(event) then
        return true
      end

      local changed = self.moveCamera
      if event.relZ ~= 0 then
        self.distance = self.distance + event.relZ * self.zoomStepMultiplier
        changed = true
      end
      if event.type == MouseEvent.MOUSE_DOWN and event.button == Mouse.Right then
        self.mousePosition.x = event.x
        self.mousePosition.y = event.y
        self.mousePosition.z = event.z
        self.moveCamera = true
      elseif event.type == MouseEvent.MOUSE_UP and event.button == Mouse.Right then
        self.moveCamera = false
      elseif event.type == MouseEvent.MOUSE_MOVE and self.moveCamera then
        local delta = Vector3.ZERO

        delta.x = event.x - self.mousePosition.x
        delta.y = event.y - self.mousePosition.y
        if self.moveCamera then
          self.uAngle = self.uAngle - delta.y * self.mouseSensivity
          self.vAngle = self.vAngle + delta.x * self.mouseSensivity

          self.mousePosition.x = event.x
          self.mousePosition.y = event.y
          self.mousePosition.z = event.z
        end
      end

      if changed then
        self:onMouseChange()
      end
      return true
    end
    event:onMouse(core, MouseEvent.MOUSE_MOVE, self.onMouseEvent)
    event:onMouse(core, MouseEvent.MOUSE_UP, self.onMouseEvent)
    event:onMouse(core, MouseEvent.MOUSE_DOWN, self.onMouseEvent)

    self.unbindEvents = function()
      if not event:unbind(core, EntityEvent.CREATE, self.onTargetCreate) then
        error("Failed to unbind target create handler")
      end
      event:unbind(core, MouseEvent.MOUSE_MOVE, self.onMouseEvent)
      event:unbind(core, MouseEvent.MOUSE_UP, self.onMouseEvent)
      event:unbind(core, MouseEvent.MOUSE_DOWN, self.onMouseEvent)
    end
  end)

  cls.onDestroy(function(self)
    self:setTarget(nil)
    self.unbindEvents()
  end)

  function cls:follow(render)
    self.pitchNode = self:getSceneNode("pitch")
    self.center = render.position + self.cameraOffset
    self:onMouseChange()
  end

  function cls:onMouseChange()
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

    local delta = math.asin(math.cos(teta)) - self.pitchNode.orientation:getPitch().radians

    self.render.position = position
    self.render:lookAt(self.center * Vector3.new(1, 0, 1) + Vector3.new(0, position.y, 0))
    if delta ~= 0 then
      self.pitchNode:pitch(Radian.new(delta))
    end
  end

  function cls:setTarget(targetName)
    if self.target and self.target.render then
      assert(event:unbind(self.target.render, OgreRenderComponent.POSITION_CHANGE, self.positionUpdater))
    end

    if targetName == nil then
      -- target reset
      self.targetName = nil
      self.target = nil
      return true
    end

    local target = eal:getEntity(targetName)
    if not target then
      log.warn("OrbitCamera: can't follow target \"" .. targetName .. "\": no such entity")
      return false
    end
    if not target.render then
      log.warn("OrbitCamera: can't follow target \"" .. targetName .. "\": entity does not have render component")
      return false
    end
    self.target = target
    self.targetName = targetName
    event:bind(self.target.render, OgreRenderComponent.POSITION_CHANGE, self.positionUpdater)
    self:follow(self.target.render)
    log.info("Camera target updated to \"" .. targetName .. "\"")
    return true
  end

  return cls
end

eal:extend({class = {name = "camera", requires = {render = "ogre"}}}, decorate)
eal:extend({mixin = "freeCamera"}, freeCamera)
eal:extend({mixin = "orbitCamera"}, orbitCamera)
