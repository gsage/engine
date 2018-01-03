require 'lib.class'


-- base class for all windowed views
ImguiWindow = class(function(self, title, docked, open)
  self.docked = docked
  if self.open == nil then
    self.open = true
  else
    self.open = open
  end
  self.title = title
  if self.docked then
    self.imguiBegin = function(self)
      local active
      active, self.open = imgui.BeginDockOpen(self.title, self.open, 0)
      return active
    end
    self.imguiEnd = function(self)  imgui.EndDock() end
  else
    self.imguiBegin = function(self)
      local active
      active, self.open = imgui.Begin(self.title, self.open, 0)
      return active
    end
    self.imguiEnd = function(self) imgui.End() end
  end
end)

-- wrap function into ImguiWindow
local ImguiWindowRender = class(ImguiWindow, function(self, callback, title, docked, open)
  ImguiWindow.init(self, title, docked, open)
  self.callback = callback
end)

-- render view
function ImguiWindowRender:__call()
  if not self.open then
    return
  end

  if self:imguiBegin() then
    self:callback()
  end
  self:imguiEnd()
end

-- imgui interface
local ImguiInterface = class(function(self)
  self.views = {}
end)

-- is imgui available for usage
function ImguiInterface:available()
  return imgui ~= nil and imgui.render ~= nil
end

-- add view to imgui render list
function ImguiInterface:addView(name, view, docked, open, title)
  if not self:available() then
    return false
  end

  if type(view) == "function" then
    view = ImguiWindowRender(view, title or name, docked, open)
  end

  local added = imgui.render:addView(name, view)
  if not added then
    return false
  end
  self.views[name] = view
  return true
end

-- remove view from imgui render list
function ImguiInterface:removeView(name, view)
  if not self:available() then
    return false
  end

  local removed = imgui.render:removeView(name, view)
  self.views[name] = nil
  return removed
end

-- open/close view
function ImguiInterface:setViewOpen(name, value)
  if self.views[name] then
    self.views[name].open = value
    return true
  end
  return false
end

local imguiInterface = imguiInterface or ImguiInterface()
return imguiInterface
