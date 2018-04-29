-- test raw geom: the object that can be requested by Recast navmesh builder from ogre render system
-- this object represents scene in the format consumable by Recast
-- verts, tris arrays
-- bmin and bmax boundaries
-- vectors are represented by float[3]

describe("#ogre getGeometry", function()
  local expected, success = json.load(TRESOURCES .. "/expected.json")

  assert.truthy(success)

  local vertices = expected.sharedgeometry.vertexbuffer.vertex

  for i, vert in pairs(vertices) do
    vertices[i].position.x = tonumber(vertices[i].position.x)
    vertices[i].position.y = tonumber(vertices[i].position.y)
    vertices[i].position.z = tonumber(vertices[i].position.z)
  end


  teardown(function()
    game:reset()
  end)

  describe("validate geom", function()
    local scales = {
      Vector3.new(1, 1, 1),
      Vector3.new(0.1, 0.1, 0.1),
      Vector3.new(20, 2, 1),
      Vector3.new(-1, -1, -1),
      Vector3.new(0, 0, 0),
    }

    local entities = {
    {
      id = "single mesh",
      render = {
        root = {
          children = {{
            type = "model",
            mesh = "Cube.mesh",
            name = "cube",
            castShadows = true,
          }},
        }
      }
    },
    {
      id = "multimesh",
      render = {
        root = {
          children = {{
            type = "node",
            position = Vector3.new(1, -2, -1),
            scale = Vector3.new(3, 2, 1),
            name = "sub",
            children = {{
              type = "model",
              mesh = "Cube.mesh",
              name = "cube",
              castShadows = true,
            }}
          }},
        }
      }
    },
    }

    local vtos = function(scale)
      return tostring(scale.x) .. ", " .. tostring(scale.y) .. ", " .. tostring(scale.z)
    end

    for _, entity in pairs(entities) do
      for _, scale in pairs(scales) do
        describe("scale " .. vtos(scale) .. ", entity: " .. entity.id , function()
          local position = Vector3.new(1, 2, 3)
          entity.render.root.scale = scale
          entity.render.root.position = position
          game:reset()
          local e = data:createEntity(entity)
          local rootNode = e:render().root

          local subNode = rootNode:getSceneNode("sub")
          if subNode then
            position = position + subNode.position * scale
            scale = scale * subNode.scale
          end

          describe("infinite BB", function()
            local bounds = BoundingBox.new(BoundingBox.EXTENT_INFINITE)
            local geom = core:render():getGeometry(bounds, 0xFF)

            local verts = geom:verts()
            local tris = geom:tris()

            it("verts ok", function()
              assert.equals(#verts, #vertices)
              for i = 1, #verts do
                local p = verts[i]
                local e = vertices[i].position

                assert.equals_float(e.x * scale.x + position.x, p.x)
                assert.equals_float(e.y * scale.y + position.y, p.y)
                assert.equals_float(e.z * scale.z + position.z, p.z)
              end
            end)

            it("tris ok", function()
              for i = 1, #tris do
                local index = tris[i]
                assert.truthy(index < #verts * 3)
              end
            end)

            local getBounds = function()
              local v = verts[1]
              local min = Vector3.new(v.x, v.y, v.z)
              local max = Vector3.new(v.x, v.y, v.z)

              for i = 1, #verts do
                local v = verts[i]
                for _, k in pairs({"x", "y", "z"}) do
                  if v[k] < min[k] then
                    min[k] = v[k]
                  end

                  if v[k] > max[k] then
                    max[k] = v[k]
                  end
                end
              end
              return min, max
            end

            it("validate bmax/bmin", function()
              local expectedbmin, expectedbmax = getBounds()
              local bmin = geom:bmin()
              local bmax = geom:bmax()

              assert.close_enough(bmin.x, expectedbmin.x, 4, 0.2)
              assert.close_enough(bmin.y, expectedbmin.y, 4, 0.2)
              assert.close_enough(bmin.z, expectedbmin.z, 4, 0.2)
              assert.close_enough(bmax.x, expectedbmax.x, 4, 0.2)
              assert.close_enough(bmax.y, expectedbmax.y, 4, 0.2)
              assert.close_enough(bmax.z, expectedbmax.z, 4, 0.2)
            end)
          end)
        end)
      end
    end
  end)

end)
