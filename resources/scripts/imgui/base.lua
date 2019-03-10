require 'lib.class'
local lm = require 'lib.locales'
local contextManager = require 'lib.context'

-- base class for all windowed views
ImguiWindow = class(function(self, label, docked, open)
  self.docked = docked
  self.flags = 0
  self.dockFlags = 0
  self.context = nil

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
      if imgui.IsMouseHoveringWindow() and imgui.IsMouseDown(1) then
        imgui.SetWindowFocus()
      end
      return active
    end
    self.imguiEnd = function(self)
      self.dragging = imgui.IsMouseDragging(0) and imgui.IsMouseHoveringWindow()
      self:updateContext()
      imgui.EndDock()
    end
  else
    self.imguiBegin = function(self)
      local active
      active, self.open = imgui.Begin(self.label, self.open, self.flags)
      if imgui.IsMouseHoveringWindow() and imgui.IsMouseDown(1) then
        imgui.SetWindowFocus()
      end
      return active
    end
    self.imguiEnd = function(self)
      self.dragging = imgui.IsMouseDragging(0) and imgui.IsMouseHoveringWindow()
      self:updateContext()
      imgui.End()
    end
  end
end)

function ImguiWindow:registerContext(actions)
  self.context = actions
  contextManager:register(self.label, actions)
end

function ImguiWindow:updateContext()
  if not self.context then
    return
  end

  local activate = imgui.IsWindowFocused()
  if activate == self.contextActive then
    return
  end

  self.contextActive = activate
  if activate then
    contextManager:activate(self.label)
  else
    contextManager:deactivate(self.label)
  end
end

function ImguiWindow:getLocalizedTitle()
  local res
  if self.params then
    res = lm("window_title." .. self.label .. "_parameterised", self.params)
  else
    res = lm("window_title." .. self.label)
  end
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
function ImguiInterface:addView(dest, name, view, open, label)
  if not self:available() then
    return false
  end

  if type(view) == "function" then
    view = ImguiWindowRender(view, label or name, open)
  end

  local added = dest:addView(name, view)
  if not added then
    return false
  end
  if view.added then
    view:added()
  end
  self.views[name] = view
  return true
end

-- remove view from imgui render list
function ImguiInterface:removeView(dest, name, view)
  if not self:available() then
    return false
  end

  local removed = dest:removeView(name, view)
  self.views[name] = nil
  if not removed then
    log.error("failed to remove view " .. name)
  end

  if view.destroy then
    view:destroy()
  end
  return removed
end

-- get view with name
function ImguiInterface:getView(name)
  return self.views[name]
end

-- open/close view
function ImguiInterface:setViewOpen(name, value)
  if self.views[name] then
    self.views[name].open = value
    return true
  end
  return false
end

function ImguiInterface:captureMouse(value)
  if not imgui.GetWantCaptureMouse() then
    return
  end

  return imgui.SetWantCaptureMouse(value)
end

function ImguiInterface:getViewDragged()
  for name, view in pairs(self.views) do
    if view.dragging then
      return view
    end
  end

  return nil
end

local imguiInterface = imguiInterface or ImguiInterface()
return imguiInterface
