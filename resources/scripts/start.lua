package.path = package.path .. ';' .. getResourcePath('scripts/?.lua') ..
                               ';' .. getResourcePath('scripts/helpers/?.lua') ..
                               ';' .. getResourcePath('behaviors/trees/?.lua') .. 
                               ";" .. getResourcePath('behaviors/?.lua') .. ";"

require 'base'
require 'math'
require 'async'
require 'behaviors'
require 'actions'
require 'factories.camera'
require 'factories.emitters'

function context()
  return rocket.contexts["main"]
end

function startup()
  if rocket == nil then
    return
  end

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

  main = resource.loadDocument(context(), "minimal.rml")
  console = resource.loadDocument(context(), "console_document.rml")
  healthbar = resource.loadDocument(context(), "healthbar.rml")

  cursor = resource.loadCursor(context(), "cursor.rml")

  main:Show()
  healthbar:Show()
  main:AddEventListener('keydown', onKeyEvent, true)

  console:AddEventListener('keydown', onKeyEvent, true)
end

function setOrbitalCam(id, cameraID)
  camera:createAndAttach('orbit', 'orbo', {target='sinbad', cameraOffset=Vector3.new(0, 4, 0), distance=20})
end

function spawnMore(count)
  for i=1, count do
    spawn()
  end
end

function spawn()
  entity.create("ninja", {movement = {speed = 10}})
end

function onKeyEvent(event)
  if context() == nil then
    return
  end

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

function onSelect(e)
  if e:hasFlags(OgreSceneNode.DYNAMIC) then
    local target = entity.get(e.entity)
    if actions.attackable(player, target) then
      player:script().state.target = target
    end
  elseif e:hasFlags(OgreSceneNode.STATIC) then
    player:script().state.target = nil
    if player:movement() == nil or player:render() == nil then
      return
    end
    player:movement():go(e.intersection)
  end
end

function onRoll(e)
  if e.type == "rollOver" then
    local target = entity.get(e.entity)
    if e:hasFlags(OgreSceneNode.DYNAMIC) then
      cursor:GetElementById("cursorIcon"):SetClass("attack", actions.attackable(player, target))
    end
  else
    cursor:GetElementById("cursorIcon"):SetClass("attack", false)
  end
end

local initialized = false

function onReady(e)
  if initialized then
    return
  end

  initialized = true
  player = entity.get("sinbad")
  core:movement():setControlledEntity(player.id)

  event:ogreSelect(core, "objectSelected", onSelect)
  if rocket ~= nil then
    event:ogreSelect(core, "rollOver", onRoll)
    event:ogreSelect(core, "rollOut", onRoll)
  end

  core:script():addUpdateListener(async.addTime)

  spawn()
  spawnMore(10)
end

event:bind(core, "load", onReady)
console_visible = false
startup()
game:loadSave('gameStart')

-- test view for imgui
TestView = class(function()
end)

function view.__call()
  imgui.ShowTestWindow()
end

if imgui then
  imgui.render:addView("test", view)
end
