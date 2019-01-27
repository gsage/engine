local camera = require 'factories.camera'
local event = require 'lib.event'
require 'imgui.components.webview'
require 'imgui.base'
local icons = require 'imgui.icons'
local lm = require 'lib.locales'

-- script editor view
ScriptEditor = class(ImguiWindow, function(self, textureID, title, docked, open)
  ImguiWindow.init(self, title, docked, open)
  self.webview = WebView(textureID, "client:/scriptEditor.html")
  self.textureID = textureID
  self.filepath = imgui.TextBuffer(1024)
  self.filepathCallback = function(data)
  end

  self.code = ""

  cef:addMessageHandler("syncEditor", function(text)
    local p = path or self.currentFilePath
    self.code = text
    if not p then
      return
    end
    data:write(p, self.code, core.env.workdir)
    log.info("Saving file " .. p)
  end)
  self.icon = icons.code
end)

-- render script editor
function ScriptEditor:__call()
  if not self.open then
    return
  end

  imgui.PushStyleVar_2(ImGuiStyleVar_WindowPadding, 5.0, 5.0)
  imgui.SetWantCaptureMouse(false)
  local drawing = self:imguiBegin()
  imgui.PopStyleVar(ImGuiStyleVar_WindowPadding)

  self.webview.visible = drawing

  if not drawing then
    return
  end

  if imgui.Button("save") then
    self:saveFile()
  end

  imgui.SameLine()
  if imgui.InputTextWithCallback(lm("script_editor.file_path"), self.filepath, ImGuiInputTextFlags_EnterReturnsTrue, self.filepathCallback) then
    local path = self.filepath:read()
    if text ~= "" then
      self:openFile(path)
    end
  end

  local w, h = imgui.GetContentRegionAvail()
  local x, y = imgui.GetCursorScreenPos()

  self.webview:render(w, h)

  self:imguiEnd()
  imgui.SetWantCaptureMouse(false)
end

-- cleanup resources
function ScriptEditor:destroy()
  self.webview:destroy()
end

-- open file for editing
function ScriptEditor:openFile(path)
  local text, success = data:read(path)
  if not success then
    log.error("Failed to load script " .. path)
    return
  end

  self.code = string.gsub(text, "(['\"])", "\\%1")

  local mode = "application/json"
  if string.match(".*lua$", path) then
    mode = "lua"
  end

  local script = [[
    setMode("]] .. mode .. [[");
    setText(`]] .. self.code .. [[`);
  ]]

  self.webview:executeJavascript(script)
  self.currentFilePath = path
end

-- save file
function ScriptEditor:saveFile(path)
  self.webview:executeJavascript("syncEditor()")
end
