require 'lib.class'

-- base class for all engine entities
-- contains only general functions
local common = {}

function common.base()
  local CoreEntity = class(function(self, entity)
    self.entity = entity
    if not self.entity then
      error("Nil entity provided")
    end
    self.id = entity.id
  end)

  local __index = CoreEntity.__index

  function CoreEntity:__index(key)
    if key == "entity" then
      return core:getEntity(self.id)
    end

    if key == "valid" then
      return self.entity ~= nil
    end

    if key ~= "entity"  and self.entity then
      if key == "props" then
        return self.entity.props
      end

      if self.entity:hasComponent(key) then
        return self.entity[key](self.entity)
      end
    end

    return __index[key]
  end

  function CoreEntity:__props()
    if not self.entity then
      return {}
    end

    local components = self.entity.componentNames
    local props = {}
    for i = 1, #components do
      props[i] = PropertyDescription.new(components[i], components[i] .. " component")
    end
    return props
  end

  -- configure entity: run all set up methods
  function CoreEntity:__configure()
    for _, method in pairs(self.configureChain) do
      method(self)
    end
  end

  -- destruction handlers
  function CoreEntity:__destroy()
    log.trace("EAL: destroying lua extensions for entity " .. self.entity.id)
    self.entity = nil
    for _, method in pairs(self.destroyChain) do
      method(self)
    end
  end

  CoreEntity.configureChain = {}
  CoreEntity.destroyChain = {}

  function CoreEntity.onCreate(method)
    CoreEntity.configureChain[#CoreEntity.configureChain + 1] = method
  end

  function CoreEntity.onDestroy(method)
    CoreEntity.destroyChain[#CoreEntity.destroyChain + 1] = method
  end

  return CoreEntity
end

return common
