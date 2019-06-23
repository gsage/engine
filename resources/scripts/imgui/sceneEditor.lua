local camera = require 'factories.camera'
local factoryUtils = require 'factories.utils'
local factories = require 'factories.base'
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

  self.selectedObjects = {}
  self.clipboard = {}

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
        self.clipboard = {}
        for id in pairs(self.selectedObjects) do
          local e = eal:getEntity(id)
          if e then
            local copy = deepcopy(e.props)
            -- erase object id for duplication
            copy.id = nil
            table.insert(self.clipboard, copy)
          end
        end
      end
    },
    {
      icon = icons.paste,
      action = "paste",
      callback = function()
        if self.clipboard then
          self:resetSelection()
          for _, e in ipairs(self.clipboard) do
            self:addEntity(deepcopy(e), false)
          end
          self:pushHistory()
        end
      end
    },
    {
      icon = icons.delete,
      action = "delete",
      callback = function()
        local entities = {}
        for id in pairs(self.selectedObjects) do
          table.insert(entities, id)
        end

        for _, id in pairs(entities) do
          self:removeEntity(id, false)
        end
        self:pushHistory()
      end,
    },
    {
      icon = icons.plus,
      action = "createObject",
      callback = function()
        self:createObject()
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
    self:resetSelection()
    self.scene = nil
    self.loadingScene = nil
    self.history = {}
    self.historyIndex = nil
  end

  self.reset = function(event)
    if event.type == EntityEvent.REMOVE and self.selectedObjects[event.id] then
      self.selectedObjects[event.id] = nil
    end
  end

  event:bind(core, Facade.LOAD, self.onSceneLoaded)
  event:bind(core, Facade.BEFORE_RESET, self.onSceneUnload)
  event:onEntity(core, EntityEvent.REMOVE, self.reset)

  self.popupID = "sceneEditorPopup"
end)

SceneEditor.ENTITIES = 0x01
SceneEditor.SETTINGS = 0x02

