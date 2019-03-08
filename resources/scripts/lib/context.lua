require 'lib.class'

local event = require 'lib.event'
local bindings = require 'lib.bindings'

Context = class(function(self, name, actions)
  self.actionsList = actions
  self.name = name
  self.callbacks = {}
  for _, value in ipairs(actions) do
    self.callbacks[value.action] = value.callback
  end
  self.bindings = bindings:get(name)
end)

-- resolve and process action on context
function Context:handleKey(event)
  if not self.bindings then
    return true
  end

  local action = self.bindings:resolve(event)
  if not action then
    return true
  end

  return self:action(action)
end

-- do action on context
function Context:action(name, ...)
  local action = self.callbacks[name]
  if not action then
    return true
  end

  return action(table.unpack({...}))
end

ContextManager = class(function(self)
  self.contexts = {}

  self.currentContext = nil
  self.onKey = function(event)
    if self.currentContext then
      local res = self.currentContext:handleKey(event)
      if res == false then
        return false
      end

      return true
    end
  end

  event:onKeyboard(core, KeyboardEvent.KEY_DOWN, self.onKey)

  self.contextChangeListeners = {}
end)

function ContextManager:onContextChange(callback)
  table.insert(self.contextChangeListeners, callback)
end

-- register new context
function ContextManager:register(name, actions)
  self.contexts[name] = Context(name, actions)
end

-- activate context
function ContextManager:activate(name)
  local context = self.contexts[name]
  if not context then
    return
  end

  self.currentContext = context
  for _, cb in ipairs(self.contextChangeListeners) do
    cb(context)
  end
  return context
end

-- deactivate context
function ContextManager:deactivate(name)
  if self.currentContext == self.contexts[name] then
    self.currentContext = nil
  end
end

-- do action on current context
function ContextManager:action(name, ...)
  if not self.currentContext then
    return
  end

  return self.currentContext:action(name, ...)
end

-- get context object
function ContextManager:getContext(name)
  return self.contexts[name]
end

local cm = ContextManager()
return cm
