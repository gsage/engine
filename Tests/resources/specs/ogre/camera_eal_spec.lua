require 'factories.camera'
require 'lib.eal.manager'
local async = require 'lib.async'

describe("test camera eal #ogre", function()
  local ninja = data:createEntity({
    id = 'ninja',
    render = {
      resources = {
        Ninja = {
          "Zip;bundles/models/v1/packs/ninja.zip"
        }
      },
      root = {
        position = Vector3.new(0, 0, 0),
        scale = Vector3.new(0.01, 0.01, 0.01),
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
    local orbit = camera:create('orbit', 'orbit', {target=ninja.id, cameraOffset=Vector3.new(0, 4, 0), distance=20})
    assert.is_not.is_nil(orbit.attach)
    local s = spy.on(orbit, "follow")
    -- attach camera and wait for update
    assert.truthy(orbit:attach())
    assert.is_not.is_nil(orbit.renderTargetName)
    ninja:render().position = Vector3.new(0, 1, 0)
    assert.spy(s).was.called()
    -- detach, so update must be stopped
    orbit:detach()
    assert.is_nil(orbit.renderTargetName)
    ninja:render().position = Vector3.new(0, 0, 0)
    s = spy.on(orbit, "follow")
    assert.spy(s).was.not_called()
    -- verify proper removal
    game:reset()
    assert.is_nil(orbit.targetName)
    assert.is_nil(orbit.target)
  end)

  it("should properly register eal for the free camera", function()
    local free = camera:create('free', 'free')
    assert.falsy(free.attached)
    assert.truthy(free:attach())
    assert.is_not.is_nil(free.renderTargetName)
    local s = spy.on(free, "update")
    async.waitSeconds(1)
    assert.spy(s).was.called()
    free:detach()
    s = spy.on(free, "update")
    async.waitSeconds(1)
    assert.is_nil(free.renderTargetName)
    assert.spy(s).was.not_called()
  end)

  it("should render to texture", function()
    local free = camera:create('free')
    free:renderToTexture("texture")
  end)

  it("should attach to and detach from renderTarget", function()
    local rtex = 'rtex test'
    local free1 = camera:create('free', 'free1')
    free1:renderToTexture(rtex)

    local free2 = camera:create('free', 'free2')
    free2:renderToTexture(rtex)

    game:reset()

    local free2 = camera:create('free', 'free2')
    free2:renderToTexture(rtex)
  end)
end)
