local camera = require 'factories.camera'
local event = require 'lib.event'
local lm = require 'lib.locales'
local imguiInterface = require 'imgui.base'
local icons = require 'imgui.icons'
local projectManager = require 'editor.projectManager'
local fs = require 'lib.filesystem'

require 'lib.utils'

-- scene editor
SceneEditor = class(ImguiWindow, function(self, modal, textureID, title, docked, open)
  ImguiWindow.init(self, title, docked, open)
  self.viewport = imgui.createOgreView("0xFF000000")
  self.textureID = textureID
  self.camera = nil
  self.modal = modal
  self.history = {}
  self.gizmo = imgui.createGizmo()
  self.targetID = nil
  self.gizmoEnabled = false

  self:registerContext({
    {
      icon = icons.insert_drive_file,
      action = "new",
      callback = function()
        local assets = imguiInterface:getView("assets")
        if not assets then
          self.modal:showError(lm("modals.errors.failed_to_create_asset"))
          return
        end

        local sceneName = nil
        assets:createAsset("new scene", "json", "scenes", false, function(file)
          local data = data:getSceneData()
          -- create default light
          if not data.entities then
            data.entities = {}
          end

          if not data.settings then
            data.settings = {}
          end

          data.settings.render = {
            ambientLight = {
              lowerHemisphere = "0x337F7FFF",
              upperHemisphere = "0xFF7F7FFF"
            },
            colourBackground = "0xc0c0c0FF",
            colourAmbient = "0x7F7F7FFF",
            colourDiffuse = "0xc0c0c0FF"
          }
          table.insert(data.entities, {
            id = "sun",
            render = {
              root = {
                position = Vector3.new(0, 10, 0),
                rotation = Quaternion.new(0.57, 0.17, 0.76, -0.3),
                children = {
                  {
                    type = "light",
                    colourSpecular = "0xFFFFFFFF",
                    direction = Vector3.new(-1, -1, -1),
                    name = "sun",
                    castShadows = true,
                    colourShadow = "0xAAAAAAAA",
                    colourDiffuse = "0xFFFFFFFF",
                    lightType = "directional",
                    powerScale = 5
                  }
                }
              }
            }
          })
          json.dump(file, data)
          sceneName = fs.path.filename(file)
        end)
        self:unloadScene()
        if sceneName ~= nil then
          self:loadScene(sceneName)
          self:createCamera("free", "editorMain", {utility = true, policy = EntityFactory.REUSE})
        end
      end
    },
    {
      icon = icons.save,
      action = "save",
      callback = function()
        self:saveScene()
        return false
      end,
      padding = 10
    },
    {
      icon = icons.undo,
      action = "undo",
      callback = function()
        self:undo()
      end
    },
    {
      icon = icons.redo,
      action = "redo",
      callback = function()
        self:redo()
      end,
      padding = 10
    },
    {
      icon = icons.copy,
      action = "copy",
      callback = function()
        if self.targetID then
          local e = eal:getEntity(self.targetID)
          if e then
            self.clipboard = deepcopy(e.props)
            -- erase object id for duplication
            self.clipboard.id = nil
          end
        end
      end
    },
    {
      icon = icons.paste,
      action = "paste",
      callback = function()
        if self.clipboard then
          local e = deepcopy(self.clipboard)
          self:addEntity(e)
        end
      end
    },
    {
      icon = icons.delete,
      action = "delete",
      callback = function()
        if self.targetID then
          self:removeEntity(self.targetID)
        end
      end,
      padding = 20
    },
    {
      action = "rotate",
      callback = function()
        self.gizmo.operation = imgui.gizmo.ROTATE
      end
    },
    {
      action = "move",
      callback = function()
        self.gizmo.operation = imgui.gizmo.TRANSLATE
      end
    },
    {
      action = "scale",
      callback = function()
        self.gizmo.operation = imgui.gizmo.SCALE
      end
    },
    {
      icon = icons.play,
      action = "simulationStart",
      callback = function()
        -- TODO
      end
    },
    {
      icon = icons.stop,
      action = "simulationStop",
      callback = function()
        -- TODO
      end
    }
  })

  self.onSceneLoaded = function()
    self.scene = self.loadingScene
    self:pushHistory()
    if self.dockspace then
      self.dockspace:activateDock(self.label)
    end
  end

  self.onSceneUnload = function(event)
    self.reset(event)
    self.scene = nil
    self.loadingScene = nil
    self.history = {}
    self.historyIndex = nil
  end

  self.reset = function(event)
    if Facade.BEFORE_RESET or (event.type == EntityEvent.REMOVE and event.id == self.targetID) then
      self:resetSelection()
    end
  end

  event:bind(core, Facade.LOAD, self.onSceneLoaded)
  event:bind(core, Facade.BEFORE_RESET, self.onSceneUnload)
  event:onEntity(core, EntityEvent.REMOVE, self.reset)
end)

