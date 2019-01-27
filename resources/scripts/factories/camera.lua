require 'lib.class'
require 'factories.base'
require 'helpers.base'
local eal = require 'lib.eal.manager'

-- CameraFactory allows creating different kinds of cameras
local CameraFactory = class(EntityFactory, function(self)
  EntityFactory.init(self, "camera")
end)

-- createAndAttach shorthand method to create camera and attach it
-- @param type camera type
-- @param name camera name
-- @param settings camera settings
-- @returns camera entity from eal
function CameraFactory:createAndAttach(type, name, settings)
  local cam = self:create(type, name, settings)
  if cam then
    cam:attach()
    return true
  end
  return false
end

-- singleton
camera = camera or CameraFactory()

local createCamera = function(name, settings)
  return {
    type = "camera",
    clipDistance = settings.clipDistance or 0.01,
    bgColour = settings.bgColour or "0x000000",
    name = name
  }
end

camera:register("free", function(name, settings)
  settings.mixins = {"freeCamera"}
  settings.class = "camera"
  local config = {
    id = name,
    flags = {"dynamic"},
    props = settings,
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
                      createCamera(name, settings)
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  local e = data:createEntity(config)
  if not e then
    return nil
  end

  return eal:getEntity(e.id)
end)

camera:register("orbit", function(name, settings)
  settings.mixins = {"orbitCamera"}
  settings.class = "camera"
  if type(settings.cameraOffset) ~= "string" then
    settings.cameraOffset =
      tostring(settings.cameraOffset.x) .. "," ..
      tostring(settings.cameraOffset.y) .. "," ..
      tostring(settings.cameraOffset.z)
  end
  local config = {
    id = name,
    props = settings,
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
                  createCamera(name, settings)
                }
              }
            }
          }
        }
      }
    }
  }

  local e = data:createEntity(config)
  if not e then
    return nil
  end

  return eal:getEntity(e.id)
end)

return camera
