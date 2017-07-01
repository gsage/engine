
-- This helpers are wrapping basic functional of the engine

-- Resource loading helpers
resource = resource or {}

if rocket ~= nil then
  function resource.loadFont(fileName)
    rocket:LoadFontFace(getResourcePath('fonts/' .. fileName))
  end

  function resource.loadDocument(context, documentName)
    return context:LoadDocument(getResourcePath('ui/' .. documentName))
  end

  function resource.loadCursor(context, cursor)
    return context:LoadMouseCursor(getResourcePath('ui/' .. cursor))
  end
end

-- Entity management helpers
--
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
