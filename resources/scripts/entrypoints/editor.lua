package.path = package.path .. ';' .. getResourcePath('scripts/?.lua') ..
                               ';' .. getResourcePath('behaviors/trees/?.lua') ..
                               ";" .. getResourcePath('behaviors/?.lua') .. ";"

require 'math'
require 'helpers.base'
require 'lib.behaviors'
require 'actions'
require 'factories.camera'
require 'factories.emitters'
local eal = require 'lib.eal.manager'
local event = require 'lib.event'

local imguiInterface = require 'imgui.base'

if imguiInterface:available() then
  require 'imgui.menu'
  require 'imgui.console'
  require 'imgui.ogreView'
  require 'imgui.stats'
  require 'imgui.transform'
  require 'imgui.sceneExplorer'
end

local rocketInitialized = false

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

event:bind(core, Facade.LOAD, onReady)

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

if imguiInterface:available() then
  imguiConsole = Console(256, true)
  imguiInterface:addView("console", imguiConsole)

  ogreView = OgreView("viewport", "viewport", true)
  imguiInterface:addView("ogreView", ogreView)

  local function onAreaLoad(event)
    ogreView:createCamera("free")
  end

  event:bind(core, Facade.LOAD, onAreaLoad)

  transform = Transform(ogreView, "transform", true)

  imguiInterface:addView("transform", transform)

  stats = Stats("stats", true)

  imguiInterface:addView("stats", stats)

  imguiInterface:addView("assets", function()
    imgui.TextWrapped("coming soon")
  end, true)

  sceneExplorer = SceneExplorer("scene explorer", true)

  imguiInterface:addView("sceneExplorer", sceneExplorer)

  imguiInterface:addView("debug", function()
    if imgui.Button("load example scene") then
      game:reset()
      game:loadArea("exampleLevel")
      core:movement():rebuildNavMesh()
      spawn()
    end
  end, true)

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

  local views = {}
  for name, view in pairs(imguiInterface.views) do
    views[#views+1] = MenuItem(view.title,
      function() view.open = not view.open end,
      function() return view.open end
    )
  end

  local menus = {
    List("Views", views)
  }
  imguiInterface:addView("mainMenu", Menu(menus))

  -- read saved editor settings
  local globalEditorState = editor:getGlobalState()
  if globalEditorState and globalEditorState.dockState then
    log.info("Restoring imgui dock state")
    imgui.SetDockState(globalEditorState.dockState)
  end
  event:bind(core, EngineEvent.STOPPING, saveDockState)
end

function reload()
  game:loadArea("exampleLevel")
  game:reset()
  game:loadArea("exampleLevel")
end
