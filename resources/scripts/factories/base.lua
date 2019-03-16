require 'lib.class'

local event = require 'lib.event'
local factories = {}

-- base factory
EntityFactory = class(function(self, id, definitions)
  self.definitions = definitions or {}
  self.id = id
  self.createdObjects = {}
  self.createdObjectsCount = 0
  local onReset = function()
    self.createdObjects = {}
    self.createdObjectsCount = 0
  end

  event:bind(core, Facade.RESET, onReset, true)

  factories[id] = self
end)

-- creation policies
EntityFactory.REPLACE = "replace"
EntityFactory.NOTHING = "do nothing"
EntityFactory.REUSE = "reuse"

function EntityFactory:getAvailableTypes()
  return self.availableTypes or {}
end

function EntityFactory:create(type, name, settings)
  local objectName = name or type .. tostring(self.createdObjectsCount)
  local settings = settings or {}
  local policy = settings.policy or EntityFactory.REPLACE
  log.info("Create " .. self.id .. " " .. type .. " name " .. objectName .. " with policy " .. policy)

  if self.createdObjects[objectName] and self.createdObjects[objectName].valid then
    if policy == EntityFactory.REPLACE then
      if not core:removeEntity(objectName) then
        log.error("Failed to delete old entity")
        return nil
      end
      self.createdObjects[objectName] = nil
    elseif policy == EntityFactory.REUSE then
      return self.createdObjects[objectName]
    else
      return nil
    end
  end

  local constructor = self:getConstructor(type)
  if not constructor then
    log.error("No constructor defined for type " .. tostring(type))
    return nil
  end

  local result = constructor(objectName, settings)
  if not result then
    log.error("Failed to create entity " .. objectName ..  " of type " .. tostring(type))
  end
  self.createdObjectsCount = self.createdObjectsCount + 1
  self.createdObjects[objectName] = result
  return result
end

function EntityFactory:getConstructor(type)
  return self.definitions[type]
end

function EntityFactory:register(type, func)
  self.definitions[type] = func
  self:updateAvailableTypes()
end

function EntityFactory:updateAvailableTypes()
  self.availableTypes = {}
  for key in pairs(self.definitions) do
    table.insert(self.availableTypes, key)
  end
end

return factories
