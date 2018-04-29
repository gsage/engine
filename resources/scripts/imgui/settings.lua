require 'lib.class'
require 'imgui.base'
local lm = require 'lib.locales'
local icons = require 'imgui.icons'

-- imgui view for editor settings
SettingsView = class(ImguiWindow, function(self, open)
  ImguiWindow.init(self, "settings", true, open)
  self.icon = icons.tune
  self.settings = editor:getGlobalState().settings or {}
end)

-- settings view
function SettingsView:__call()
  if self:imguiBegin() then
    local currentLocale = lm:getLocale()
    imgui.ListBoxHeader(lm("lang.select"), size)
    for _, info in pairs(lm.availableLocales) do
      if imgui.Selectable(info.name, info.id == currentLocale) then
        lm:setLocale(info.id)
        self.settings.locale = info.id
        editor:putToGlobalState("settings", self.settings)
      end
    end
    imgui.ListBoxFooter();
    self:imguiEnd()
  end
end

return SettingsView
