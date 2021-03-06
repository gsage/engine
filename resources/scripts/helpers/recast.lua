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
              diffuse = "0xFFFFCC00",
              ambient = "0xFFffcc00",
              illumination = "0xFFFFCC00",
            },
            vertexProgram = "stdquad_vp",
            fragmentProgram = "stdquad_fp",
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
              },
              vertexProgram = "stdquad_vp",
              fragmentProgram = "stdquad_fp",
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

function recast.visualizeGeom()
  local bounds = BoundingBox.new(BoundingBox.EXTENT_INFINITE)
  local geom = core:render():getGeometry(bounds, RenderComponent.STATIC)

  local verts = geom:verts()
  local tris = geom:tris()
  local norms = geom:normals()

  local points = {}
  for i = 1,#verts do
    points[i] = verts[i]
  end

  local indices = {}
  for i = 1,#tris do
    indices[i] = tris[i]
  end

  local normals = {}
  for i = 1,#norms do
    normals[i] = norms[i]
  end

  return data:createEntity({
    id = "geom",
    render = {
      root = {
        position = Vector3.new(0, 0, 0),
        rotation = Quaternion.new(1, 0, 0, 0),
        children = {
          {
            type = "manualObject",
            name = "rawgeom",
            data = {
              material = {
                techniques = {{
                  passes = {{
                    colors = {
                      diffuse = "0xFF335555",
                      ambient = "0xFF335555",
                      illumination = "0xFF114499",
                    },
                    vertexProgram = "stdquad_vp",
                    fragmentProgram = "stdquad_fp",
                  },}
                },}
              },
              renderOperation = ogre.OT_TRIANGLE_LIST,
              points = points,
              indices = indices,
              normals = normals,
            }
          }
        }
      },
      vertexProgram = "stdquad_vp",
      fragmentProgram = "stdquad_fp",
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
                diffuse = "0xFF335555",
                ambient = "0xFF335555",
                illumination = "0xFF114499",
              },
              vertexProgram = "stdquad_vp",
              fragmentProgram = "stdquad_fp",
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
                diffuse = "0xFF335555",
                ambient = "0xFF335555",
                illumination = "0xFF1199FF",
              },
              vertexProgram = "stdquad_vp",
              fragmentProgram = "stdquad_fp",
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
                  diffuse = "0xFF114444",
                  ambient = "0xFF114444",
                  illumination = "0xFF113355",
                },
                vertexProgram = "stdquad_vp",
                fragmentProgram = "stdquad_fp",
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
        },
        vertexProgram = "stdquad_vp",
        fragmentProgram = "stdquad_fp",
      }
    })
  end

  return nil
end

return recast
