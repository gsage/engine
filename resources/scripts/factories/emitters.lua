local eal = require 'lib.eal.manager'

emitter = emitter or {}

local systems = {}

function systems.damage(name)
  local data = {
    id = name,
    props = {
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

  return data
end


function emitter.create(name)
  local factory = systems[name]
  if not factory then
    return nil
  end

  if not data:createEntity(factory(name)) then
    return nil
  end
  return eal:getEntity(name)
end
