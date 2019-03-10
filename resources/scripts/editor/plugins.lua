require 'lib.class'

local time = require 'lib.time'
local lm = require 'lib.locales'
local eal = require 'lib.eal.manager'
local imguiInterface = require 'imgui.base'

local function decorate(cls)
  local RENDER_QUEUE_ASSET_VIEWER = 103
  cls.onCreate(function(self)
    self.meshNode = "__assetViewerContainer__"
    self.cameraName = "cn.flip.__assetViewerCamera__"
    self.container = self.render.root:getSceneNode(self.meshNode)
    self.aligner = self.render.root:getSceneNode(self.meshNode .. ".__assetViewerAligner__")
    if not self.container or not self.aligner then
      log.error("Failed to get mesh container")
      error("Failed to get mesh container")
    end
    self.camera = self.render.root:getCamera(self.cameraName)
    if not self.camera then
      log.error("Failed to get camera")
      error("Failed to get camera")
    end
    self.cameraNode = self.render.root:getSceneNode("cn")
    self.viewport = imgui.createOgreView("0x00000000")
    self.textureID = "__assetViewer__"
    local renderTarget = core:render():getRenderTarget(self.textureID)
    if not renderTarget then
      renderTarget = core:render():createRenderTarget(self.textureID, RenderTarget.Rtt, {
        autoUpdated = true,
        backgroundColor = "0x00000000",
        renderQueueSequence = {RENDER_QUEUE_ASSET_VIEWER},
        workspaceName = "assetViewer"
      });
    end
    self.viewport:setTexture(self.textureID)
    self.camera:attach(renderTarget)
  end)

  cls.onDestroy(function(self)
    if self.hasHandler then
      time.removeHandler(self.meshNode, self.onTime)
    end
  end)

  function cls:setEnabled(value)
    self.enabled = value
  end

  function cls:getSize()
    return 320, 320
  end

  function cls:run(asset)
    local meshName = asset.fullpath
    local w, h = imgui.GetContentRegionAvail()
    if w <= 0 or h <= 0 then
      return
    end
    self.viewport:render(w, h)

    if self.meshName == meshName then
      return
    end
    local meshID = "__assetViewerModel__"

    if not self.hasHandler then
      -- start rotating mesh container
      self.onTime = function(dt)
        if self.enabled then
          self.container:yaw(Radian.new(dt), OgreNode.TS_LOCAL)
        end
      end
      time.addHandler(self.meshNode, self.onTime)
      self.hasHandler = true
    end

    local renderData = self.render.props
    renderData.root.children[1].children[1].children = {{
      type = "model",
      mesh = meshName,
      name = meshID,
      renderQueue = RENDER_QUEUE_ASSET_VIEWER,
      workspaceName = "assetViewer",
      castShadows = false
    }}

    if pcall(function()
      self.render.props = renderData
    end) then
      self.enabled = true
      local entity = self.aligner:getEntity(meshID)
      self.aligner.position = Vector3.ZERO
      local aabb = entity:getAabb()
      local c = self.camera:getCamera()
      local distance = aabb:getRadiusOrigin() * .7 / (c.fovy.radians * 0.5)
      local dirvec = self.cameraNode.position - Vector3.ZERO
      dirvec:normalise()
      self.aligner.position = Vector3.ZERO - aabb.center
      self.container.orientation = Quaternion.IDENTITY
      self.cameraNode.position = dirvec * distance
      self.cameraNode:lookAt(Vector3.ZERO + Vector3.new(0, self.cameraNode.position.y, 0), OgreNode.TS_WORLD)
    else
      self.enabled = false
    end
    self.meshName = meshName
  end

  return cls
end

eal:extend({class = {name = "__assetViewer__", requires = {render = "ogre"}}}, decorate)

