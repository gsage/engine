require 'imgui.base'

List = class(function(self, text, items)
  self.text = text
  self.items = items
end)

function List:render()
  if imgui.BeginMenu(self.text) then
    for _, item in pairs(self.items) do
      local selected = nil
      if item.selected then
        selected = item.selected()
      end
      if imgui.MenuItem(item.text, self.shortcut, selected) then
        item.action()
      end
    end
    imgui.EndMenu()
  end
end

MenuItem = class(function(self, text, action, selected, shortcut)
  self.text = text
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