-- create camera
function SceneEditor:createCamera(type, name, settings)
  self.camera = camera:create(type, name, settings)
  local texture = self.camera:renderToTexture(self.textureID, {
    autoUpdated = true,
    -- V2 workspace
    workspaceName = "ogreview",
    -- V1 viewport settings
    viewport = {
      compositors = {
        "Utility/Highlight"
      },
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

  local grid = eal:getEntity("__grid__")
  if not grid then
    data:createEntity({
      id = "grid",
      vars = {
        utility = true
      },
      render = {
        root = {
          children = {
            {
              type = "billboard",
              name = "__grid__",
              commonUpVector = Vector3.new(0, 0, 1),
              commonDirection = Vector3.new(0, 1, 0),
              billboardType = "BBT_PERPENDICULAR_COMMON",
              materialName = "Utility.Grid",
              billboards = {
                {
                  position = Vector3.new(0, 0, 0),
                  width = 20,
                  height = 20
                }
              }
            }
          }
        }
      }
    })
  end
end

-- render scene editor
function SceneEditor:__call()
  imgui.PushStyleVar_2(ImGuiStyleVar_WindowPadding, 5.0, 5.0)
  local render = self:imguiBegin()
  imgui.PopStyleVar(ImGuiStyleVar_WindowPadding)
  if self.openPopup then
    imgui.OpenPopup(self.openPopup)
    self.openPopup = nil
  end

  self:renderPopup(self.popupID, self.popupContent)

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

  local target = nil
  if imgui.IsWindowHovered() and self.camera then
    local rt = self:getRenderTarget()
    if rt then
      local point
      point, target = rt:raycast(30, 0.1, ogre.RAYCAST_DEFAULT_MASK + ogre.QUERY_ENTITY_DEFAULT_MASK + ogre.QUERY_FX_DEFAULT_MASK)
    end
  end

  if self.hovered and (not target or target.id ~= self.hovered.id) then
    self.hovered:setHovered(false)
    self.hovered = nil
  end

  if not usingGizmo and imgui.IsWindowHovered() then
    if target then
      target = eal:getEntity(target.id)
      if not target then
        return
      end

      if imgui.IsMouseClicked(0) then
        -- deselect all if not modifiers are pressed
        if not imgui.KeyShift() then
          self:resetSelection()
        end

        -- deselect
        if imgui.KeyShift() and self.selectedObjects[target.id] then
          self:resetSelection(target)
        else
          -- select
          self:setSelection(target)
        end
      end

      if target.setHovered then
        target:setHovered(true)
        self.hovered = target
      end
    elseif imgui.IsMouseClicked(0) and not imgui.KeyShift() then
      self:resetSelection()
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

  if entity.setSelected then
    entity:setSelected(true)
    if render.root then
      self.gizmo:addTarget(render.root)
    end
  end
  self.selectedObjects[entity.id] = entity
end

-- reset gizmo target
function SceneEditor:resetSelection(target)
  local function deselect(id)
    if not self.selectedObjects[id] then
      return
    end

    local e = eal:getEntity(id)
    if e and e.setSelected then
      e:setSelected(false)
      self.gizmo:removeTarget(e.render.root)
    end
    self.selectedObjects[id] = nil
  end

  if target then
    deselect(target.id)
  else
    for id in pairs(self.selectedObjects) do
      deselect(id)
    end

    self.gizmo:resetTargets()
  end
end

function SceneEditor:getRenderTarget()
  if not self.camera then
    return nil
  end

  return self.camera:getRenderTarget()
end

-- add entity to scene
function SceneEditor:addEntity(props, pushHistory)
  local entity = data:createEntity(props)
  if not entity then
    self.modal:showError(lm("modals.errors.failed_to_add_entity", {entity = json.dumps(props), details = lm("modals.errors.details.failed_to_create_object")}))
    return
  end

  if pushHistory == nil or pushHistory then
    self:pushHistory()
  end
  self:setSelection(eal:getEntity(entity.id))
end

-- remove entity from scene
function SceneEditor:removeEntity(id, pushHistory)
  if data:removeEntity(id) then
    if pushHistory == nil or pushHistory then
      self:pushHistory()
    end
  end
end

-- saves current scene state in the history object
function SceneEditor:pushHistory(flags)
  if self.historyIndex and self.historyIndex < #self.history then
    local newHistory = {}
    for i = 1,self.historyIndex do
      newHistory[i] = self.history[i]
    end
    self.history = newHistory
  end

  flags = flags or SceneEditor.ENTITIES

  local historyRecord = data:getSceneData()
  if bit.band(flags, SceneEditor.ENTITIES) == 0 then
    historyRecord.entities = nil
  end

  if bit.band(flags, SceneEditor.SETTINGS) == 0 then
    historyRecord.settings = nil
  end

  table.insert(self.history, historyRecord)
  self.historyIndex = #self.history
end

-- loads scene
function SceneEditor:loadScene(scene)
  self:unloadScene()
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

function SceneEditor:createObject()
  -- load all entity factories
  factoryUtils.loadAll()
  self.popupContent = {}
  for id, value in pairs(factories) do
    local popupID = self.popupID .. "." .. id

    table.insert(self.popupContent,
      function()
        local menuText = lm("viewport.factories." .. id .. ".menu")
        if menuText == lm.MISSING then
          menuText = id
        end
        if imgui.BeginMenu(menuText) then
          for _, t in ipairs(value:getAvailableTypes()) do
            text = lm("viewport.factories." .. id .. "." .. t)
            if text == lm.MISSING then
              text = t
            end
            if imgui.MenuItem(text) then
              if not self.camera or not self.scene then
                self.modal:showError(lm("modals.errors.failed_to_add_entity", {entity = text .. " " .. menuText, details = lm("modals.errors.details.no_scene_opened")}))
              else
                local success, result = pcall(function()
                  return value:create(t)
                end)
                local entity = result

                if entity and success then
                  self:resetSelection()
                  self:setSelection(entity)
                else
                  local details = lm("modals.errors.details.failed_to_create_object")
                  if not success then
                    details = ": " .. tostring(result)
                  end

                  self.modal:showError(lm("modals.errors.failed_to_add_entity", {entity = text .. " " .. menuText, details = details}))
                end
              end
            end
          end
          imgui.EndMenu()
        end
      end
    )
  end

  self.openPopup = self.popupID
end

function SceneEditor:renderPopup(id, items)
  if imgui.BeginPopup(id) and items then
    for _, item in ipairs(items) do
      item()
    end
    imgui.EndPopup()
    return true
  end

  return false
end
