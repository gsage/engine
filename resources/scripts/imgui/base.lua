require 'lib.class'
local lm = require 'lib.locales'

-- base class for all windowed views
ImguiWindow = class(function(self, label, docked, open)
  self.docked = docked
  self.flags = 0
  self.dockFlags = 0
  if self.open == nil then
    self.open = true
  else
    self.open = open
  end
  self.label = label
  if self.docked then
    self.imguiBegin = function(self)
      local active
      active, self.open = imgui.BeginDockTitleOpen(self.label, self:getLocalizedTitle(), self.open, self.flags, self.dockFlags)
      return active
    end
    self.imguiEnd = function(self)  imgui.EndDock() end
  else
    self.imguiBegin = function(self)
      local active
      active, self.open = imgui.Begin(self.label, self.open, self.flags)
      return active
    end
    self.imguiEnd = function(self) imgui.End() end
  end
end)

function ImguiWindow:getLocalizedTitle()
  local res = lm("window_title." .. self.label)
  if res == lm.MISSING then
    res = self.label
  end

  if self.icon then
    res = self.icon .. " " .. res
  end

  return res
end

-- wrap function into ImguiWindow
local ImguiWindowRender = class(ImguiWindow, function(self, callback, label, docked, open)
  ImguiWindow.init(self, label, docked, open)
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
  return imgui ~= nil and imgui.manager ~= nil
end

-- add view to imgui render list
function ImguiInterface:addView(name, view, docked, open, label)
  if not self:available() then
    return false
  end

  if type(view) == "function" then
    view = ImguiWindowRender(view, label or name, docked, open)
  end

  local added = imgui.manager:addView(name, view, not(not docked))
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

  local removed = imgui.manager:removeView(name, view)
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
