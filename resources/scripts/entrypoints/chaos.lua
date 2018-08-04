-- this entrypoint is a mess, which is used mostly for concepts test purposes

package.path = package.path .. ';' .. getResourcePath('scripts/?.lua') ..
                               ';' .. getResourcePath('behaviors/trees/?.lua') ..
                               ";" .. getResourcePath('behaviors/?.lua') .. ";"

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

if imgui then
  require 'imgui.console'
  require 'imgui.stats'
  require 'imgui.ogreView'
end

local event = require 'lib.event'
local eal = require 'lib.eal.manager'
local async = require "lib.async"

local rocketInitialized = false

function onRoll(e)
  if e.type == OgreSelectEvent.ROLL_OVER then
    local target = eal:getEntity(e.entity)
    if e:hasFlags(RenderComponent.DYNAMIC) then
      cursor:GetElementById("cursorIcon"):SetClass("attack", actions.attackable(player, target))
    end
  else
    cursor:GetElementById("cursorIcon"):SetClass("attack", false)
  end
end

function initLibrocket(e)
  local ctx = rocket.contexts[e.name]
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
  console = resource.loadDocument(ctx, "console_document.rml")
  healthbar = resource.loadDocument(ctx, "healthbar.rml")

  cursor = resource.loadCursor(ctx, "cursor.rml")

  main:Show()
  healthbar:Show()
  main:AddEventListener('keydown', onKeyEvent, true)

  console:AddEventListener('keydown', onKeyEvent, true)

  event:onOgreSelect(core, SelectEvent.ROLL_OVER, onRoll)
  event:onOgreSelect(core, SelectEvent.ROLL_OUT, onRoll)
end

-- librocket initialization
if event.onRocketContext ~= nil then
  event:onRocketContext(core, RocketContextEvent.CREATE, initLibrocket)
end

function setOrbitalCam(id, cameraID)
  --camera:createAndAttach('orbit', 'orbo', {target='sinbad', cameraOffset=Vector3.new(0, 4, 0), distance=20, policy=EntityFactory.REUSE})
  --camera:createAndAttach('free', 'orbo', {target='sinbad', cameraOffset=Vector3.new(0, 4, 0), distance=20, policy=EntityFactory.REUSE})
end

function spawnMore(count)
  for i=1, count do
    spawn()
  end
end

function spawn()
  data:createEntity(getResourcePath('characters/gunspider.json'), {movement = {speed = 10}})
end

function onKeyEvent(event)
  if event.parameters['key_identifier'] == rocket.key_identifier.F9 then
    if console_visible then
      console:Hide()
      main:Focus()
    else
      console:Show(DocumentFocus.NONE)
    end
    console_visible = not console_visible
  end
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

function onSelect(e)
  local target = eal:getEntity(e.entity)

  if selectTransform then
    ogreView:setGizmoTarget(target)
    return
  end

  if e:hasFlags(RenderComponent.DYNAMIC) then
    if actions.attackable(player, target) then
      player.script.state.target = target
    end
  elseif e:hasFlags(RenderComponent.STATIC) then
    player.script.state.target = nil
    if player.movement == nil or player.render == nil then
      return
    end
    player.navigation:go(e.intersection.x, e.intersection.y, e.intersection.z)
  end
end

local initialized = false

function onReady(e)
  if initialized then
    return
  end

  initialized = true
  player = eal:getEntity("sinbad")

  --spawn()
  --spawnMore(10)
  camera:createAndAttach('free', 'orbo', {target='sinbad', cameraOffset=Vector3.new(0, 4, 0), distance=20, policy=EntityFactory.REUSE})
end

event:onOgreSelect(core, SelectEvent.OBJECT_SELECTED, onSelect)
event:bind(core, Facade.LOAD, onReady)
console_visible = false
game:loadSave('gameStart')

if imgui then
  imguiConsole = Console(256)
  imguiConsole:setOpen(true)
  imgui.manager:addView("console", imguiConsole, false)
  stats = Stats("stats", false)
  imgui.manager:addView("stats", stats, false)

  local recastEditor = require 'imgui.systems.recast'
  imgui.manager:addView("recast", recastEditor(true, false), false)
end
