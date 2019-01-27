package.path = package.path .. ';' .. getResourcePath('scripts/?.lua') ..
                               ';' .. getResourcePath('behaviors/trees/?.lua') ..
                               ";" .. getResourcePath('behaviors/?.lua') ..
                               ";" .. getResourcePath('locales/?.lua') .. ";"

local version = _VERSION:match("%d+%.%d+")
package.path = ';' .. getResourcePath('luarocks/packages/share/lua/' .. version .. '/?.lua') ..
               ';' .. getResourcePath('luarocks/packages/share/lua/' .. version .. '/?/init.lua') .. ';' .. package.path
package.cpath = getResourcePath('luarocks/packages/lib/lua/' .. version .. '/?.so') .. ';' ..
                getResourcePath('luarocks/packages/lib/lua/' .. version .. '/?.dll') .. ';' .. package.cpath

require 'math'
require 'helpers.base'
require 'lib.behaviors'
require 'actions'
require 'factories.camera'
require 'factories.emitters'
local eal = require 'lib.eal.manager'
local event = require 'lib.event'
local lm = require 'lib.locales'

local imguiInterface = require 'imgui.base'
local icons = require 'imgui.icons'

if imguiInterface:available() then
  require 'imgui.menu'
  require 'imgui.console'
  require 'imgui.ogreView'
  require 'imgui.stats'
  require 'imgui.transform'
  require 'imgui.sceneExplorer'
  require 'imgui.settings'
  require 'imgui.tools'
  require 'imgui.scriptEditor'
  require 'imgui.projectWizard'
end

local rocketInitialized = false

local globalEditorState = editor:getGlobalState()
if globalEditorState.settings then
  lm:setLocale(globalEditorState.settings.locale)
end

function setOrbitalCam()
end

function spawn()
  data:createEntity(getResourcePath('characters/ninja.json'), {movement = {speed = 10}})
end

function initLibrocket(event)
  local ctx = rocket.contexts[event.name]
  if not rocketInitialized then
    local fonts =
    {
      "Delicious-Roman.otf",
      "Delicious-BoldItalic.otf",
      "Delicious-Bold.otf",
      "Delicious-Italic.otf",
      "lucida.ttf"
    }
    for _, font in pairs(fonts) do
      resource.loadFont(font)
    end
  end

  main = resource.loadDocument(ctx, "minimal.rml")
  cursor = resource.loadCursor(ctx, "cursor.rml")

  main:Show()
end

-- librocket initialization
if event.onRocketContext ~= nil then
  event:onRocketContext(core, RocketContextEvent.CREATE, initLibrocket)
end

local selectTransform = false

function handleKeyEvent(e)
  if e.type == KeyboardEvent.KEY_UP then
    selectTransform = false
    return
  end

  if e.key == Keys.KC_T and e.type == KeyboardEvent.KEY_DOWN then
    selectTransform = true
  end
end

event:onKeyboard(core, KeyboardEvent.KEY_DOWN, handleKeyEvent)
event:onKeyboard(core, KeyboardEvent.KEY_UP, handleKeyEvent)

local initialized = false

local function saveDockState()
  local state = imgui.GetDockState()
  editor:putToGlobalState("dockState", state)
  log.info("Saving imgui dock state")
  if not editor:saveGlobalState() then
    log.error("Failed to save global state")
  end
end

local function onSelect(e)
  local target = eal:getEntity(e.entity)
  if not target then
    return
  end

  ogreView:setGizmoTarget(target)
end

event:onOgreSelect(core, SelectEvent.OBJECT_SELECTED, onSelect)

local function startEditors()
  if imguiInterface:available() then
    imguiConsole = Console(256, true)
    imguiInterface:addView("console", imguiConsole)

    ogreView = OgreView("viewport", "viewport", true)
    imguiInterface:addView("ogreView", ogreView)

    scriptEditor = ScriptEditor("script", "script_editor", true)
    imguiInterface:addView("script", scriptEditor)

    local function onAreaLoad(event)
      ogreView:createCamera("free", {utility = true})
    end

    event:bind(core, Facade.LOAD, onAreaLoad)

    transform = Transform(ogreView, "transform", true)

    imguiInterface:addView("transform", transform)

    stats = Stats("stats", true)

    imguiInterface:addView("stats", stats)

    imguiInterface:addView("assets", function()
      imgui.TextWrapped(icons.directions_run .. " coming soon")
    end, true)

    sceneExplorer = SceneExplorer("scene explorer", true)

    imguiInterface:addView("sceneExplorer", sceneExplorer)

    imguiInterface:addView("debug", function()
      if imgui.Button("load example scene") then
        game:reset()
        game:loadArea("exampleLevel")
      end
    end, true)

    settings = SettingsView(true)
    imguiInterface:addView("settings", settings)

    local states = {}
    imguiInterface:addView("dock states", function()

      for i = 1, 5 do
        imgui.BeginChildFrame(i, 100, 70)
        imgui.Text(i)

        if imgui.Button("save") then
          states[i] = imgui.GetDockState()
        end


        if states[i] then
          imgui.SameLine()
          if imgui.Button("load") then
            if states[i] then
              imgui.SetDockState(states[i])
            end
          end
        end

        imgui.EndChildFrame()
      end
    end)

    tools = ToolsView(true)
    imguiInterface:addView("tools", tools)

    local views = {}
    for name, view in pairs(imguiInterface.views) do
      views[#views+1] = MenuItem("window_title." .. view.label,
        function() view.open = not view.open end,
        function() return view.open end
      )
    end

    local menus = {
      List("views", views)
    }
    imguiInterface:addView("mainMenu", Menu(menus))

    -- read saved editor settings
    if globalEditorState and globalEditorState.dockState then
      log.info("Restoring imgui dock state")
      imgui.SetDockState(globalEditorState.dockState)
    end
    event:bind(core, EngineEvent.STOPPING, saveDockState)

    local systemEditors = {}

    local function createEditorView(systemType)
      local success, editorView = pcall(function() return require('imgui.systems.' .. systemType) end)
      if not success then
        log.warn("Failed to load editor for system " .. systemType)
      else
        systemEditors[systemType] = editorView(true)
        imguiInterface:addView(systemType, systemEditors[systemType])
      end
    end

    local function onSystemUpdate(event)
      local systemType = event.system.info.type

      if event.type == EngineEvent.SYSTEM_ADDED then
        createEditorView(systemType)
      else
        if systemEditors[systemType] then
          imguiInterface:removeView(systemType, systemEditors[systemType])
        end
      end
    end

    event:onSystemChange(core, SystemChangeEvent.SYSTEM_ADDED, onSystemUpdate)
    event:onSystemChange(core, SystemChangeEvent.SYSTEM_REMOVED, onSystemUpdate)

    local systems = core:getSystems()
    for name, system in pairs(systems) do
      if system.info then
        local systemType = system.info.type
        if systemType then
          createEditorView(systemType)
        end
      end
    end
  end
end

if imguiInterface:available() then
  -- starting from project creation wizard
  if not cef then
    log.error("editor requires CEF plugin to be installed")
    game:shutdown(1)
  end

  wizard = ProjectWizard("wizard", "wizard", true)
  local a, b = pcall(function()
    imguiInterface:addView("wizard", wizard)
  end)

  cef:addMessageHandler("createProject", function(projectType)
    if projectType ~= "preview" then
      return
    end
    imguiInterface:removeView("wizard", wizard)
    startEditors()
  end)
else
  log.error("editor requires ImGUI plugin to be installed")
  game:shutdown(1)
end
