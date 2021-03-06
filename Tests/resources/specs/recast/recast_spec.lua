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
          "Zip;bundles/models/".. version .. "/packs/gunspider.zip"
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
          colourSpecular = "0xFFFFFFFF",
          colourShadow = "0xffffffff",
          direction = Vector3.new(0, -0.5, 0.5),
          name = "sun",
          castShadows = true,
          colourDiffuse = "0xFFFFFFFF",
          lightType = "directional",
          powerScale = 8,
        },}
      }
    }
  }

  local navmeshVerifyCases = {
    entity = {
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
  }

  -- test Ogre::Item
  if core:render().info.type == "ogre" and core:render().info.version >= 0x020100 then
    navmeshVerifyCases.item = {
      id = "cube",
      render = {
        root = {
          scale = Vector3.new(20, 20, 20),
          children = {{
            type = "item",
            mesh = "CubeV2.mesh",
            castShadows = true
          },}
        }
      }
    }
  end

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

    for id, entity in pairs(navmeshVerifyCases) do
      for name, settings in pairs(settingsTestCases) do
        -- disable merging to make each iteration generate brand new navmesh
        settings.merge = false

        describe(id .. " " .. name .. " settings", function()
          local checkPoints = {
            tl = geometry.Vector3.new(30, 30, 30),
            tr = geometry.Vector3.new(30, 30, -30),
            bl = geometry.Vector3.new(-30, 30, 30),
            br = geometry.Vector3.new(30, 30, -30),
          }

          for name, pos in pairs(checkPoints) do
            it("check " .. name .. " point", function()
              game:reset()
              local area = data:createEntity(entity)
              assert.is_not.is_nil(area)
              local entity = eal:getEntity(area.id)
              assert.is_not.is_nil(entity.render.root)
              local orbit = camera:create('orbit', 'testcamera', {target=area.id, cameraOffset=Vector3.new(0, 0.5, 0), distance=20})
              assert.truthy(core:navigation():rebuildNavMesh(settings))

              local result, found = core:navigation():findNearestPointOnNavmesh(pos)
              assert.truthy(found)
              local s = entity.render.root.scale
              pos = pos / 30
              assert.close_enough(result.x, pos.x * s.x, 2, 0.1)
              assert.close_enough(result.y, pos.y * s.y, 2, 0.1)
              assert.close_enough(result.z, pos.z * s.z, 2, 0.1)
            end)
          end
        end)
      end
    end

    it("must handle empty scene", function()
      game:reset()
      assert.falsy(core:navigation():rebuildNavMesh(defaultRecastOptions))
    end)

    describe("cache", function()
      it("reload works", function()
        -- TODO
      end)
    end)
  end)


  describe("movement", function()
    game:reset()
    cfg = core:render().config
    cfg.settings = {
      ambientLight = {
        lowerHemisphere = "0x337F7FFF",
        upperHemisphere = "0xFF7F7FFF"
      },
      colourBackground = "0xc0c0cFF",
      colourAmbient = "0x7F7F7FFF",
      colourDiffuse = "0xc0c0c0FF"
    }
    core:configureSystem("render", cfg, false)

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
      -- TODO
    end)
  end)
end)