-- creates asset rendering box
local function getAssetViewer()
  local id = "__assetViewer__"
  local entity = eal:getEntity(id)
  if entity then
    return entity
  end

  local e  = data:createEntity({
    id = "__assetViewer__",
    flags = {"dynamic"},
    vars = {
      class = "__assetViewer__",
      utility = true,
      enabled = false,
    },
    render = {
      root = {
        position = Vector3.ZERO,
        children = {
          {
            type = "node",
            name = "cn",
            position = Vector3.new(40, 0, 0),
            children = {
              {
                type = "node",
                name = "flip",
                rotation = Quaternion.new(0, 0, 90, 1),
                children = {
                  {
                    type = "camera",
                    name = "__assetViewerCamera__",
                    clipDistance = 0.01,
                    bgColour = "0x000000"
                  }
                }
              }
            }
          },
          {
            type = "node",
            name = "__assetViewerContainer__",
            children = {{
              type = "node",
              name = "__assetViewerAligner__"
            }}
          },
          {
            type = "node",
            rotation = Quaternion.new(0.57, 0.17, 0.75, -0.27),
            children = {{
              type = "light",
              name = "sun",
              renderQueue = RENDER_QUEUE_ASSET_VIEWER,
              direction = Vector3.new(-1, -1, -1),
              castShadows = true,
              colourShadow = "0x00000000",
              colourDiffuse = "0xFFFFFFFF",
              lightType = "directional",
              powerScale = 0
            }}
          }
        }
      }
    }
  })

  if not e then
    return nil
  end

  return eal:getEntity(e.id)
end

local function enableTextEdit(asset)
  function asset:open(assetManager)
    assetManager.scriptEditor:openFile(asset.fullpath)
  end
end

return {
  coreSystems = {
    script = {"lua"},
    movement = {"3dmovement"}
  },
  pluginsInfo = {
    OgrePlugin = {
      systems = {
        render = {"ogre"}
      },
      assets = {
        models = {
          mesh = {
            import = function(filepath, asset)
              asset.install = "models"
              local success, viewer = pcall(getAssetViewer)
              if success then
                asset.viewer = viewer
              else
                log.error("Failed to set up asset viewer " .. viewer)
              end

              asset.addToScene = function(asset, assetManager, position)
                position = position or Vector3.new(0, 0, 0)
                local textBuffer = imgui.TextBuffer(256)
                textBuffer:write("")
                local choices = {}
                choices[lm("modals.ok")] = function()
                  local sceneEditor = imguiInterface:getView("sceneEditor")
                  if sceneEditor then
                    local name = textBuffer:read()
                    local entity = {
                      render = {
                        root = {
                          position = position,
                          children = {{
                            type = "model",
                            mesh = asset.file
                          }}
                        }
                      }
                    }
                    if name ~= "" then
                      entity.id = name
                    end

                    sceneEditor:addEntity(entity)
                  end
                end
                choices[lm("modals.cancel")] = function()
                end

                assetManager.modal:show(lm("modals.create_entity.title", {asset = asset.file}), function()
                  imgui.Text(lm("modals.create_entity.desc"))
                  imgui.InputTextWithCallback(lm("modals.create_entity.entity_id"), textBuffer, 0, function() end)
                end, choices)
              end

              asset.actions = {{
                id = "addToScene",
                callback = asset.addToScene
              }}

              return true
            end
          },
        },
        scripts = {
          material = {
            import = function(filepath, asset)
              asset.install = "materials"
              enableTextEdit(asset)
              return true
            end
          },
          compositor = {
            import = function(filepath, asset)
              enableTextEdit(asset)
              asset.install = "materials"
              return true
            end
          },
          glsl = {},
          hlsl = {},
          metal = {}
        }
      }
    },
    SDLPlugin = {
      managers = {
        input = {"sdl"},
        window = {"sdl"}
      }
    },
    CEFPlugin = {
    },
    ImGUIPlugin = {
    },
    OisInputPlugin = {
      managers = {
        input = "OIS"
      }
    },
    RecastNavigationPlugin = {
      systems = {
        navigation = {"recast"}
      }
    },
    RocketUIPlugin = {
      depends = {
        OgrePlugin = "<=1.9.0"
      },
      assets = {
        scripts = {
          rcss = {
            import = function(filepath, asset)
              asset.install = "ui"
              enableTextEdit(asset)
              return true
            end
          },
          rml = {
            import = function(filepath, asset)
              asset.install = "ui"
              enableTextEdit(asset)
              return true
            end
          }
        }
      }
    }
  }
}
