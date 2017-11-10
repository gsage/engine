package.path = package.path .. ';' .. getResourcePath('scripts/?.lua') ..
                               ';' .. getResourcePath('behaviors/trees/?.lua') ..
                               ";" .. getResourcePath('behaviors/?.lua') .. ";"

require 'math'
require 'helpers.base'
require 'lib.behaviors'
require 'actions'
require 'factories.camera'
require 'factories.emitters'
require 'imgui.gizmo'
local eal = require 'lib.eal.manager'
local event = require 'lib.event'

function context()
  return rocket.contexts["main"]
end

function initLibrocket()
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
  cursor = resource.loadCursor(context(), "cursor.rml")

  main:Show()
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
initLibrocket()

if imgui then
  local gizmo = Gizmo()
  imgui.render:addView("gizmo", gizmo)
end
