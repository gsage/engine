package.path = package.path .. ';' .. getResourcePath('scripts/?.lua') ..
                               ';' .. getResourcePath('scripts/helpers/?.lua') ..
                               ';' .. getResourcePath('behaviors/trees/?.lua') .. 
                               ";" .. getResourcePath('behaviors/?.lua') .. ";"

require 'base'
require 'math'
require 'async'
require 'behaviors'
require 'actions'


function startup()
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

  maincontext = rocket.contexts["main"]
  main = resource.loadDocument(maincontext, "minimal.rml")
  console = resource.loadDocument(maincontext, "console_document.rml")
  healthbar = resource.loadDocument(maincontext, "healthbar.rml")

  cursor = resource.loadCursor(maincontext, "cursor.rml")

  main:Show()
  healthbar:Show()
end

counter = 0

function spawnMore(count)
  for i=1, count do
    spawn()
  end
end

function spawn()
  local id = "ninja" .. counter
  counter = counter + 1
  entity.create("ninja", {name = id, speed = "10" })
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

function onSelect(event)
  if event:hasFlags(SceneNode.DYNAMIC) then
    local target = entity.get(event.entity)
    if actions.attackable(player, target) then
      player.script.state.target = target
    end
  elseif event:hasFlags(SceneNode.STATIC) then
    player.script.state.target = nil
    if player.movement == nil or player.render == nil then
      return
    end
    player.movement:go(event.intersection)
  end
end

function onRoll(event)
  if event.type == "rollOver" then
    local target = entity.get(event.entity)
    if event:hasFlags(SceneNode.DYNAMIC) then
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
  core.movement:setControlledEntity(player.id)

  event:bind(core, "objectSelected", onSelect)
  event:bind(core, "rollOver", onRoll)
  event:bind(core, "rollOut", onRoll)

  core.script:addUpdateListener(async.addTime)

  spawn()
  spawnMore(10)
end

event:bind(core, "load", onReady)

console_visible = false
startup()
game:loadSave('gameStart')
main:AddEventListener('keydown', onKeyEvent, true)
console:AddEventListener('keydown', onKeyEvent, true)
