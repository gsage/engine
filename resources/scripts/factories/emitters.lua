local eal = require 'lib.eal.manager'
require 'factories.base'

local EmitterFactory = class(EntityFactory, function(self)
  EntityFactory.init(self, "emitter")
end)

emitter = emitter or EmitterFactory()

emitter:register("damage", function(name)
  local e = {
    id = name,
    vars = {
      class = "damageEmitter",
      pc = "damageHit",
      miss = "damageMiss",
      hostile = "damageEnemy",
    },
    render = {
      root = {
        children = {
          {
            type = "particleSystem",
            name = "damageHit",
            template = "Damage/Hit"
          },
          {
            type = "particleSystem",
            name = "damageMiss",
            template = "Damage/Miss"
          },
          {
            type = "particleSystem",
            name = "damageEnemy",
            template = "Damage/Enemy"
          }
        }
      }
    }
  }

  if not data:createEntity(e) then
    return nil
  end

  return eal:getEntity(name)
end)

return emitter
