
-- This helpers are wrapping basic functional of the engine

-- Resource loading helpers
resource = resource or {}

function resource.loadFont(fileName)
  rocket:LoadFontFace(getResourcePath('fonts/' .. fileName))
end

function resource.loadDocument(context, documentName)
  return context:LoadDocument(getResourcePath('ui/' .. documentName))
end

function resource.loadCursor(context, cursor)
  return context:LoadMouseCursor(getResourcePath('ui/' .. cursor))
end

-- Entity management helpers

entity = entity or {}

function entity.get(id)
  if entity[id] == nil then
    local e = EntityProxy.new(id, core)
    if e.valid then
      entity[id] = e
    end
  end
  return entity[id]
end

function entity.create(template, params)
  data:createEntity(getResourcePath('characters/' .. template .. '.json'), params)
end

-- View related functions

view = view or {}

function view.getObjectsAround(id, distance, flags, targetId)
  local target = entity.get(id)
  if target == nil or target:render() == nil then
    return {}
  end

  local entities = {}
  local objects = core:render():getObjectsInRadius(target:render().position, distance, flags, targetId)
  for i = 1, #objects do
    table.insert(entities, entity.get(objects[i].id))
  end
  return entities
end

-- Logging

log = log or {}

log.ERROR = Log.logtype.error
log.WARNING = Log.logtype.warning
log.INFO = Log.logtype.info
log.DEBUG = Log.logtype.debug

function log.error(message)
  log.msg(log.ERROR, message)
end

function log.warn(message)
  log.msg(log.WARNING, message)
end

function log.info(message)
  log.msg(log.INFO, message)
end

function log.debug(message)
  log.msg(log.DEBUG, message)
end

function log.msg(level, message)
  Log.Message(level, message)
end
