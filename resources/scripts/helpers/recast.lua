-- recast utility methods

local recast = {}

function recast.visualizePath(points, drawPoints, name)
  name = name or "line"
  local children = {{
    type = "manualObject",
    name = name .. "path",
    data = {
      material = {
        techniques = {{
          passes = {{
            depthCheckEnabled = false,
            colors = {
              diffuse = "0xFFCC00",
              ambient = "0xffcc00",
              illumination = "0xFFCC00",
            }
          },}
        },}
      },
      renderOperation = ogre.OT_LINE_STRIP,
      points = points,
    }
  },}
  if drawPoints then
    children[#children + 1] = {
      type = "manualObject",
      name = name .. "dots",
      data = {
        material = {
          techniques = {{
            passes = {{
              depthCheckEnabled = false,
              colors = {
                diffuse = "0xFFDD33",
                ambient = "0xFFDD33",
                illumination = "0xFFDD33",
              }
            },}
          },}
        },
        renderOperation = ogre.OT_POINT_LIST,
        points = points,
      }
    }
  end

  return data:createEntity({
    id = name .. "path",
    render = {
      root = {
        position = Vector3.new(0, 0, 0),
        rotation = Quaternion.new(1, 0, 0, 0),
        children = children
      }
    }
  })
end

function recast.visualizeNavmesh()
  if core.navigation then
    local points = core:navigation():getNavMeshRawPoints()

    local children = {{
      type = "manualObject",
      name = "navmesh",
      data = {
        material = {
          techniques = {{
            passes = {{
              colors = {
                diffuse = "0x335555",
                ambient = "0x335555",
                illumination = "0x114499",
              }
            },}
          },}
        },
        renderOperation = ogre.OT_TRIANGLE_LIST,
        points = points,
      }
    },{
      type = "manualObject",
      name = "points",
      data = {
        material = {
          techniques = {{
            passes = {{
              colors = {
                diffuse = "0x335555",
                ambient = "0x335555",
                illumination = "0x1199FF",
              }
            },}
          },}
        },
        renderOperation = ogre.OT_POINT_LIST,
        points = points,
      }
    },}

    local index = 1

    for i = 1, #points / 3 do
      local line = {}

      for j = 1, 3 do
        line[j] = points[index]
        index = index + 1
      end

      line[4] = line[1]

      children[#children + 1] = {
        type = "manualObject",
        name = "outline" .. tostring(#children),
        data = {
          material = {
            techniques = {{
              passes = {{
                colors = {
                  diffuse = "0x114444",
                  ambient = "0x114444",
                  illumination = "0x113355",
                }
              },}
            },}
          },
          renderOperation = ogre.OT_LINE_LIST,
          points = line,
        }
      }
    end

    return data:createEntity({
      id = "navmesh",
      render = {
        root = {
          position = Vector3.new(0, 0, 0),
          rotation = Quaternion.new(1, 0, 0, 0),
          children = children
        }
      }
    })
  end

  return nil
end

return recast
