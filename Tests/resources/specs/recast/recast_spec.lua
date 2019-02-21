local async = require 'lib.async'
local event = require 'lib.event'
local recast = require 'helpers.recast'
require 'factories.camera'

describe("#recast #ogre", function()
  local version = "v1"
  if core:render().info.type == "ogre" and core:render().info.version >= 0x020100 then
    version = "v2"
  end

  local actor = {
    id = "recastNavigationTester",
    render = {
      resources = {
        Gunspider = {
          "Zip:bundles/models/".. version .. "/packs/gunspider.zip"
        }
      },
      animations = {
        states = {
          idle = {
            base = "model.idle"
          },
          walk = {
            base = "model.walk"
          }
        },
        defaultState = "idle"
      },
      root = {
        position = Vector3.new(10, 0, 20),
        scale = Vector3.new(0.5, 0.5, 0.5),
        orientationVector =  Vector3.new(1, 0, 0),
        children = {{
          query = "dynamic",
          type = "model",
          mesh = "gunspider.mesh",
          name = "model",
          castShadows = true,
        },
        {
          type = "node",
          name = "child",
          scale = Vector3.new(0.5, 0.5, 0.5),
          position = Vector3.new(0, 1, 0.2),
          rotation = Quaternion.new(1.0, 0, -1.0, 0),
          children = {{
            query = "dynamic",
            type = "model",
            mesh = "arrow.mesh",
            name = "arrow",
            castShadows = true,
          }}
        }}
      }
    },
    movement = {
      speed = 150,
      moveAnimation = "walk",
      animSpeedRatio = 0.30,
    },
    navigation = {
      align = true,
      alignNormal = true,
    }
  }

  local testArea = {
    id = "testArea",
    render = {
      root = {
        position = Vector3.new(0, 0, 0),
        rotation = Quaternion.new(1, 0, 0, 0),
        children = {{
          type = "model",
          mesh = "testArea.mesh",
          castShadows = true,
          name = "recastTestArea"
        },}
      }
    }
  }

  local light = {
    id = "lamp",
    render = {
      root = {
        position = Vector3.new(40, 10, 40),
        rotation = Quaternion.new(0.5, 0, 0, 0),
        children = {{
          type = "light",
          colourSpecular = "0xFFFFFF",
          direction = Vector3.new(0, -0.5, 0.5),
          name = "lamp",
          castShadows = true,
          colourDiffuse = "0xFFFFFF",
          lightType = "point"
        },}
      }
    }
  }

  -- 1x1x1 cube to verify navmesh generator
  local simpleTestArea = {
    id = "cube",
    render = {
      root = {
        scale = Vector3.new(20, 20, 20),
        children = {{
          type = "model",
          mesh = "Cube.mesh",
          castShadows = true
        },}
      }
    }
  }

  local defaultRecastOptions = {
    walkableSlopeAngle = 45,
    merge = false,
    tileSize = 300,
    walkableRadius = 3,
    walkableClimb = 2,
  }

  setup(function()
    game:reset()
    assert.truthy(game:loadPlugin("RecastNavigationPlugin"))
    assert.truthy(game:createSystem("3dmovement"))
    assert.truthy(game:createSystem("recast"))
  end)

  teardown(function()
    game:reset()
    core:removeSystem("navigation")
    core:removeSystem("movement")
    assert:truthy(game:unloadPlugin("RecastNavigationPlugin"))
  end)

  local settingsTestCases = {
    default = {
      merge = false,
      walkableRadius = 0.6
    }
  }

  describe("rebuild", function()
    game:reset()
    local area = data:createEntity(simpleTestArea)
    local orbit = camera:create('orbit', 'testcamera', {target=area.id, cameraOffset=Vector3.new(0, 0.5, 0), distance=20})

    for name, settings in pairs(settingsTestCases) do
      -- disable merging to make each iteration generate brand new navmesh
      settings.merge = false

      describe(name .. " settings", function()
        assert.truthy(core:navigation():rebuildNavMesh(settings))
        local checkPoints = {
          tl = geometry.Vector3.new(30, 30, 30),
          tr = geometry.Vector3.new(30, 30, -30),
          bl = geometry.Vector3.new(-30, 30, 30),
          br = geometry.Vector3.new(30, 30, -30),
        }

        for name, pos in pairs(checkPoints) do
          it("check " .. name .. " point", function()
            local result, found = core:navigation():findNearestPointOnNavmesh(pos)
            assert.truthy(found)
            local s = simpleTestArea.render.root.scale
            pos = pos / 30
            assert.close_enough(result.x, pos.x * s.x, 2, 0.1)
            assert.close_enough(result.y, pos.y * s.y, 2, 0.1)
            assert.close_enough(result.z, pos.z * s.z, 2, 0.1)
          end)
        end
      end)
    end

    it("must handle empty scene", function()
      game:reset()
      assert.falsy(core:navigation():rebuildNavMesh(defaultRecastOptions))
    end)

    describe("cache", function()
      it("reload works", function()
      end)
    end)
  end)


  describe("movement", function()
    game:reset()
    assert.truthy(data:createEntity(actor))
    assert.truthy(data:createEntity(light))
    assert.truthy(data:createEntity(testArea))
    local orbit = camera:create('orbit', 'testcamera', {target=actor.id, cameraOffset=Vector3.new(0, 0.5, 0), distance=20})
    assert.is_not.is_nil(orbit.attach)
    assert.truthy(orbit:attach())
    assert.is_not.is_nil(orbit.renderTargetName)
    assert.truthy(core:navigation():rebuildNavMesh(defaultRecastOptions))

    if core:render().info.type == "ogre" and core:render().info.version < 0x020100 then
      recast.visualizeNavmesh()
    end

    local actorEntity = eal:getEntity(actor.id)

    it("works", function()
      local assertPosition = function(x, y, z)
        local position = actorEntity.render.position
        assert.close_enough(position.x, x, 4, 0.2)
        assert.close_enough(position.y, y, 4, 0.2)
        assert.close_enough(position.z, z, 4, 0.2)
      end

      -- walk randomly using navmesh
      actorEntity.navigation:go(-6.987621307373,	10.210243225098,	-12.846557617188)
      async.waitSeconds(2)
      assertPosition(-6.987621307373,	10.210243225098,	-12.846557617188)
      actorEntity.navigation:go(15.985151290894,	0.61024355888367,	16.548219680786)
      async.waitSeconds(2)
      assertPosition(15.985151290894,	0.61024355888367,	16.548219680786)
      actorEntity.navigation:go(-3.3225250244141,	0.61024355888367,	-16.12223815918)
      async.waitSeconds(1)
      assertPosition(-3.3225250244141,	0.61024355888367,	-16.12223815918)
    end)

    it("agent paging works", function()
    end)
  end)
end)
