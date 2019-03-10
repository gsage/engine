local camera = require 'factories.camera'
local event = require 'lib.event'
require 'imgui.components.webview'
local imguiInterface = require 'imgui.base'
local icons = require 'imgui.icons'
local lm = require 'lib.locales'

-- script editor view
ScriptEditor = class(ImguiWindow, function(self, textureID, title, docked, open)
  ImguiWindow.init(self, title, docked, open)
  self.webview = WebView(textureID, "client:/scriptEditor.html")
  self.textureID = textureID

  self.code = ""

  cef:addMessageHandler("syncEditor", function(text)
    local p = path or self.currentFilePath
    self.code = text
    local writeFile = function(file)
      data:write(file, self.code, core.env.workdir)
      log.info("Saving file " .. file)
    end

    if p then
      writeFile(p)
    else
      local assets = imguiInterface:getView("assets")
      if not assets then
        self.modal:showError(lm("modals.errors.failed_to_create_asset"))
        return
      end

      local sceneName = nil
      assets:createAsset("new", "json", nil, true, function(file)
        writeFile(file)
        self.currentFilePath = file
      end)

      return
    end
  end)
  self.icon = icons.code
  self.titleBase = self.title

  self:registerContext({
    {
      icon = icons.insert_drive_file,
      action = "new",
      callback = function()
        self:reset()
      end
    },
    {
      icon = icons.save,
      action = "save",
      callback = function()
        self:saveFile()
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
    }
  })
end)

-- render script editor
function ScriptEditor:__call()
  if not self.open then
    return
  end

  imgui.PushStyleVar_2(ImGuiStyleVar_WindowPadding, 5.0, 5.0)
  local drawing = self:imguiBegin()

  imgui.PopStyleVar(ImGuiStyleVar_WindowPadding)

  self.webview.visible = drawing

  if not drawing then
    return
  end

  imguiInterface:captureMouse(not imgui.IsWindowHovered())

  local w, h = imgui.GetContentRegionAvail()
  local x, y = imgui.GetCursorScreenPos()

  self.webview:render(w, h)

  self:imguiEnd()
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

  self.params = {filename = game.filesystem:filename(path)}

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
  if self.dockspace then
    self.open = true
    self.dockspace:activateDock(self.label)
  end
end

function ScriptEditor:reset()
  local script = [[
    setText('');
  ]]
  self.webview:executeJavascript(script)
  self.currentFilePath = nil
end

-- save file
function ScriptEditor:saveFile(path)
  self.webview:executeJavascript("syncEditor()")
end

function ScriptEditor:undo()
  self.webview:executeJavascript([[execCommand("undo");]])
end

function ScriptEditor:redo()
  self.webview:executeJavascript([[execCommand("redo");]])
end
