require 'coroutine'
local event = require 'lib.event'

-- coroutine safe game loop update dispatcher
local time = {}

time.handlers = {}
time.rawHandlers = {}

-- game loop update handler
-- @param elapsed elapsed time
function time.update(elapsed)
  for id, object in pairs(time.handlers) do
    object.handler(elapsed)
  end
end

-- add new update handler
-- @param id handler id
-- @param handler that listens for the update
-- @param global don't remove handler on facade reset
function time.addHandler(id, handler, global)
  local co, isRoot = coroutine.running()
  -- if called from root coroutine, then we can register the handler
  -- directly in the script system
  if not co or isRoot then
    core:script():addUpdateListener(handler, global)
    time.rawHandlers[id] = {
      handler = handler,
      global = global,
    }
    return
  end

  time.handlers[id] = {
    handler = handler,
    global = global,
  }
end

-- remove update handler
-- @param id handler id
-- @param handler
-- @return false if failed to remove the handler
function time.removeHandler(id, handler)
  if id == nil then
    return false
  end
  time.handlers[id] = nil
  time.rawHandlers[id] = nil

  return true
end

-- clear handlers
-- @param include global handlers
function time.clearHandlers(all)
  if all == true then
    time.handlers = {}
    time.rawHandlers = {}
    return
  end

  for k, v in pairs(time.handlers) do
    if not v.global then
      time.handlers[k] = nil
    end
  end

  -- no need to unsubscribe rawHandlers, because script system manages it itself
  for k, v in pairs(time.rawHandlers) do
    if not v.global then
      time.rawHandlers[k] = nil
    end
  end
end

event:bind(core, Facade.BEFORE_RESET, time.clearHandlers)
core:script():addUpdateListener(time.update, true)

return time
