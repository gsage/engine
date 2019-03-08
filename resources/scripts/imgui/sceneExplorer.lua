require 'lib.class'
local time = require 'lib.time'
require 'imgui.base'
local eal = require 'lib.eal.manager'
local lm = require 'lib.locales'
local icons = require 'imgui.icons'

-- allows browsing scene objects
SceneExplorer = class(ImguiWindow, function(self, title, docked, open)
  ImguiWindow.init(self, title, docked, open)
  self.icon = icons.list
end)

-- render
function SceneExplorer:__call()
  if self:imguiBegin() then
    imgui.Text(lm("sceneExplorer.entities"))
    imgui.Separator()
    local entities = core:getEntities()
    for i = 1, #entities do
      local entity = entities[i]
      if not entity.vars.utility then

        if imgui.TreeNode(entity.id) then
          local components = entity.componentNames
          if imgui.Button(lm("sceneExplorer.entity.reload")) then

            local d = data:getEntityData(entity.id)
            data:removeEntity(entity.id)
            data:createEntity(d)
          end

          for j = 1, #components do
            local component = components[j]
            if imgui.TreeNode(component) then

              imgui.Separator()
              self:renderComponentEditor(entity.id, component)
              imgui.Separator()
              imgui.TreePop()
            end
          end

          imgui.TreePop()
        end
      end
    end

    self:imguiEnd()
  end
end

local renderers = {}

local function tableEditor(t, id)
  local diff = {}

  local function propertyEditor(key, value)
    local keyID = id .. "." .. key
    if type(value) ~= "table" then
      imgui.PushID(keyID)

      local newValue, changed = nil
      if type(value) == "boolean" then
        changed, newValue = imgui.Checkbox(key, value)
      elseif type(value) == "number" then
        changed, newValue = imgui.DragFloat(key, value, 1, 0, 0, "%.3f")
        if changed then
          diff[key] = newValue
        end
      else
        local buffer = imgui.TextBuffer(30)
        buffer:write(value)
        if imgui.InputText(key, buffer, ImGuiInputTextFlags_EnterReturnsTrue) then
          diff[key] = buffer:read()
        end
        newValue = buffer:read()
      end
      if newValue ~= value then
        diff[key] = newValue
      end
      imgui.PopID()
    else
      imgui.TextWrapped(key)
      diff[key] = tableEditor(value, keyID)
    end
  end

  for key, value in pairs(t) do
    propertyEditor(key, value)
  end

  return diff
end

-- render component editor
function SceneExplorer:renderComponentEditor(id, componentName)
  local entity = eal:getEntity(id)
  local type = "generic"

  if core[componentName] then
    local info = core[componentName](core).info
    if info then
      type = info.type
    end
  end

  local function defaultRenderer()
    local component = entity[componentName]
    if component.props then
      tableEditor(component.props, id)
    else
      imgui.TextWrapped(lm("sceneExplorer.no_editor", {component = componentName, type = type}))
    end
  end

  if not renderers[componentName] then
    defaultRenderer()
    return
  end

  callback = renderers[componentName][type]
  if not callback then
    defaultRenderer()
    return
  end

  callback(entity)
end

function registerComponentEditor(component, systemType, callback)
  if not renderers[component] then
    renderers[component] = {}
  end

  renderers[component][systemType] = callback
end
