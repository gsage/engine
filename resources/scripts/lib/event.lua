require 'lib.class'

-- event proxy allows subscribing lua functions to C++ events
local EventProxy = class(function(self)
  self.handler = function(id, event, target)
    self:handle(id, event, target)
  end

  -- initializing C++ connection
  self.connection = LuaEventConnection.new(self.handler)
  -- direct handler subscriber
  --self.direct = LuaEventProxy.new()
  self.handlers = {__mode = 'v'}
  self.routes = {}
end)

-- bind callback to generic event type
-- @param target event dispatcher
-- @param type event type
-- @param callback event listener
function EventProxy:bind(target, type, callback, global, handlerName)
  if not callback then
    return
  end

  local id = self.connection[handlerName or "bind"](self.connection, target, type)
  self:traverseSet(target, type, callback, id)
  self.handlers[id] = callback
end

-- unbind callback
-- @param target event dispatcher
-- @param type event type
-- @param callback event listener
function EventProxy:unbind(target, type, callback, all)
  local ids = self:traverseGet(target, type, callback)
  if not ids then
    return false
  end

  local removed = false
  for id in pairs(ids) do
    self.handlers[id] = nil
    removed = self.connection:unbind(id)
    ids[id] = nil
    if not all then
      break
    end
  end

  return removed
end

local __index = EventProxy.__index

function EventProxy:__index(key)
  if __index[key] == nil and self.connection[key] then
    return function(self, target, type, callback, global)
      return self:bind(target, type, callback, global, key)
    end
  end

  return __index[key]
end

function EventProxy:handle(id, event, target)
  local handler = self.handlers[id]
  if not handler then
    -- returning true will unsubscribe the listener
    return true
  end

  handler(event, target)
end

function EventProxy:traverseSet(target, type, callback, id)
  local targetID = target:id()
  if not self.routes[targetID] then
    self.routes[targetID] = {}
  end

  if not self.routes[targetID][type] then
    self.routes[targetID][type] = {}
  end

  local ids = self.routes[targetID][type][callback]
  if not ids then
    self.routes[targetID][type][callback] = {}
    ids = self.routes[targetID][type][callback]
  end
  ids[id] = true
end

function EventProxy:traverseGet(target, type, callback)
  local targetID = target:id()
  if not self.routes[targetID] then
    return nil
  end

  if not self.routes[targetID][type] then
    return nil
  end

  return self.routes[targetID][type][callback]
end

local eventProxy = EventProxy()
return eventProxy
