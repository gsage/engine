require 'lib.class'
require 'imgui.base'
local icons = require 'imgui.icons'
local lm = require 'lib.locales'
local cm = require 'lib.context'

-- editor tools
ToolsView = class(ImguiWindow, function(self, open, positioner)
  ImguiWindow.init(self, "tools", false, open)
  self.icon = icons.dehaze
  self.flags = ImGuiWindowFlags_NoScrollWithMouse + ImGuiWindowFlags_NoScrollbar + ImGuiWindowFlags_NoTitleBar + ImGuiWindowFlags_NoResize + ImGuiWindowFlags_NoMove + ImGuiWindowFlags_NoBringToFrontOnFocus
  self.positioner = positioner
  self.buttons = {}

  self.onContextChange = function(context)
    self.buttons = {}
    for _, value in ipairs(context.actionsList) do
      if value.icon then
        local description = lm(context.name .. ".actions." .. value.action)
        if description == lm.MISSING then
          description = ""
        end

        table.insert(self.buttons, {
          description = description,
          icon = value.icon,
          callback = value.callback,
          padding = value.padding or 2
        })
      end
    end
  end

  cm:onContextChange(self.onContextChange)
end)

-- render tools
function ToolsView:__call()
  local x, y, width, height = self.positioner()
  imgui.SetNextWindowPos(x, y)
  imgui.SetNextWindowSize(width, height)

  if self:imguiBegin() then

    for _, button in ipairs(self.buttons) do
      if imgui.Button(button.icon) then
        button.callback()
      end

      if button.description ~= "" and imgui.IsItemHovered() then
        imgui.BeginTooltip()
        imgui.Text(button.description)
        imgui.EndTooltip()
      end

      imgui.SameLine(0, button.padding)
    end
    self:imguiEnd()
  end
end
