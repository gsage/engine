emitter = emitter or {}

local systems = {}

function systems.damage(name)
  local data = {
    id = name,
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
    },
    script = {
      setupFunction = function(self)
        -- injecting
        damageHit = self:render().root:getParticleSystem('damageHit')
        damageMiss = self:render().root:getParticleSystem('damageMiss')
        damageEnemy = self:render().root:getParticleSystem('damageEnemy')
      end
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
  return entity.get(name)
end
