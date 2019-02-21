require 'lib.class'
require 'imgui.base'
local icons = require 'imgui.icons'
local lm = require 'lib.locales'
local projectManager = require 'editor.projectManager'

-- editor assets
Assets = class(ImguiWindow, function(self, docked, open)
  ImguiWindow.init(self, "assets", docked, open)
  self.icon = icons.folder_special
  self.levels = nil
end)

-- render assets browser
function Assets:__call()
  if not self.open then
    return
  end

  local drawing = self:imguiBegin()

  if not drawing then
    return
  end

  if imgui.TreeNode(lm("assets.levels")) then
    if self.levels == nil and projectManager.openProjectFile then
      self.levels = game.filesystem:ls(projectManager.openProjectFile.projectRoot .. "/" .. data.levelsFolder)
    end
    for i = 1,#self.levels do
      if imgui.Button(self.levels[i]) then
        game:loadArea(self.levels[i])
      end
    end
    imgui.TreePop()
  end

  self:imguiEnd()
end