-- create camera
function SceneEditor:createCamera(type, name, settings)
  self.camera = camera:create(type, name, settings)
  local texture = self.camera:renderToTexture(self.textureID, {
    autoUpdated = true,
    workspaceName = "ogreview",
    viewport = {
      backgroundColor = "0x00000000",
      renderQueueSequence = {
        ogre.RENDER_QUEUE_BACKGROUND,
        ogre.RENDER_QUEUE_SKIES_EARLY,
        ogre.RENDER_QUEUE_MAIN,
        ogre.RENDER_QUEUE_OVERLAY,
      }
    }
  })

  if texture then
    self.viewport:setTexture(texture.name)
  end
end

-- render scene editor
function SceneEditor:__call()
  imgui.PushStyleVar_2(ImGuiStyleVar_WindowPadding, 5.0, 5.0)
  local render = self:imguiBegin()
  imgui.PopStyleVar(ImGuiStyleVar_WindowPadding)

  if not self.open or not render then
    if self.camera then
      self.camera:setEnabled(false)
    end
    return
  end

  local w, h = imgui.GetContentRegionAvail()
  local x, y = imgui.GetCursorScreenPos()

  imguiInterface:captureMouse(not imgui.IsWindowHovered())
  if self.camera then
    self.camera:setEnabled(imgui.IsWindowFocused())
  end

  if w < 0 or h < 0 then
    self:imguiEnd()
    return
  end

  local usingGizmo = false
  self.viewport:render(w, h)
  if self.camera then
    self.gizmo:render(x, y, self.textureID)
    usingGizmo = imgui.gizmo.IsUsing()
    if usingGizmo ~= self.usingGizmo then
      self.usingGizmo = usingGizmo
      if not usingGizmo then
        self:pushHistory()
      end
    end
  end

  if not usingGizmo and imgui.IsMouseClicked(0) and imgui.IsWindowHovered() and self.camera then
    local rt = self:getRenderTarget()
    if rt then
      local _, target = rt:raycast(30, 0.1, 0xFF)
      if target then
        target = eal:getEntity(target.id)
        if not target then
          return
        end

        self:setSelection(target)
      end
    end
  end

  self:imguiEnd()
end

-- set gizmo target
function SceneEditor:setSelection(entity)
  local render = entity.render
  if render == nil then
    log.error("Can't transform target " .. entity.id .. ": no render component")
    return
  end

  if entity.vars.utility then
    return
  end

  self.targetID = entity.id
  self.gizmo:setTarget(render.root)
  self.gizmo:enable(true)
  self.gizmoEnabled = true
end

-- reset gizmo target
function SceneEditor:resetSelection()
  if not self.gizmoEnabled then
    return
  end
  self.gizmoEnabled = false
  self.gizmo:setTarget(nil)
  self.gizmo:enable(false)
end

function SceneEditor:getRenderTarget()
  if not self.camera then
    return nil
  end

  return self.camera:getRenderTarget()
end

-- add entity to scene
function SceneEditor:addEntity(props)
  local entity = data:createEntity(props)
  if not entity then
    self.modal:showError(lm("modals.errors.failed_to_add_entity", {entity = json.dumps(props)}))
    return
  end

  self:pushHistory()
  self:setSelection(eal:getEntity(entity.id))
end

-- remove entity from scene
function SceneEditor:removeEntity(id)
  if self.targetID == id then
    self:resetSelection()
  end
  if data:removeEntity(id) then
    self:pushHistory()
  end
end

-- saves current scene state in the history object
function SceneEditor:pushHistory()
  if self.historyIndex and self.historyIndex < #self.history then
    local newHistory = {}
    for i = 1,self.historyIndex do
      newHistory[i] = self.history[i]
    end
    self.history = newHistory
  end

  table.insert(self.history, data:getSceneData())
  self.historyIndex = #self.history
end

-- loads scene
function SceneEditor:loadScene(scene)
  if self.scene ~= nil then
    -- TODO: check if saved
    self:unloadScene()
  end

  game:reset(function(entity)
    return entity.vars.utility ~= true
  end)
  self.scene = nil
  self.loadingScene = scene
  game:loadScene(scene)
end

-- unloads scene
function SceneEditor:unloadScene()
  game:reset(function(entity)
    return entity.vars.utility ~= true
  end)
end

-- saves scene
function SceneEditor:saveScene()
  if self.scene == nil then
    return
  end

  if not projectManager.openProjectFile then
    return
  end

  local root = projectManager.openProjectFile.projectRoot
  data:write(self.scene, json.dumps(data:getSceneData()), fs.path.join(root, data.scenesFolder))
end

function SceneEditor:undo()
  if not self.historyIndex then
    self.historyIndex = #self.history
  end

  if self.historyIndex > 1 then
    self.historyIndex = self.historyIndex - 1
    data:setSceneData(self.history[self.historyIndex])
  end
end

function SceneEditor:redo()
  if not self.historyIndex then
    return
  end

  if self.historyIndex < #self.history then
    self.historyIndex = self.historyIndex + 1
    data:setSceneData(self.history[self.historyIndex])
  end
end
