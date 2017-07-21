require 'factories.camera'
require 'lib.eal.manager'
local async = require 'lib.async'

describe("test camera eal #ogre", function()
  local ninja = data:createEntity({
    id = 'ninja',
    render = {
      resources = {
        Ninja = {
          "Zip:models/packs/ninja.zip"
        }
      },
      root = {
        position = Vector3.new(0, 0, 0),
        scale = Vector3.new(0.03, 0.03, 0.03),
        rotation = Quaternion.new(1, 0, 0.5, 0),
        orientationVector = Vector3.new(0, 0, -1),
        children = {
          {
            type = "model",
            query = "dynamic",
            name = "model",
            mesh = "ninja.mesh",
            castShadows = true
          }
        }
      }
    }
  })

  it("should properly register eal for the orbit camera", function()
    local orbit = camera:create('orbit', 'testcamera', {target=ninja.id, cameraOffset=Vector3.new(0, 4, 0), distance=20})
    assert.is_not.is_nil(orbit.attach)
    local s = spy.on(orbit, "follow")
    -- attach camera and wait for update
    assert.truthy(orbit:attach())
    assert.truthy(orbit.attached)
    ninja:render().position = Vector3.new(0, 1, 0)
    assert.spy(s).was.called()
    -- detach, so update must be stopped
    orbit:detach()
    assert.falsy(orbit.attached)
    ninja:render().position = Vector3.new(0, 0, 0)
    s = spy.on(orbit, "follow")
    assert.spy(s).was.not_called()
  end)

  it("should properly register eal for the free camera", function()
    local free = camera:create('free', 'free')
    assert.falsy(free.attached)
    assert.truthy(free:attach())
    assert.truthy(free.attached)
    local s = spy.on(free, "update")
    async.waitSeconds(1)
    assert.spy(s).was.called()
    free:detach()
    s = spy.on(free, "update")
    async.waitSeconds(1)
    assert.falsy(free.attached)
    assert.spy(s).was.not_called()
  end)
end)
