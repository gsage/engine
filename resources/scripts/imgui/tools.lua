require 'lib.class'
require 'imgui.base'
local icons = require 'imgui.icons'
local lm = require 'lib.locales'

-- editor tools
ToolsView = class(ImguiWindow, function(self, docked, open)
  ImguiWindow.init(self, "tools", docked, open)
  self.icon = icons.dehaze
  self.flags = ImGuiWindowFlags_NoScrollWithMouse + ImGuiWindowFlags_NoScrollbar
end)

-- render tools
function ToolsView:__call()
  if self:imguiBegin() then
    -- new project
    imgui.Button(icons.insert_drive_file)
    if imgui.IsItemHovered() then
        imgui.BeginTooltip();
        imgui.Text(lm("tools.tooltips.create") .. "(CMD + N)");
        imgui.EndTooltip();
    end
    imgui.SameLine(0, 2)
    -- save project
    imgui.Button(icons.save)
    if imgui.IsItemHovered() then
        imgui.BeginTooltip();
        imgui.Text(lm("tools.tooltips.save"));
        imgui.EndTooltip();
    end
    imgui.SameLine(0, 10)
    -- undo
    imgui.Button(icons.undo)
    if imgui.IsItemHovered() then
        imgui.BeginTooltip();
        imgui.Text(lm("tools.tooltips.undo"));
        imgui.EndTooltip();
    end
    imgui.SameLine(0, 2)
    -- redo
    imgui.Button(icons.redo)
    if imgui.IsItemHovered() then
        imgui.BeginTooltip();
        imgui.Text(lm("tools.tooltips.redo"));
        imgui.EndTooltip();
    end
    imgui.SameLine(0, 2)
    self:imguiEnd()
  end
end
