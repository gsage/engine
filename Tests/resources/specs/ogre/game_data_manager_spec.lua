require 'factories.camera'
require 'lib.eal.manager'

describe("test game data manager #ogre", function()
  it("should save and load level data", function()
    local ninja = data:createEntity({
      flags = {"dynamic"},
      id = 'ninja',
      render = {
        resources = {
          Ninja = {
            "Zip;bundles/models/v1/packs/ninja.zip"
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
    local orbit = camera:create('orbit', 'testgdmcamera', {target=ninja.id, cameraOffset=Vector3.new(0, 4, 0), distance=20, policy=REUSE})
    orbit:attach()
    game:dumpSave("savetest")
    game:reset()
    game:loadSave("savetest")
    game:reset()
    orbit:detach()
  end)
end)
