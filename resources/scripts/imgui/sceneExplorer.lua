require 'lib.class'
local time = require 'lib.time'
require 'imgui.base'
local eal = require 'lib.eal.manager'

-- allows browsing scene objects
SceneExplorer = class(ImguiWindow, function(self, title, docked, open)
  ImguiWindow.init(self, title, docked, open)
end)

-- render
function SceneExplorer:__call()
  if self:imguiBegin() then
    imgui.Text("Entities")
    imgui.Separator()
    local entities = core:getEntities()
    for i = 1, #entities do
      local entity = entities[i]
      if imgui.TreeNode(entity.id) then
        local components = entity.componentNames
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
      local buffer = imgui.TextBuffer(30)
      buffer:write(value)
      if imgui.InputText(key, buffer, ImGuiInputTextFlags_EnterReturnsTrue) then
        diff[key] = buffer:read()
      end

      imgui.PopID()
      local newVal = buffer:read()
      if newVal ~= value then
        diff[key] = newVal
      end
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
function SceneExplorer:renderComponentEditor(id, component)
  local entity = eal:getEntity(id)
  local type = "generic"

  if core[component] then
    local info = core[component](core).info
    if info then
      type = info.type
    end
  end

  local function defaultRenderer()
    local component = entity[component]
    if component.props then
      component.props = tableEditor(component.props, id)
    else
      imgui.TextWrapped("No editor defined for component " .. component .. ": " .. type)
    end
  end

  if not renderers[component] then
    defaultRenderer()
    return
  end

  callback = renderers[component][type]
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
