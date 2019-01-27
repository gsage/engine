require 'imgui.components.webview'
require 'imgui.base'
local lm = require 'lib.locales'

-- project creation wisard view
ProjectWizard = class(ImguiWindow, function(self, textureID, title, open)
  ImguiWindow.init(self, title, false, open)

  local recent = {
    {
      name = "Working title",
      opened = "06.08.2009",
      path = "/Users/artem/gsageProjects/working_title"
    },
    {
      name = "The very first gsage project",
      opened = "06.08.2008",
      path = "/Users/artem/gsageProjects/first"
    },
  }
  local data = {
    wizard = core.settings.projectWizard,
    hasRecentProjects = #recent > 0,
    recent = recent,
  }
  self.webview = WebView(textureID, "client:/projectWizard/index.html.template", data)
  self.textureID = textureID

  self.flags = ImGuiWindowFlags_NoTitleBar + ImGuiWindowFlags_NoResize + ImGuiWindowFlags_NoMove + ImGuiWindowFlags_NoScrollbar
end)

-- render wisard
function ProjectWizard:__call()
  if not self.open then
    return
  end

  imgui.SetNextWindowPos(0, 0)
  local width, height = imgui.DisplaySize()
  imgui.SetNextWindowSize(width, height)
  imgui.PushStyleVar_2(ImGuiStyleVar_WindowPadding, 0.0, 0.0)
  imgui.SetWantCaptureMouse(false)
  local drawing = self:imguiBegin()
  imgui.PopStyleVar(ImGuiStyleVar_WindowPadding)

  self.webview.visible = drawing

  if not drawing then
    return
  end

  local w, h = imgui.GetContentRegionAvail()
  local x, y = imgui.GetCursorScreenPos()

  self.webview:render(w, h)

  self:imguiEnd()
  imgui.SetWantCaptureMouse(false)
end

-- cleanup resources
function ProjectWizard:destroy()
  self.webview:destroy()
end
