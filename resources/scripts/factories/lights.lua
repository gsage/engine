local eal = require 'lib.eal.manager'
require 'factories.base'
require 'lib.utils'

local LightFactory = class(EntityFactory, function(self)
  EntityFactory.init(self, "light")
end)

local function createLight(t, s)
  return {
    type = "light",
    colourSpecular = "0xFFFFFFFF",
    castShadows = true,
    colourShadow = "0x33333333",
    colourDiffuse = "0xFFFFFFFF",
    lightType = t,
    powerScale = s,
  }
end

local directional = createLight("directional", 2)
directional.direction = Vector3.new(-1, -1, -1)

local spot = createLight("spot", 2)
spot.direction = Vector3.new(-1, -1, -1)

local lightsData = {
  point = {
    render = {
      root = {
        children = {createLight("point", 2)}
      }
    }
  },
  directional = {
    render = {
      root = {
        children = {directional}
      }
    }
  },
  spot = {
    render = {
      root = {
        children = {spot}
      }
    }
  }
}

lights = lights or LightFactory()

for key, value in pairs(lightsData) do
  lights:register(key, function(name, settings)
    local e = deepcopy(value)
    e = tableMerge(e, settings)
    e.id = name
    if not data:createEntity(e) then
      return nil
    end

    return eal:getEntity(name)
  end)
end

return lights
