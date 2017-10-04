actions = actions or {}

local async = require 'lib.async'

function actions.attackable(self, target)
  if target.id == self.id then
    return false
  end

  if self.stats == nil or target.stats == nil then
    return false
  end

  if target.stats:getNumber("hp", 0) <= 0 then
    return false
  end

  return true
end

function actions.inflictDamage(self, target)
  -- TODO: use damage system
  target.stats:increase("hp", -actions.calculateDamage(self, target))
  if target.movement.speed == 0 then
    return
  end
  local previousSpeed = target.movement.speed
  target.movement.speed = 0
  async.callLater(target.stats:getNumber("stopDelay", 0.5), function()
    target.movement.speed = previousSpeed
  end)

end

function actions.calculateDamage(self, target)
  local damage = self.stats:getNumber("atk") or 0
  local emitter = eal:getEntity("damage")
  if not emitter then
    return
  end
  damage = math.max(0, math.random() + math.random(0, damage))
  emitter:onDamage(self, target, damage)
  return damage
end
