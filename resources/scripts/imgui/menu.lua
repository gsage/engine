require 'imgui.base'
local lm = require 'lib.locales'

List = class(function(self, text, items)
  self.text = text
  self.items = items
end)

function List:render()
  if imgui.BeginMenu(lm(self.text)) then
    for _, item in pairs(self.items) do
      if item.type == "separator" then
        imgui.Separator()
      else
        local selected = nil
        if item.selected then
          selected = item.selected()
        end
        if imgui.MenuItem(lm(item.text), self.shortcut, selected) then
          item.action()
        end
      end

    end
    imgui.EndMenu()
  end
end

MenuItem = class(function(self, text, action, selected, shortcut)
  self.text = text
  self.type = "MenuItem"
  self.action = action
  self.selected = selected
  self.shortcut = shortcut
end)

Menu = class(function(self, menus)
  self.menus = menus
end)

function Menu:__call()
  imgui.BeginMainMenuBar()
  for _, menu in pairs(self.menus) do
    menu:render()
  end
  imgui.EndMainMenuBar()
end

Separator = {type = "separator"}
